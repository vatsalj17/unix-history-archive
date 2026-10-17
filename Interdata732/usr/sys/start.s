	title	7/32 unix hardware support routines

*
* conditional assembly switches
*
PIC	equ	0	if 1, use precision interrupt clock timing
*				(must also be defined in low.s)
LRA	equ	0	if 1, use hardware 'lra' instruction for address
*				translation (8/32 only)

	entry	u
	entry	start,idle
	entry	initf,icode
	entry	fuword,fubyte,fuiword,fuibyte
	entry	suword,subyte,suiword,suibyte
	entry	savu,retu,aretu
	entry	copyin,copyout,clearseg,copyseg
	entry	incupc
	entry	spl
	entry	ss,oc,wd,wh,wdh,rd,rh,rdh
	entry	lra,lraddr
	extrn	uisa0,kisa0,ka6
	extrn	memtop
	extrn	ksp
	extrn	addrsw
	extrn	main
	extrn	end,edata
    ifnz	PIC
	extrn	stclk.a,stclk.b,sytim
    endc
*
* psw bit definitions
*
ps.wait equ	x'8000'	wait state
ps.io	equ	x'4000'	immediate interrupt mask
ps.mm	equ	x'2000'	machine malfunction interrupt mask
ps.af	equ	x'1000'	arith fault interrupt mask
ps.il	equ	x'0800'	interrupt levels (8/32)
ps.rp	equ	x'0400'	memory relocation / protection
ps.sq	equ	x'0200'	system queue service mask
ps.prot equ	x'0100'	protect mode
ps.ureg equ	x'00f0'	user register set
*
* psw definitions
*
ps.user equ	ps.io+ps.mm+ps.af+ps.rp+ps.prot+ps.ureg
ps.idle equ	ps.wait+ps.io+ps.mm+ps.af+ps.rp+ps.ureg
ps.kern equ	ps.io+ps.mm+ps.af+ps.rp+ps.ureg
ps.disb equ	ps.mm+ps.af+ps.rp+ps.ureg
ps.trap equ	ps.mm+ps.af
*
* reserved memory location definitions
*
isp	equ	x'd0'	interrupt-service pointer table
mac.r0	equ	x'300'	memory-access controller seg regs
*
* register definitions
*
r0	equ	0
r1	equ	1
r2	equ	2
r3	equ	3
r4	equ	4
r5	equ	5
r6	equ	6
r7	equ	7
r8	equ	8
r9	equ	9
r10	equ	10
r11	equ	11
r12	equ	12
r13	equ	13
r14	equ	14
r15	equ	15
rf	equ	15
sp	equ	r7
*
* per-process data area for current process - always mapped to segment e
*
u	equ	y'e0000'
usize	equ	6
	title	start -- 7/32 unix system initialization
	pure
start	equ	*
*
* on entry from bootstrap loader, disable mac & interrupts
*
	lhi	r1,ps.ureg	disable everything
	epsr	r0,r1
*
* clear per-process data area and bss to zeroes
*
	lis	r0,0
	la	r5,end		end of kernel
	ahi	r5,255		round up to 256 byte block
	nhi	r5,-256
	la	r2,256*usize(r5)	end of u area
	la	r1,edata		start of bss
clp	equ	*
	st	r0,0(r1)	clear a word
	ais	r1,4		next word
	cr	r1,r2		finished?
	bl	clp		no - repeat
*
* Find out how much memory is available
*
	lr	r1,r2		end of u area
	li	r2,y'fedcba98'	test data value
memlp	equ	*
	st	r2,0(r1)	try storing word
	c	r2,0(r1)	did it work?
	bne	memend		no - end of memory
	ahi	r1,256		yes - try next 256 byte block
	c	r1,memtop	reached upper limit?
	bl	memlp		no - keep looking
memend	equ	*
	sis	r1,1		highest valid address
	st	r1,memtop	save it
	ais	r1,1
	srl	r1,8		max memory (in 256-byte blocks)
*
* initialize prototype mac registers for kernel address space:
*
* segments 0-n: map real low-core addressess (up to n*64k)
*           e : maps per-process data area for current process
* all others  : invalid
*
	la	r2,kisa0	kernel seg regs
	li	r0,y'0ff00010'	prototype seg reg value
seglp	equ	*
	chi	r1,256		more than full segment left?
	bm	seglast		no - set last segment
	st	r0,0(r2)	set seg reg
	ais	r2,4
	ai	r0,y'10000'	origin of next segment
	shi	r1,256		memory left
	b	seglp
*
seglast equ	*
	sis	r1,1		last segment length - 1
	bm	segx		no partial segment - skip
	sll	r1,20		move to seg length field
	ni	r0,y'fffff'
	or	r0,r1
	st	r0,0(r2)	set seg reg
	ais	r2,4
*
segx	equ	*
	l	r2,ka6		per-process data segment
	lr	r0,r5		address of end of kernel
	oi	r0,usize*y'100000'+x'10'	size & protection
	st	r0,0(r2)	set as segment for pda
*
* start kernel stack pointer at top of per process area
*
	la	sp,256*usize+u-4
	st	sp,ksp		save sp for interrupts
*
* load hardware mac registers from kernel prototypes, and enable
* memory relocation / protection and interrupts
*
	la	r1,kisa0	kernel seg regs
	bal	r6,addrsw	load into mac regs
	li	r1,ps.kern	enable mac & interrupts
	epsr	r0,r1
*
* call main() c routine to complete initialization
*
	bal	rf,main
*
* on return from main, enter 'user state' at user address 0 to exec
*  init process
*
	epsr	r3,r3		disable the mac
	nhi	r3,x'ffff'-ps.rp
	epsr	r0,r3
	la	r1,uisa0	user seg regs
	bal	r6,addrsw	switch address space
	lhi	r0,ps.user	user state psw
	lis	r1,0		address 0 in user space
	lpswr	r0		enter process 1
	title	bootstrap program to load in init process
*
*	exec("/etc/init", "/etc/init");
*	for (;;)
*		;
*
	align	adc
icode	equ	*
	svc	14,11		* exec *
	dc	a(initf-icode)
	dc	a(initp-icode)
	b	*		didn't work -- loop forever
initp	dc	a(initf-icode)
	dc	0
* string '/etc/init' -- must be in hex to avoid uppercase xlate
initf	db	x'2f',x'65',x'74',x'63'
	db	x'2f',x'69',x'6e',x'69',x'74',0
* extra patch space in case of emergency
	db	0,0,0,0,0,0,0,0,0,0
	db	*
	title	7/32 unix -- inter-address-space data transfer routines
*
* fubyte(i) - fetch a byte from user address <i>
* fuword(i) - fetch a word from user address <i>
* subyte(i,a) - store byte <a> at user address <i>
* suword(i,a) - store word <a> at user address <i>
*
*   - if <i> is an illegal address (or write-protected for subyte or suword)
*     the routine returns (-1).
*
fubyte	equ	*
fuibyte	equ	*
	l	r1,0(sp)	user address
	bal	r6,lrau		relocate
	btc	x'c',illaddr	illegal address
	lb	r0,0(r1)	fetch byte
	br	rf
*
fuword	equ	*
fuiword	equ	*
	l	r1,0(sp)	user address
	bal	r6,lrau		relocate
	btc	x'c',illaddr	illegal address - exit
	lhl	r0,0(r1)	fetch word as 2 halfwords
	exhr	r0,r0		... in case not on word boundary
	lhl	r1,2(r1)	... (8/32)
	or	r0,r1
	br	rf
*
subyte	equ	*
suibyte	equ	*
	l	r1,0(sp)	user address
	bal	r6,lrau		relocate
	btc	x'e',illaddr	illegal or protected address - exit
	l	r0,4(sp)	byte to be stored
	stb	r0,0(r1)	store it
	lis	r0,0		normal return
	br	rf
*
suword	equ	*
suiword	equ	*
	l	r1,0(sp)	user address
	bal	r6,lrau		relocate
	btc	x'e',illaddr	illegal or write protected - exit
	l	r0,4(sp)	word to be stored
	sth	r0,2(r1)	store it as 2 halfwords
	exhr	r0,r0		... in case not on word boundary
	sth	r0,0(r1)	... (8/32)
	lis	r0,0	normal return
	br	rf
*
* copyin (uaddr, kaddr, n)
* copyout (kaddr, uaddr, n)
*	- copy <n> bytes from/to user address <uaddr> to/from kernel
*	  address <kaddr>
*
* - <n> must be a multiple of wordsize
*
copyin	equ	*
	l	r1,0(sp)	user source address
	bal	r6,lrau
	btc	x'c',illaddr	illegal address - exit
	l	r2,4(sp)	kernel target address
	b	copy
*
copyout	equ	*
	l	r1,4(sp)	user target address
	bal	r6,lrau
	btc	x'e',illaddr	illegal address - exit
	lr	r2,r1
	l	r1,0(sp)	kernel source address
copy	equ	*
	l	r4,8(sp)	no. of bytes
	sis	r4,4		start at last word
copylp	equ	*
	l	r0,0(r1,r4)	move a word
	st	r0,0(r2,r4)
	sis	r4,4		back up
	bnm	copylp		repeat
*
	lis	r0,0
	br	rf
*
* Illegal or write-protected address - return (-1)
*
illaddr	equ	*
	lcs	r0,1	return -1
	br	rf
	eject
*
* clearseg(i) : clear block number <i> in kernel space to zeroes
*
clearseg	equ	*
	l	r1,0(sp)	block no.
	sll	r1,8		block origin address
	lis	r0,0
	lhi	r2,63*adc	start at last word
clearlp	equ	*
	st	r0,0(r1,r2)	clear a word
	sis	r2,4		back up
	bnm	clearlp		repeat
	br	rf
*
* copyseg (i,j) : copy kernel block number <i> into kernel block <j>
*
copyseg	equ	*
	l	r2,4(sp)	target block no.
	sll	r2,8		block origin
	l	r1,0(sp)	source block no.
	sll	r1,8
	lhi	r3,63*adc	start at last word
copyslp	equ	*
	l	r0,0(r1,r3)	move a word
	st	r0,0(r2,r3)
	sis	r3,4		back up
	bnm	copyslp		repeat
	lis	r0,0
	br	rf
	title	7/32 unix -- user-kernel address space mapping
*
* lraddr - C interface to lra
*
* lraddr(aaddr, seg)
* char **addr;		/* pointer to unrelocated address */
* int seg[];		/* pointer to copy of MAC segmentation registers */
*
*	- replaces *addr by relocated address
*	- returns condition code from lra instruction
*
lraddr	equ	*
	l	r5,0(sp)	pointer to virtual addr
	l	r1,0(r5)	virtual address
	l	r2,4(sp)	pointer to seg regs
	bal	r6,lra		relocate
	st	r1,0(r5)	change virtual to real address
	nhi	r0,15		isolate cond code in new psw
	br	rf
* lra: load 'real' address
*  input:	r1 - address in user program space
*		r2 - address of segmentation regs
*  output:	r1 - corresponding address in kernel space
*		cc - condition code set as in real lra instruction
* uses:		r0
*
* lrau : entry point using user seg regs (uisa0)
*
lrau	equ	*
	la	r2,uisa0	use uisa0 for seg regs
lra	equ	*

    ifnz	LRA
	lra	r1,0(r2)	use hardware translation
	br	r6
    else
	lr	r0,r1		save input addr
	srl	r1,14		seg no. * 4
	ni	r1,x'3c'	clear extra bits
	l	r1,0(r1,r2)	user seg reg
	thi	r1,x'10'	check 'present' bit
	bz	lrabad		zero - illegal address
	lr	r2,r1
	srl	r2,12		seg limit * 256
	ahi	r2,x'100'	seg length in bytes
	ni	r2,y'fff00'	clear extra bits
	ni	r0,y'ffff'	offset in segment
	cr	r0,r2		within range?
	bnl	lrabad		no - illegal address
	lis	r2,0
	thi	r1,x'60'		write-protected?
	bz	lranwp		no - skip
	lis	r2,2		set G bit in cc
lranwp	equ	*
	ni	r1,y'fff00'	segment origin
	ar	r1,r0		add offset
	b	lraset
* unmapped address
lrabad	equ	*
	lis	r2,8		set C bit
* set condition code
lraset	equ	*
	epsr	r0,r0		current psw
	nhi	r0,-16		mask off old cond code
	or	r0,r2		set new cond code
	epsr	r2,r0		set new psw
	br	r6
    endc

	title	7/32 unix -- kernel-process switching routines
*
* savu(a)
*	- save current kernel process stack environment
*	  in location <a>
*
savu	equ	*
	l	r1,0(sp)	location
	st	sp,0(r1)	save stack ptr
	st	r14,adc(r1)	save stack base
	br	rf
*
* retu(a)
*	- restore previously-saved kernel process stack environment
*	  from ppda at block number -- not address -- <a>
*	- reset kernel seg reg e to map to address <a>, thus
*	  switching the 'current process'
*
retu	equ	*
	l	r1,0(sp)	block number
	sll	r1,8		convert to address
	lr	r0,r1
	oi	r0,usize*y'100000'+x'10'	length = usize
	li	r3,ps.disb-ps.rp	disable the mac
	epsr	r2,r3
	l	r4,ka6		set as kernel ppd segment
	st	r0,0(r4)
	l	sp,0(r1)	restore stack ptr
	l	r14,adc(r1)	restore stack base
*
* switch to new ppd segment
*
	la	r1,kisa0	set new seg regs into 
	bal	r6,addrsw	hardware mac regs
	epsr	r0,r2		enable mac again
	br	rf
*
* aretu(a)
*	- restore kernel process stack environment from alternate
*	  location <a>, in the current per-process data area
*
aretu	equ	*
	l	r1,0(sp)	location
	l	sp,0(r1)	restore stack ptr
	l	r14,adc(r1)	restore stack base
	br	rf
	title incupc -- update execution profile buffer
*
* incupc (pc, prof)
* struct \(
*   int base;  int length;  int offset;  int scale;
* \) *prof;
*
*  - called from clock interrupt handler to update user execution profile
*
incupc	equ	*
	l	r4,4(sp)	base of prof
	l	r3,0(sp)	current program counter
	s	r3,8(r4)	subtract offset
	bmr	rf		< 0 : not within buffer
*
* scale pc into buffer
*
	m	r2,12(r4)	scale pc
	ais	r2,1		round to fullword
	slls	r2,1
	nhi	r2,x'fffc'
	c	r2,4(r4)	within buffer?
	bnlr	rf		no : return
	a	r2,0(r4)	address = base + offset
*
* get physical address of buffer location
*
	lr	r1,r2		program address
	bal	r6,lrau		get physical address
	btc	x'e',incupcx	illegal or write-protected -- error
*
* increment profile counter
*
	lis	r0,1
	am	r0,0(r1)	increment counter in buffer
	br	rf
*
* memory fault -- turn off profiling
*
incupcx equ	*
	lis	r0,0		zero prof scale
	st	r0,12(r4)	to turn off profiling
	br	rf
	title	7/32 unix - set processor level
*
* spl(n)
*
*  - sets 'processor level' to <n> (i.e. only devices with 
*    priority >n may interrupt
*  - returns previous value of processor level

*  note:
*    as currently implemented, spl(0) enables interrupts, and any
*    nonzero processor level disables all interrupts.  eventually
*    the pdp-11 processor level mechanism should be emulated more
*    closely, probably by using interdata system queue.
*
spl	equ	*
	lis	r0,0
	epsr	r1,r1		current psw status
	thi	r1,ps.io	immediate interrupts enabled?
	bnz	spl.new		yes - skip
	lis	r0,1		no - 'previous level' is 1
spl.new equ	*
	nhi	r1,x'ffff'-ps.io	mask off immediate interrupts
	l	r2,0(sp)	new processor level
	bnz	spl.epsr	nonzero - skip
	ohi	r1,ps.io	zero - enable interrupts again
spl.epsr equ	*
	epsr	r2,r1		load new psw status
	br	rf
	title	i/o instruction routines
ss	equ	*	sense status
	l	r1,0(sp)
	ssr	r1,r0
	br	rf
*
oc	equ	*	output command
	l	r1,0(sp)
	oc	r1,7(sp)
	br	rf
*
wd	equ	*	write data
	l	r1,0(sp)
	wd	r1,7(sp)
	br	rf
*
wh	equ	*	write halfword
	l	r1,0(sp)
	wh	r1,6(sp)
	br	rf
*
wdh	equ	*	write 3 bytes
	l	r1,0(sp)
	wd	r1,5(sp)
	wh	r1,6(sp)
	br	rf
*
rd	equ	*	read data
	l	r1,0(sp)
	rdr	r1,r0
	br	rf
*
rh	equ	*	read halfword
	l	r1,0(sp)
	rhr	r1,r0
	br	rf
*
rdh	equ	*	read 3 bytes
	l	r1,0(sp)
	rdr	r1,r2
	rhr	r1,r0
	exhr	r2,r2
	or	r0,r2
	br	rf
	title	'idle' routine -- go into enabled wait state
idle	equ	*
    ifnz	PIC
	bal	r6,ras		read and update system time
    endc
	li	r1,ps.idle	load 'wait' psw
	epsr	r0,r1
    ifnz	PIC
	bal	r6,ras		read and update sys time
    endc
	epsr	r1,r0		restore previous psw
	br	rf		return to caller

    ifnz	PIC
*
*	read and restart pic 
*	add current interval to sys time
*
ras	equ	*		read and start clock
	lhi	r2,x'6c'	addr of pic
	rhr	r2,r3		read interval
ras1	wh	r2,stclk.a		restart clock
	ssr	r2,r4
	btc	8,ras1
	oc	r2,stclk.b
	lhi	r5,x'fff'	max interval
	sr	r5,r3		real current clock interval
	am	r5,sytim	add to system time
	br	r6		return
    endc

	end	start
