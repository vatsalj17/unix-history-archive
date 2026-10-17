	title	fptrap -- 7/32 unix floating-point emulator
	entry	fptrap
	extrn	uisa0,lra

*
* The following equates are used to generate single and/or double
* precision versions.
* Note that the same flags must also be defined in param.h
*

SPFPT	equ	1		if 1, simulate single-precision f.p.
DPFPT	equ	1		if 1, simulate double-precision f.p.
LRAI	equ	0		if 1, lra instruction available

SIGBUS	equ	10
SIGFPT	equ	8
SIGINS	equ	4
r0	equ	0
r1	equ	1
r2	equ	2
r3	equ	3
r4	equ	4
r5	equ	5
r6	equ	6
sp	equ	7
e8	equ	8
e9	equ	9
ea	equ	10
eb	equ	11
ec	equ	12
ed	equ	13
ee	equ	14
ef	equ	15
e9.x	equ	9
ec.s	equ	12
ee.stat	equ	14
ef.loc	equ	15


*	register usage:	r0,r1,r2,r6  	scratch
*			r3	address of users saved r8 ( see reg.h )
*			r4	address of users fp regs in ppda
*			r5	real pc in kernal segment
*			ed	(rd) real destination addr
*

*
* stack data area definition
*
data	struc
handlr	ds	adc
gregs	ds	8*adc
	ends
	title	entry sequence
*--------------pre-processor

fptrap	equ	*
	shi	sp,data		allocate local variables on stack
	stm	e8,gregs(sp)	save r8-r15 just like c
	lis	r0,SIGINS	set to return error if not fp opcode
*				on first instruction
	stb	r0,laflag	look ahead flag
	l	r3,data(sp)	addr of users saved r0 ( see reg.h )
*
	lm	ee.stat,9*adc(r3) old psw
	shi	r3,10*adc	adjust to point to users saved r8
*					so all offsets are +ve
	l	r4,data+4(sp)	pointer to users fp regs
*
fptrap1	equ	*		look ahead entry ( see nofault )
	lr	r1,ef.loc	make pc real wrt kernal seg regs
	la	r2,uisa0	mac registers 
	ifnz	LRAI
	lra	r1,0(r2)
	else
	bal	r6,lra		load real addr
	endc
*
	btc	x'd',sigbuse	if pc not valid ( not present or 
*					not executable )
	lr	r5,r1		save real pc
*

*--------------the simulated instruction interpretation

	lb	e8,0(r5)	opcode
	lb	ed,1(r5)	r1 & r2 field
	lr	ec.s,ed		r1 into source register
	srls	ec.s,2		r1*4
	ais	ef.loc,2	update location counter
	ais	r5,2		update real pc
	lb	e8,opcodes(e8)	pointer to vector tables
	l	e9,vectab1(e8)	1st level handler pointer
	l	e8,vectab2(e8)	second level handler pointer
	sta	e8,handlr(sp)
	br	e9		go to 1st level handler
	title	format preprocessors
*--------------register & register instructions
	ifnz	SPFPT
xer	equ	*
	srls	ec.s,1		force r1 to multiple-of-4 boundary
	nhi	ec.s,x'1c'
	ar	ec.s,r4		address of pseudo register
	ar	ed,ed		r2*2
	nhi	ed,x'1c'	force r2 to multiple-of-4 boundary
	ar	ed,r4		address of pseudo-register
go.to.it l	eb,handlr(sp)
	br	eb		go to 2nd level handler


	endc
	ifnz	DPFPT
xdr	equ	*
	nhi	ec.s,x'38'	force r1 to multiple-of-8 boundary
	slls	ed,2		r2*4
	nhi	ed,x'38'	force r2 to multiple-of-8 boundary
	ai	ec.s,32(r4)	make up the register's physical addr
	ai	ed,32(r4)
	l	eb,handlr(sp)
	br	eb		go to second levle handler
	endc


*--------------register & memory instruction
	ifnz	SPFPT
xes	equ	*
	lis	r0,1		if store type instruction
	b	xe1
*
xe	equ	*
	lis	r0,0		set load type instruction
xe1	sth	r0,lsflag
	srls	ec.s,1		force r1 to multiple-of-4 boundary
	nhi	ec.s,x'1c'
	ar	ec.s,r4		address of pseudo - register


	endc
	ifnz	DPFPT
	b	xd.2
xds	equ	*		enter here if store type instruction
	lis	r0,1		set flag
	b	xd1
xd	equ	*
	lis	r0,0		set load type instruction
xd1	sth	r0,lsflag
	nhi	ec.s,x'38'	force r1 to multiple-of-8 boundary
	ai	ec.s,32(r4)

	endc
xd.2	equ	*
	lhl	e9,0(r5)	get 1st address h/w
	thi	e9,x'8000'
	bnz	rx2		rx2 format
	thi	e9,x'4000'
	bz	rx1		rx1 format

*--------------rx3

	ais	ef.loc,2	update location counter
	ais	r5,2		update real pc as well
	exhr	eb,e9		address to bits 0-15 of eb
	lhl	ea,0(r5)	get 2nd address h/w
	or	eb,ea		merge address parts
	ni	eb,y'ffffff'	retain 24 bit address
	nhi	ed,x'f'		test 1st index register
	bz	rx3nondx	no index

rx3.2	lb	ed,grtab(ed)	offset from users saved r8
	a	eb,0(r3,ed)	users saved rx
	lr	ed,e9
	srls	ed,8		rx2 to bits 28-31 of ed
	lr	e9,eb		partially completed address

*--------------rx1, rx2 indexing enter here

rx1	la	e8,rx3.5	prepare exit address
	nhi	ed,x'f'		do indexing
	bz	noindex		no 2nd indexing for rx3
rx1.1	equ	*
*
rx3.4	lb	ed,grtab(ed)	offset from users saved r8
	a	e9,0(r3,ed)	users saved rx
*                                  no-indexing rx1 & rx2 join here
noindex	lr	r1,e9		destination addr
	la	r2,uisa0	mac regs
	ifnz	LRAI
	lra	r1,0(r2)
	else
	bal	r6,lra		load real address
	endc
	btc	x'c',sigbuse	send illegal adddress msg to user 
	bfc	x'2',noindx1	if not write protected
	lh	r0,lsflag
	bz	noindx1		if load type instuction
	b	sigbuse		else illegal address
noindx1	lr	ed,r1		real address returned form lra
	br	e8		goto rx3.5 or fldr or flr.2
rx3.5	ais	ef.loc,2	update location counter
	ais	r5,2		update real pc as well
	ni	ed,y'fffffffc'	force onto a full-word boundary
*              ed contains destination addr in program space
	l	e8,handlr(sp)
	br	e8		go to 2nd level handler

*--------------rx2

rx2	sll	e9,17		get the relative address
	sra	e9,17		with an appropriate sign
	ai	e9,2(ef.loc)	adjust it within physical space
	b	rx1

*--------------no indexing for rx3

*
rx3nondx lr	r1,eb		make destination addr real
	la	r2,uisa0	mac regs
	ifnz	LRAI
	lra	r1,0(r2)
	else
	bal	r6,lra		make addr real
	endc
	btc	x'c',sigbuse	address is illegal
	bfc	x'2',rx3no1	if not write protected
	lh	r0,lsflag
	bz	rx3no1		if load type instruction
	b	sigbuse		else bus error
rx3no1	lr	ed,r1		real address 2
	b	rx3.5


*--------------termination logic

dpfinal	equ	*
	ti	ee.stat,4	did overflow occur ?
	bz	nofault		nope
	ti	ee.stat,x'1000'	is af interrupt enabled ?
	bz	nofault		oh no
*
sigfpte	equ	*
	li	r0,SIGFPT	indicate arith error to trap.c
	b	creturn
*

*
*	exit here if illegal address was encountered
*
sigbuse	equ	*
*
	li	r0,SIGBUS	indicate bus error to trap.c
	b	creturn
*
*	exit here if non-floating point opcode 
*
flt.iih	equ	*
crash	equ	*
	sis	ef.loc,2	set pc back to the instruction
	lb	r0,laflag	return code 
	b	creturn
*--------------happy end

errfree	equ	*
nofault	equ	*
	lis	r0,0
	stb	r0,laflag	set return code for subsequent non-fp
*				opcodes
	b	fptrap1		look ahead at next instruction.
*
*	common exit point 
*
creturn	equ	*
	stm	ee.stat,19*adc(r3) store revised psw back
*					into users saved regs
	lm	e8,gregs(sp)	restore callers regs
	ahi	sp,data		remove temporaries from stack
	br	ef		exit back to trap.c
	title	ste and ce
	ifnz	SPFPT
*-----------------------------store floating

ste	l	ea,0(ec.s)	move reg
	st	ea,0(ed)	to memory
	b	nofault		else ok


*-----------------------------compare floating

ce	equ	*
	ni	ee.stat,-16	clear current condition code
	l	ea,0(ec.s)	get source
	l	eb,0(ed)	and destn
	bnm	ce.2		b if dest 0+
	lr	ea,ea		both minus?
	bm	ce.1		b if so

*-----------------------------plus always greater than minus
stps	ais	ee.stat,2	set g flag
	b	dpfinal		get out


*-----------------------------both minus

ce.1	sr	eb,ea		generate b-a
	bm	stms		b if a> b
	bp	stps		b if b>a
	b	dpfinal		exit if equal


*-----------------------------dest plus

ce.2	lr	ea,ea		test source
	bnm	ce.3		branch if both plus

*-----------------------------minus always less than plus
stms	ais	ee.stat,9
	b	dpfinal		get out


*-----------------------------both plus

ce.3	sr	ea,eb		generate a-b
	bm	stms		b if a>b
	bp	stps		b if b>a
	b	dpfinal		exit if equal
	title	le
*-----------------------------load floating

le	equ	*
	ni	ee.stat,-16	clear current cc
	l	ea,0(ed)	get datum to be loaded
	ti	ea,y'f00000'	normalized already?
	bz	le.2		branch if no
	lr	ea,ea		test sign
	bm	stm		store with proper status
	b	stp

*-----------------------------number needs to be normalized
le.2	ti	ea,y'ffffff'	test for zero
	bz	stz
	lr	e9.x,ea		exponent to x
le.4	slls	ea,4		adjust left 1 digit
	si	e9,y'1000000'	decrement exponent
	btc	12,underflo	b if underflow
le.4a	ti	ea,y'f00000'	normalized yet?
	bz	le.4		b if no
le.5	xr	ea,e9.x		merge exponent and a
	ni	ea,y'ffffff'
	xr	ea,e9.x
le.6	bm	stm		store a with appropriate status
	bp	stp
	b	sta


*-----------------------------store a with minus status

stm	ais	ee.stat,1	set l flag
	b	sta		go store a


*-----------------------------store a with plus status

stp	ais	ee.stat,2	set g flag


*-----------------------------just plain store a

sta	st	ea,0(ec.s)	store a
	b	dpfinal		get out


*-----------------------------underflow

underflo ais	ee.stat,4	set v flag

*-----------------------------store zero
stz	xr	ea,ea		clear a
	b	sta		go store a
	title	me
*-----------------------------multiply floating


me	equ	*
	ni	ee.stat,-16	clear current cc
	l	ea,0(ec.s)	fetch a
	bz	sta		exit if zero
	l	eb,0(ed)	load b
	bz	stz		exit if zero
	lr	e9.x,ea		exponent to x
	ni	ea,y'ffffff'
	xr	e9.x,ea		zero x's fraction field
	ar	e9.x,eb		add exponents
	bc	me.c		go to me.2 if c=v
	bno	me.2
	b	me.1
me.c	bo	me.2

*-----------------------------carry out of exponent field
me.1	ti	e9.x,y'40000000' test for overflow
	bnz	overflo
	b	me.3

*-----------------------------no carry out of exponent field
me.2	ti	e9.x,y'40000000' test for underflow
	bz	underflo
me.3	si	e9.x,y'40000000' restore excess-64 notation
	ni	eb,y'ffffff'	clear b's expoment
	slls	eb,7		adjust a and b for best precision
	slls	ea,7
	lr	ed,ea
	xr	ea,ea
	mr	ea,ed
	ti	ea,y'3c000000'	test result magnitude
	bz	me.4		b if only 2-place shift reqd
	ahi	ea,32
me.3a	srls	ea,6		shift right by 6
	b	le.5		merge exp and store result


*-----------------------------only shift by 2, result is small

me.4	ais	ea,2		round
	ti	ea,y'3c000000'	did rounding cause another digit?
	bnz	me.3a		b if yes
	si	e9.x,y'1000000'	else adjust exponent
	btc	12,underflo
	srls	ea,2		normalize
	b	le.5		merge exp and store result
	title	stme and lme
*-----------------------------store multiple floating


stme	equ	*
stmeloop equ	*
	l	ea,0(ec.s)	move reg
	st	ea,0(ed)	store in user's address space
*
	ais	ed,4
	ais	ec.s,4		bump reg counter
	cli	ec.s,32(r4)	dont forget that r4 has pointer to users fp r
	bl	stmeloop
	b	nofault


*-----------------------------load multiple floating


lme	equ	*
lmeloop	equ	*
	l	ea,0(ed)	move memory 
	st	ea,0(ec.s)	to register
	ais	ed,4
	ais	ec.s,4		done?
	clhi	ec.s,32(r4)
	bl	lmeloop
	b	nofault
	title	de
*-----------------------------divide by zero

div.by.0 ais	ee.stat,12	set c and v flags
	l	ea,0(ec.s)
	b	sta
*-----------------------------divide floating


de	equ	*
	ni	ee.stat,-16	clear cc
	l	eb,0(ed)	get divisor
	ti	eb,y'00ffffff'
	bz	div.by.0	out if divisor=0
	l	ea,0(ec.s)	now get dividend
	bz	sta		out if dividend=0
	lr	e9.x,ea		prepare to subtract exponents
	oi	e9.x,y'ffffff'	eliminate any chance of borrow
	sr	e9.x,eb		a exponent minus b exponent
	bc	de.c		go to de.2 if c=v
	bno	de.2
	b	de.1		else de.1
de.c	bo	de.2


*-----------------------------borrow out of exponent field

de.1	ti	e9.x,y'40000000' check for underflow
	bz	underflo
	b	de.3		go adjust exponent


*-----------------------------no borrow out of exponent field

de.2	ti	e9.x,y'40000000' test for overflow
	bnz	overflo2
de.3	ai	e9.x,y'40000000' restore excess-64 notation
	li	ed,y'ffffff'	put b magnitude in divisor reg
	ti	eb,y'00f00000'	operand should be normalized
	bz	dpfinal
	nr	ed,eb
	slls	ed,6
	xr	eb,eb		prepare for divide
	ni	ea,y'ffffff'	get rid of a exponent
	dr	ea,ed		divide.
	lr	ea,eb
	ti	ea,y'3c000000'	test top digit
	bnz	de.4		branch unless zero
	ais	ea,2		round
	ti	ea,y'3c000000'	did rounding cause carry?
	bnz	de.4a		b if yes
	srls	ea,2		normalize (2 bits)
	b	le.5		go store result


*-----------------------------large quotient

de.4	ahi	eb,32		round
de.4a	srls	ea,6		normalize (6 bits)
	b	aese.2b		go test for overflow
	title	ae and se
*-----------------------------add floating

ae	equ	*
	l	eb,0(ed)	pick up b
	b	aese		enter common process


*-----------------------------subtract floating

se	equ	*
	l	eb,0(ed)	pick up b
	xi	eb,y'80000000'	reverse sign

*-----------------------------add and subtract common process
aese	l	ea,0(ec.s)	pick up a
	ni	ee.stat,-16	clear cc
	exhr	e9.x,ea		exponents to work registers
	exhr	ed,eb
	nhi	e9.x,x'7f00'	strip off signs
	nhi	ed,x'7f00'
	sr	e9.x,ed		a exp minus b exp
	bz	aese.3		b if equal
	bm	aese.4		b if b > a
	clhi	e9.x,x'600'	a>>b?
	bnl	aese.sta
	srls	e9.x,6
	lr	ed,eb		save b exp
	ni	eb,y'ffffff'	strip b's exponent
	srl	eb,0(e9.x)	adjust b
	lr	e9.x,ea		save a exp
	ni	ea,y'ffffff'	strip a's exponent

*-----------------------------a > b
agb	xr	ed,e9.x		effective add or subtract?
	bnm	aese.2		b if add
	sr	ea,eb		subtract
	bz	sta		if zero, go store it
	b	le.4a		else go normalize


*-----------------------------a >> b
aese.sta lr	ea,ea
	b	le.6
*-----------------------------effective add

aese.2a	lr	e9.x,ed		(b>a enters here)
aese.2	ar	ea,eb		add
	ti	ea,y'f000000'	carry?
	bz	le.5		if no, go store
	srl	ea,4		normalize
aese.2b	ai	e9.x,y'1000000'	bump exponent
	bfc	12,le.5		go store if no overflow


*-----------------------------overflow

overflo	xi	e9.x,y'80000000' restore valid sign

*-----------------------------overflow entry if sign is ok
overflo2 li	ea,y'7fffffff'	largest possible number
	ais	ee.stat,4	set v bit
	or	ea,e9.x		merge sign with number
	b	le.6		go store with proper status


*-----------------------------equal exponents

aese.3	lr	e9.x,ea		save exponents
	lr	ed,eb
	ni	ea,y'ffffff'	isolate magnitudes
	ni	eb,y'ffffff'
	clr	ea,eb		if a=b pretend a>b
	bnl	agb
	b	bga


*-----------------------------b exp > a exp

aese.4	clhi	e9.x,-x'500'	b>>a?
	bnl	aese.5		b if no
	lr	ea,eb		else go store b
	b	le.6

*-----------------------------a must be adjusted
aese.5	ar	e9.x,ed		complement exponent difference
	sr	ed,e9.x
	srls	ed,6
	lr	e9.x,ea		save a exp
	ni	ea,y'ffffff'	strip exponent from a
	srl	ea,0(ed)	adjust a
	lr	ed,eb		save b exp
	ni	eb,y'ffffff'	strip b exponent

*-----------------------------b > a
bga	xr	e9.x,ed		effective add or subtract?
	bnm	aese.2a		b if effective add
	sr	eb,ea		subtract
	bz	stz		if zero, go store it
	lr	ea,eb		else put reault in "a" registers
	lr	e9.x,ed
	b	le.4a		and go normalize
	title	fxr
*-----------------------------fix (convert to integer)


fxr	nhi	ee.stat,-16	zero current cc
	ar	ed,ed		r2 is floating reg
	nhi	ed,x'1c'	force it even
	ar	ed,r4		address of pseudo reg
	l	ea,0(ed)	get floating number
	lr	eb,ea		magnitude to b
	slls	eb,8		left justified
	lb	e9.x,0(ed)	exponent to x (low byte)
	ni	e9.x,x'7f'	dump sign
	si	e9.x,x'40'	is there an integer part?
	bnp	fxrzero		b if no
	lis	ed,8		compare exponent with 8
	sr	ed,e9.x
	bm	fxrovf		exit if number too big
	bnz	fxr.1		go adjust number unless exp=8
	lr	eb,eb		exp=8 but it could still be too big
	bnm	fxr.2		b if it is ok
fxrovf	ais	ee.stat,4	set v flag
	li	eb,y'7fffffff'	set number as big as possible
	lr	ea,ea
	bp	fxrstore
	b	fxr.2a
fxr.1	slls	ed,2		prepare to adjust
	srl	eb,0(ed)	adjust
fxr.2	lr	ea,ea		test sign
	bp	fxrstore
fxr.2a	xi	eb,-1
	ais	eb,1
	b	fxrstore
fxrzero	xr	eb,eb		zero b
fxrstore lr	eb,eb		test sign of number
	bz	fxr.6		b if zero
	bm	fxr.5		b if minus
	ais	ee.stat,2	set g flag
	b	fxr.6
fxr.5	ais	ee.stat,1	set l flag
fxr.6	equ	*
*
	srls	ec.s,2		was r1 * 4
	lb	r1,grtab(ec.s)	offset from users saved r8
	st	eb,0(r3,r1)	put into users saved general reg
*
	b	nofault
	title	flr
*-----------------------------flr preprocessing

flr.1	srls	ec.s,1		source reg
	nhi	ec.s,x'1c'	word boundary
	ar	ec.s,r4		address of pseudo reg
*
	nhi	ed,x'f'
	lb	ed,grtab(ed)	offset from users saved r8
	l	ed,0(r3,ed)	general register
*
* note: in this case only, ed contains the value of the second
*    operand, not its address!!
*


*-----------------------------float (convert to real)

flr.2	nhi	ee.stat,-16
	lr	ea,ed		get number to float
	bz	sta		out if zero
	bm	flr.3		b if minus
	li	e9.x,y'46000000' get starter exponent
	b	flr.4
flr.3	li	e9.x,y'c6000000' negative number
	xi	ea,-1
	ai	ea,1
flr.4	ti	ea,y'ff000000'	normalized to 6 digits?
	bz	le.4a		<= 6 digits, go finish normalization
	ai	e9.x,y'1000000'	> 6 digits, shift right
	srls	ea,4		and fix exponent
	b	flr.4		try again

*        since everything is done at this point, fxr.10 (see the
*        instruction modification routines section) returns directly
*        to nofault.
	endc
	title	load and store double-precision floating
	ifnz	DPFPT
*--------------load double-precision floating


ld	equ	*
	nhi	ee.stat,-16	clear current condition code
	l	e9,4(ed)
	l	e8,0(ed)
	ti	e8,y'f00000'	is it normalized?
	bz	normlize
ld.50	lr	e8,e8
	bm	stmd		datum is -ve

*--------------positive number - flag g

stpd	ais	ee.stat,2	set g flag
	st	e9,4(ec.s)
	st	e8,0(ec.s)
	b	dpfinal

*--------------negative number - flag l

stmd	ais	ee.stat,1	set l flag

*--------------zero or exception

stad	st	e9,4(ec.s)
	st	e8,0(ec.s)
	b	dpfinal

*--------------forced zero

stzd	xr	e9,e9
	xr	e8,e8
	b	stad


*--------------store double-precision floating


std	equ	*
	l	e9,4(ec.s)	no condition code changes
	l	e8,0(ec.s)
	st	e8,0(ed)
	st	e9,4(ed)
	b	errfree
	title	load multiple and store multiple double-precision float
*--------------load multiple double-precision floating


lmd	equ	*

*--------------lmd loop

lmd.10	l	e8,0(ed)	move memory
	l	e9,4(ed)
	st	e8,0(ec.s)	to double-precision floating reg
	st	e9,4(ec.s)
	ais	ed,8
	ais	ec.s,8
	cli	ec.s,64+32(r4)	finished?  r4 = fwa of sp fp regs
	bl	lmd.10		not quite
	b	errfree

*--------------store multiple double-precision floating


stmd.00	equ	*

*--------------stmd loop

stmd.10	l	e8,0(ec.s)	move double-precision floating reg
	l	e9,4(ec.s)
	st	e8,0(ed)	to memory
	st	e9,4(ed)
	ais	ec.s,8
	ais	ed,8
	cli	ec.s,64+32(r4)	done?
	bl	stmd.10		not yet
	b	errfree
	title	overflow and underflow
*--------------overflow

overfld	li	e8,y'7fffffff'	largest possible no
	li	e9,y'ffffffff'
	ais	ee.stat,4	set v flag
	or	e8,ed		get an appropriate sign
	bm	stmd		-ve
	b	stpd		+ve

*--------------underflow

underfld ais	ee.stat,4	set v flag
	xr	e8,e8		zeroise the number
	xr	e9,e9
	b	stad
	title	normalization
*--------------normalization of double-precision floating


normlize equ	*
	ti	e8,y'ffffff'	test if a1=zero
	bz	norm.10
	lhi	ed,-1
	lr	ea,e8		get exponent
	ni	e8,y'ffffff'	separate fraction
	xr	ea,e8		separate sign & exponent
norm.05	si	ea,y'1000000'	decrement exponent
	btc	12,underfld	c or v set - underflow
	slls	e8,4		a1
	rll	e9,4		a2
	slls	ed,4		mask
	ti	e8,y'f00000'
	bz	norm.05		needs more ...
	lr	eb,e9		save a2
	nr	e9,ed		result2
	xr	eb,e9		separate most significant of old a2
	or	e8,eb		fraction a1
	ar	e8,ea		attach sign and exponent
	bm	stmd		-ve
	b	stpd		+ve not zero

*--------------a1 fraction zero

norm.10	lr	e9,e9		a2 ? 0
	bz	stzd		forced x'0000000000000000'
	si	e8,y'6000000'	cater for 6 x digits from a1
	btc	12,underfld	c or v set - underflow
	rrl	e9,8
norm.15	ti	e9,y'f00000'	potentially normalized?
	bnz	norm.20		yes, branch
	si	e8,y'1000000'	decrement exponent
	btc	12,underfld	c or v set - underflow
	rll	e9,4
	b	norm.15
norm.20	lr	ea,e9		normalization's terminated
	ni	e9,y'ff000000'	a2
	xr	ea,e9		most significant of old a2
	ar	e8,ea		a1
	bm	stmd		-ve
	b	stpd		+ve not zero
	title	compare double-precision floating
*--------------compare double-precision floating


cd	equ	*
	nhi	ee.stat,-16	clear current condition code
	l	ea,0(ed)	b1
	bnm	cd.10		b is +ve
	l	e8,0(ec.s)	a1
	bm	cd.30		a -ve, b -ve
cd.5	ais	ee.stat,2	a > b , set flag g
	b	dpfinal
*
cd.10	l	e8,0(ec.s)	a1
	bnm	cd.20		a is +ve
cd.15	ais	ee.stat,9	a < b , set flags l & c
	b	dpfinal
*
cd.20	clr	e8,ea		a ? b , both identical signs
	bc	cd.15		a < b
	btc	3,cd.5		a > b
	l	e9,4(ec.s)	a2, a1 = b1
	cl	e9,4(ed)	a2 ? b2
	bc	cd.15		a2 < b2, a < b
	btc	3,cd.5		a > b
	b	dpfinal
cd.30	clr	e8,ea		a -ve, b -ve
	bc	cd.5		b<a
	btc	3,cd.15		b>a
	l	e9,4(ec.s)
	cl	e9,4(ed)
	bc	cd.5		b2<a2
	btc	3,cd.15		b>a
	b	dpfinal
	title	add and subtract double-precision floating
*--------------add double-precision floating


ad	equ	*
	nhi	ee.stat,-16	clear current condition code
	l	eb,4(ed)	b2
	l	ea,0(ed)	b1
	b	adsd.00

*--------------subtract double-precision floating


sd	equ	*
	nhi	ee.stat,-16	clear current condition code
	l	eb,4(ed)	b2
	l	ea,0(ed)	b1
	xi	ea,y'80000000'	reverse sign

*--------------add and subtract common sequence

adsd.00	l	e9,4(ec.s)	a2
	l	e8,0(ec.s)	a1
	st	ec.s,source
	exhr	ec,e8		a's exponent
	exhr	ed,ea		b's exponent
	nhi	ec,x'7f00'	value of a's exponent
	nhi	ed,x'7f00'	value of b's exponent
	sr	ec,ed
	bz	adsd.60		exponents are equal
	bm	adsd.70		a<b   (magnitudes)
	clhi	ec,x'e00'	a>b     (magnitudes)
	bnl	adsd.55		a>>b    (magnitudes)

*--------------a > b

adsd.03	srls	ec,6
	st	ea,expb		b's exponent
	shi	ec,24		test if shifted more or less than 6x
	bnm	adsd.40		shift 6 or more hex digits
	ni	ea,y'ffffff'	b1
	ahi	ec,24
	srl	eb,0(ec)
	rrl	ea,0(ec)
	lhi	ed,-1		construct a mask
	srl	ed,0(ec)
	lr	ec,ea
	nr	ea,ed		adjusted b1
	xr	ec,ea		t'least signif dig of original b1
	or	eb,ec		new b2
adsd.05	lr	ec,e8		preserve a's exponent
	ni	e8,y'ffffff'
	xr	ec,e8		clear a1 fraction
adsd.12	st	ec,exp		preserve result's exponent
	x	ec,expb		effective + or -
	bnm	adsd.25		effective add

*--------------effective subtraction

	l	ec.s,source	restore source address
	sr	e9,eb		a2-b2=r2
	bnc	adsd.15
	sis	e8,1		a1-1:=a1
adsd.15	sr	e8,ea		a1-b1=r1
	bz	adsd.20		r1=0
adsd.17	o	e8,exp		merge result and exponent
	ti	e8,y'f00000'
	bz	normlize	normalize the result
	b	ld.50		store the result
adsd.20	lr	e9,e9
	bz	stad		result is zero
	b	adsd.17

*--------------effective addition

adsd.25	l	ec.s,source	restore source address
	ar	e9,eb		a2+b2=r2
	bnc	adsd.30
	ais	e8,1		a1+1:=a1
adsd.30	ar	e8,ea		a1+b1=r1
	bz	stzd		result is zero
	l	ed,exp
	ti	e8,y'f000000'
	bnz	adsd.35
	or	e8,ed		no change in exponent 's value
	bm	stmd
	b	stpd
adsd.35	ai	ed,y'1000000'	increase exponent's value
	btc	12,adsd.85	overflow
	srls	e9,4
	rrl	e8,4
	lr	eb,e8
	ni	e8,y'ffffff'	result1 (r1)
	xr	eb,e8		the least significant digit of r1
	or	e9,eb		result2 (r2)
	or	e8,ed		add exponent
	bm	stmd
	b	stpd

*--------------a>b, continued 1

adsd.40	srl	eb,24		6 or more hex digits difference
	slls	ea,8		b1
	or	eb,ea		new b2
	xr	ea,ea		zero to b1
	lr	ec,ec
	bz	adsd.05		no more adjustments to b1
	srl	eb,0(ec)	new adjusted b
	b	adsd.05

*--------------a >> b

adsd.55	l	ec.s,source
	lr	e8,e8
	bm	stmd
	b	stpd

*--------------a's exp = b's exp

adsd.60	st	ea,expb		preserve b1
	lr	ec,e8		a's sign and exponent
	ni	ea,y'ffffff'	b1 - strip off sign & exponent
	ni	e8,y'ffffff'	a1
	xr	ec,e8		result's exponent
	clr	e8,ea		a compared to b
	bc	adsd.65		a < b
	btc	3,adsd.12	a > b
	clr	e9,eb
	bnc	adsd.12		a > or = b
adsd.65	lr	ea,e8		a1 to b1
	l	e8,expb		b1 to a1
	lr	ed,e9		a2 to save
	lr	e9,eb		b2 to a2
	lr	eb,ed		a2 to b2
	st	ec,expb		a's exponent becomes b's
	lr	ec,e8		new a's exponent
	ni	e8,y'ffffff'	strip off the new exponent
	xr	ec,e8
	b	adsd.12

*--------------a < b

adsd.70	clhi	ec,-x'd00'
	bl	adsd.80		a<<b
	xhi	ec,-1
	ais	ec,1
	lr	ed,e8		exchange a and b
	lr	e8,ea
	lr	ea,ed
	lr	ed,e9
	lr	e9,eb
	lr	eb,ed
	b	adsd.03

*--------------a << b

adsd.80	l	ec.s,source
	lr	e9,eb
	lr	e8,ea
	bm	stmd
	b	stpd
*
adsd.85	si	ed,y'1000000'	overflow
	b	overfld
	title	multiply double-precision floating
*--------------multiply double-precision floating
md	equ	*
	nhi	ee.stat,-16	clear current condition code
	l	e9,4(ec.s)	a2
	l	e8,0(ec.s)	a1
	bz	stzd		if zero exit
	l	eb,4(ed)	b2
	l	ea,0(ed)	b1
	bz	stzd		if zero exit
	lr	ed,e8
	ni	ed,y'ff000000'	zero to a1 fraction
	ar	ed,ea		a-exp + b-exponent
	bc	md.04		if c=v go to md.08
	bno	md.08
	b	md.06
md.04	bo	md.08

*--------------carry out of exponent field

md.06	ti	ed,y'40000000'	test for overflow
	bz	md.10
	si	ed,y'1000000'	test if potentially o.k.
	ti	ed,y'40000000'
	bnz	md.75		no, overflow
	ni	ea,y'ffffff'	potential overflow
	xr	ed,ea		retain excess-128 notation and
	ais	ed,1		set potential overflow flag
	b	md.12

*--------------no carry out of exponent field

md.08	ti	ed,y'40000000'	must be gt or eq 64
	bz	underfld	underflow, < 64

md.10	ni	ea,y'ffffff'	clear b's exponent
	xr	ed,ea		retain result of exp. manipulation
md.12	si	ed,y'40000000'	restore excess-64 notation
	ni	e8,y'ffffff'	clear a's exponent
	stm	ec.s,source	save: ec...source address
*                                        ed...exponent with potential
*                                             overflow flag on/off
*                                        ee,ef...the instruction's psw
*                                  multiply both a and b by 16 in order
*                                       to utilize the most significant
*                                       digits of a2 and b2
	slls	e8,4		a1
	slls	ea,4		b1
	rll	e9,4		a2
	rll	eb,4		b2
	lr	ec,e9
	lr	ed,eb
	nhi	ec,x'f'		retain f7  of a
	nhi	ed,x'f'		retain f7  of b
	ar	e8,ec		seven x digits in a1
	ar	ea,ed		7 x digits     in b1
	xr	e9,ec		clear extra digit in a2 & b2
	xr	eb,ed
	lr	ed,e9		a2
	bm	md.50		bit 0 set
	lr	ef,eb		b2
	bm	md.50		bit 0 set

*--------------all four pseudo operands are positive numbers

	mr	ec,eb		a2*b2 = c
	mr	ee,e8		b2*a1 = d
	lr	ed,ec		forget the irrelevant c2
	lr	ec,ee		d1
	ar	ed,ef		c1+d2 := d2
	bnc	md.15
	ais	ec,1
md.15	lr	ef,e9		a2
	mr	ee,ea		a2*b1 = e
	ar	ec,ee		d1+e1 := d1, no carry possible
	ar	ed,ef		d2+e2 := d2
	bnc	md.20
	ais	ec,1
md.20	lr	ef,e8		a1
	mr	ee,ea		a1*b1 = f
	ar	ef,ec		d1+f2 = r2
	bnc	md.30		f1 = r1
	ais	ee,1		f1+1 = r1

*-------------result constructed in e8 & e9

md.30	l	e8,exp		restore exponent
	ti	ee,y'f00000'	1-st x digit
	bz	md.40		not normalized
	thi	e8,1		test potential overflow flag
	bnz	md.70		overflow
	ar	e8,ee
	lr	e9,ef
*                                  restore saved values
md.35	lm	ee.stat,status
	l	ec.s,source
	b	ld.50		test if -ve or +ve

*--------------normalize the result of multiplication

md.40	thi	e8,1		test the potential overflow flag
	bz	md.42		no overflow detected earlier
	sis	e8,1		no test for underflow,
*                                   no further exponent adjustment,
*                                  just clear the flag
	b	md.43
md.42	si	e8,y'1000000'	result is to be normalizedp
	btc	12,md.45	underflow
md.43	slls	ee,4		r1*16 := r1
	ar	e8,ee		r1+exponent
	rll	ef,4		r2*16 := r2
	lr	e9,ef		r2
	nhi	ef,x'f'		retain the most sign digit of r2
	xr	e9,ef		clear rubbish
	ar	e8,ef		most sign digit of r2 times 16
	rll	ed,4		bring in another hex digit
	lr	ef,ed
	nhi	ef,x'f'
	or	e9,ef
*                                  restore saved values
	b	md.35
md.45	lm	ee.stat,status	underflow due to normalization
	l	ec.s,source
	b	underfld

*--------------adjusted a2 or b2 is negative

md.50	srls	e9,4		a2/16
	srls	eb,4		b2/16
	lr	ed,e9		a2
	mr	ec,eb		a2*b2 = c
	slls	ec,4		c1*16 := c1
	rll	ed,4
	nhi	ed,x'f'		get the most sign.digit times 16
	ar	ed,ec		forget the rest
	lr	ef,eb		b2
	mr	ee,e8		b2*a1 = d
	lr	ec,ee		d1
	ar	ed,ef		c1+d2 := d2
	bnc	md.55
	ais	ec,1
md.55	lr	ef,e9		a2
	mr	ee,ea		a2*b1 = e
	ar	ec,ee		e1+d1 := d1, no chance of carry
	ar	ed,ef		e2+d2 := d2
	bnc	md.60
	ais	ec,1
md.60	slls	ec,4		d1*16
	rll	ed,4		d2*16
	lr	eb,ed		bring in another hex digit
	nhi	eb,x'f'
	or	ec,eb
	lr	ef,e8		a1
	mr	ee,ea		a1*b1 = e
	ar	ef,ec		e2+d1 = r2
	bnc	md.30		r1
	ais	ee,1		r1+1 := r1
	b	md.30
md.70	lm	ee.stat,status	overflow
	l	ec.s,source
	lr	ed,e8		get the sign source
	b	overfld
*
md.75	xr	e8,ea		make up the sign
	lr	ed,e8
	b	overfld
	title	divide double-precision floating
*--------------divide double-precision floating


dd	equ	*
	nhi	ee.stat,-16	clear current condition code
	l	ea,0(ed)	divisor b1
	bz	dd.70		divide by zero requested
	l	eb,4(ed)	b2
	l	e9,4(ec.s)	a2
	l	e8,0(ec.s)	a1
	bz	stzd		result is zero
	lr	ed,e8		get exponent
	oi	ed,y'ffffff'	eliminate any chance of borrow
	sr	ed,ea		subtract exponents
	bc	dd.04
	bno	dd.08
	b	dd.06
dd.04	bo	dd.08

*--------------borrow out of exponent field

dd.06	ti	ed,y'40000000'	check for underflow
	bnz	dd.10
	ai	ed,y'1000000'	test if potentially o.k.
	ti	ed,y'40000000'
	bz	underfld	definitely underflow
	ni	ed,y'ff000000'	pure exponent  in ed
	ais	ed,1		set potential underflow flag
	b	dd.12

*--------------no borrow out of exponent field

dd.08	ti	ed,y'40000000'	test for overflow
	bnz	dd.75		yes, overflow
dd.10	ni	ed,y'ff000000'
dd.12	ai	ed,y'40000000'	resultant exponent
*                                      with the potential underflow
*                                      flag set or reset
	ni	e8,y'ffffff'	clear a's exponent
	ni	ea,y'ffffff'	clear b's exponent
	stm	ec.s,source	save: ec...source address
*                                        ed...exponent with potential
*                                             underflow flag on/off
*                                        ee,ef...the instruction's psw
	xr	ee,ee		zero to result1
	xr	ef,ef		zero to result2
	li	ec,y'1000000'
*                                  divide by repeated subtraction.
*                                       hexadecimal digits are basic
*                                       sources for decision-making and
*                                       the arithmetics
	clr	e8,ea		a1 ? b1
	bc	dd.30		a1 < b1
	btc	3,dd.20		a1 > b1
	clr	e9,eb		a2 ? b2
	bc	dd.30		a2 < b2
	btc	3,dd.20		a2 > b2
	lr	ee,ec		a=b, result's obvious
	b	dd.40

*--------------a>b

dd.20	sr	e9,eb		current a2-b2 = c2
	bnc	dd.22
	sis	e8,1		a1-1 := a1, no chance of borrow
dd.22	sr	e8,ea		current a1-b1 = c1
	ar	ef,ec
	clr	e8,ea		a1 ? b1
	bc	dd.30		a1<b1
	btc	3,dd.20		a1>b1
	clr	e9,eb		a2?b2
	bnc	dd.20		a2> or = b2

*--------------a<b

dd.30	srls	ec,4		divide the adder by 16
	btc	3,dd.35		there is a value in the adder
	lr	ee,ee		end of cycle, adder = 0
	bnz	dd.40		end of 2nd cycle
	lr	ee,ef
	xr	ef,ef
	li	ec,y'10000000'

*--------------next hexadecimal digit

dd.35	slls	e8,4		a1*16
	rll	e9,4		a2*16
	lr	ed,e9		get the most signif digit of a2
	ni	e9,y'fffffff0'
	xr	ed,e9		separate current f7 of a2
	ar	e8,ed
	clr	e8,ea		a1 ? b1
	bc	dd.30		a1<b1
	btc	3,dd.20		a1>b1
	clr	e9,eb		a2?b2
	bnc	dd.20		a2 > or = b2
	b	dd.30

*--------------result build-up

dd.40	l	e8,exp
	ti	ee,y'f000000'	does it need normalization ?
	bnz	dd.45		yes
	thi	e8,1		test the potential underflow flag
	bnz	dd.80		underflow
	b	dd.50		and off we go...
dd.45	rrl	ee,4		normalize, r1/16
	srls	ef,4		r2/16
	lr	ed,ee		separate the least signif digit of r
	ni	ee,y'ffffff'
	xr	ed,ee
	ar	ef,ed
	thi	e8,1		test the potential underflow flag
	bz	dd.55		not set - increase exponent's value
	sis	e8,1		reset the flag
dd.50	or	e8,ee		everything's o.k.
	lr	e9,ef
	lm	ee.stat,status
	l	ec.s,source
	lr	e8,e8
	bm	stmd
	b	stpd
dd.55	ai	e8,y'1000000'	increase the exponent value
	bfc	12,dd.50	it is not an overflow
*                                  overflow
dd.60	lm	ee.stat,status
	l	ec.s,source
	l	ed,exp
	b	overfld
*                                  division by zero
dd.70	ais	ee.stat,12	c and v flags
	b	dpfinal
dd.75	xr	e8,ea
	lr	ed,e8
	b	overfld
*                                  underflow
dd.80	lm	ee.stat,status
	l	ec.s,source
	b	underfld
	title	float register - double-precision
*--------------fldr pre-process


fldr.00	equ	*
	nhi	ed,x'f'		r2 is general register
	lb	ed,grtab(ed)	offset from user's saved r8
	l	ed,0(r3,ed)	contents of general register
*
*	note: in this case ed is the contents not address of r2
*
	nhi	ec.s,x'38'	r1 is float register
	ai	ec.s,32(r4)	address of double fp reg r1
*--------------float register - double precision (convert to real)


fldr	equ	*
	nhi	ee.stat,-16	clear current condition code
	lr	ea,ed		get the number to float
	bz	stzd
	bm	fldr.20		-ve
	li	e8,y'48000000'	starter exponent

*--------------+ve and -ve common process

fldr.10	rrl	ea,8
	lr	e9,ea		two least significant x digits
	ni	e9,y'ff000000'	result2
	xr	ea,e9		6 most significant x digits
	or	e8,ea		result1
	ti	e8,y'f00000'	needs normalization?
	bz	normlize
	lr	e8,e8
	bm	stmd
	b	stpd

*--------------negative integer

fldr.20	xhi	ea,-1		complement the bit pattern
	ais	ea,1		complement the -ve number
	li	e8,y'c8000000'	starter exponent - negative mantissa
	b	fldr.10
	title	fix register - double-precision
*--------------fix register double-precision (convert to integer)


fxdr	equ	*
	nhi	ee.stat,-16	clear current condition code
	slls	ed,2		multiply by 4 to get floating reg
	nhi	ed,x'38'	force it on & bytes boundary
	ai	ed,32(r4)	add fwa of double precision regs
	l	e8,0(ed)	a1
	l	e9,4(ed)	a2
	exhr	ea,e8
	nhi	ea,x'7f00'	separate exponent
	shi	ea,x'4000'	is there an integer part?
	bnp	fxdr.30		no
	srls	ea,8		pure exponent
	sis	ea,8
	bp	fxdr.25		number is too big
	xhi	ea,-1		complement the bit pattern
	ais	ea,1		complement the number
	ti	e8,y'800000'	is the number potentially too big?
	bnz	fxdr.20		yes

*--------------final conversion to integer

fxdr.05	srl	e9,24		move two least significant x digits
	lr	eb,e8
	slls	eb,8		adjust 6 most signif x digits
	or	eb,e9		construct number's magnitude
	lr	ea,ea		examine effective exponent
	bz	fxdr.07		no adjustment required
	slls	ea,2		multiply by 4 to get no of bits
	srl	eb,0(ea)	result's magnitude
fxdr.07	lr	e8,e8
	bp	fxdr.15		+ve result
fxdr.08	equ	*
	xhi	eb,-1		complement the bit pattern
	ais	eb,1		complement the number
fxdr.10	ais	ee.stat,1	-ve result, set l flag
	b	fxdr.6
fxdr.15	ais	ee.stat,2	+ve result, set g flag
fxdr.17	b	fxdr.6		go store result in general reg.

*--------------exceptional values for conversion

fxdr.20	lr	ea,ea		test pseudo exponent
	bnz	fxdr.05		it will fit in a user register

*--------------number is too big

fxdr.25	ais	ee.stat,4	set v flag
	li	eb,y'7fffffff'	value as big as possible
	lr	e8,e8
	bm	fxdr.08
	b	fxdr.15		+ve result

*--------------number too small or zero

fxdr.30	xr	eb,eb		zeroise

fxdr.6	equ	*
*
	srls	ec.s,2		was r1 * 4
	lb	r1,grtab(ec.s)	offset from users saved r8
	st	eb,0(r3,r1)	put into users saved general reg
*
	b	nofault

	endc

	title	vector tables
vectab1	dac	flt.iih		illegals all have a code of 0
	ifnz	SPFPT		single-precision
	dac	xer,xer,xer,xer,xer,xer rr floating
	dac	y'80000000'+fxr	fxr
	dac	y'80000000'+flr.1 flr
	dac	xes		rx floating (store)
	dac	xe,xe,xe,xe,xe,xe rx floating
	dac	y'80000000'+xes	stme
	dac	y'80000000'+xe	lme
	endc
	ifz	SPFPT
	do	17
	dac	flt.iih
	endc
lv	equ	*-vectab1
	ifnz	DPFPT		double-precision
	dac	xdr,xdr,xdr,xdr,xdr,xdr rr floating
	dac	y'80000000'+fxdr fxdr
	dac	y'80000000'+fldr.00 fldr
	dac	xds		rx floating (store)
	dac	xd,xd,xd,xd,xd,xd rx floating
	dac	y'80000000'+xds	stmd
	dac	y'80000000'+xd	lmd
	endc
	ifz	DPFPT
	do	17
	dac	flt.iih
	endc
*--------------second level vector table

vectab2	dac	flt.iih		shouldn't ever get here
	ifnz	SPFPT		single-precision
	dac	le,ce,ae,se,me,de rr floating
	dac	flt.iih		fxr
	dac	flr.2		flr
	dac	y'80000000'+ste	ste
	dac	le,ce,ae,se,me,de rx floating
	dac	y'80000000'+stme stme
	dac	lme		lme
	endc
	ifz	SPFPT
	do	17
	dac	crash
	endc
	ifnz	DPFPT		double-precision
	dac	ld,cd,ad,sd,md,dd rr floating
	dac	flt.iih		fxdr
	dac	fldr		fldr
	dac	y'80000000'+std	std
	dac	ld,cd,ad,sd,md,dd rx floating
	dac	y'80000000'+stmd.00 stmd
	dac	lmd		lmd
	endc
	ifz	DPFPT
	do	17
	dac	crash
	endc
	title	operation code table
*--------------opcode table
	align	4
opcodes	do	5
	db	0,0,0,0,0,0,0,0
	ifnz	SPFPT		single-precision
	db	4,8,12,16,20,24,28,32
	else
	db	0,0,0,0,0,0,0,0
	endc
	db	0,0,0,0,0,0,0,0
	ifnz	DPFPT		double-precision
	db	lv,lv+4,lv+8,lv+12,lv+16,lv+20,lv+24,lv+28
	else
	db	0,0,0,0,0,0,0,0
	endc
	do	4
	db	0,0,0,0,0,0,0,0
	ifnz	SPFPT		single-precision
	db	36,0,0,0,0,0,0,0,40,44,48,52,56,60,0,0
	else
	do	2
	db	0,0,0,0,0,0,0,0
	endc
	ifnz	DPFPT		double-precision
	db	lv+32
	else
	db	0
	endc
	ifnz	SPFPT		single-precision
	db	64,68
	else
	db	0,0
	endc
	db	0,0,0,0,0
	ifnz	DPFPT		double-precision
	db	lv+36,lv+40,lv+44,lv+48,lv+52,lv+56,lv+60,lv+64
	else
	db	0,0,0,0,0,0,0,0
	endc
	do	16
	db	0,0,0,0,0,0,0,0
	title	constants and work areas
*--------------work areas
*                                  used by:  ad,sd
*--------------work areas
*
*		offsets from users saved r8 of users 
*		general registers.
*		see reg.h for additional info
*		we cant use reg.h exactly because offsets
*		must be +ve
*
grtab	equ	*
	db	10*adc		r0
	db	11*adc		r1
	db	12*adc		r2
	db	13*adc		r3
	db	14*adc		r4
	db	15*adc		r5
	db	16*adc		r6
	db	17*adc		r7 - sp
	db	0*adc		r8
	db	1*adc		r9
	db	2*adc		r10
	db	3*adc		r11
	db	4*adc		r12
	db	5*adc		r13
	db	6*adc		r14
	db	18*adc		r15


lsflag	dc	h'0'		load/store type instruction
laflag	db	0		look ahead flag
*
	align	4
source	das	1		save area for source address
*                                  used by:  ad,sd,md,dd
exp	das	1		save area for result's exponent
*                                  used by:  ad,sd,md,dd
status	das	2		save area for the user's psw
*                                  used by: md,dd
***************the order of source, exp & status should not be changed
expb	das	1		auxiliary area for b's exponent
*                                  used by:  ad,sd
	end
