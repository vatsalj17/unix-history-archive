.pdp70 = 4	/ size of fec/fea area
.mappedb = 2	/ mapped buffers in system (out=0,in=2)
/	ilj mods
/
/	mods to support shared data type a.outs and core images
/
/
/	andy mods
/
/	to enable patches of charachters,
/	i.e.	"qw! will place "qw" at the word addressed.
/
/ 	craig macgregor mods
/
/	print out a label before data if available
/
/	chrism mods		22/2/79
/	know about shared text and data a.outs
/
/	chrism mods		 9/3/79
/	know about separate i/d space cores & a.outs!
/	allow multiple ^'s
/
/
/ db1 -- debugger

fpp = 1
eae = 0
ldfps = 170100^tst


db:
	mov	sp,r5
	mov	(r5)+,r4
	tst	(r5)+
	cmp	r4,$2
	blt	1f
	mov	(r5),dbfile
	mov	(r5),odbfil
	mov	(r5)+,namfil
	cmp	r4,$2
	beq	1f
	mov	(r5)+,namfil
1:
	sys	open; dbfile: core; 0
	bec	9f
	jmp	fnfnd
9:
	mov	r0,dbfin
	clr	dbfout
	sys	open; odbfil:core; 1
	bes	1f
	mov	r0,dbfout
1:
	sys	open; namfil: a.out; 0
	bes	1f
	mov	r0,r1
	mov	r0,symfin
	sys	read; nambuf; 20
	cmp	nambuf,objmagic			/ilj 1/3/76
	blt	1f			/ilj 1/3/76
	cmp	nambuf,nobjmagic			/ilj 1/3/76
	bgt	1f			/ilj 1/3/76
2:
	mov	nambuf+2,r0	/ text
	mov	nambuf, magic		/chm 22/2/79 begins...
	mov	r0,txtsiz
	cmp	magic,$411	/separate?	(chm 3:9:79)
	beq	3f
	mov	r0,rtxtsiz
	add	$17777,rtxtsiz
	bic	$17777,rtxtsiz		/chm 22/2/79 .....ends
3:
	add	nambuf+4,r0	/ data
	tst	nambuf+16	/ relocation?
	bne	6f
	asl	r0		/ sym origin
6:
	add	$20,r0
	mov	r0,0f
	mov	r1,r0
	sys	seek; 0:..; 0
	mov	nambuf+10,r0	/ symbol size
	cmp	r0,$maxsym
	blos	3f
	mov	$maxsym,r0
3:
	add	r0,0f
	sys	break; 0: nambuf+12.
	mov	r0,0f
	mov	r1,r0
	sys	read; nambuf; 0:..
	add	$nambuf,r0
	mov	r0,namsiz
1:
	jsr	r5,get; zero	/ test new object
	cmp	r0,objmagic			/ilj 1/3/76
	blt	1f			/ilj 1/3/76
	cmp	r0,nobjmagic			/ilj 1/3/76
	bgt	1f			/ilj 1/3/76
2:
	mov	$20,getoff
1:
	mov	sp,savsp
	sys	signal; 2; 1
	ror	r0
	bcs	1f
	sys	signal; 2; errexit
1:
	cmp	r4,$2		/ arg count
	beq	9f		/ not core image
	cmp	r4,$4		/ no-core image flag
	beq	9f
	mov	$1024.,getoff
	mov	dbfin,r0
	sys	seek; 0; 0
	mov	dbfin,r0
	sys	read; regbuf; 1024.
	mov	txtsiz,r0
	ash	$6,r0
	mov	r0,txtsiz
	tst	separate	/ sep i/d	(chm 9:3:79)
	bne	1f
	add	$17777,r0
	bic	$17777,r0
	mov	r0,rtxtsiz
1:
	mov	datsiz,r0
	ash	$6,r0
	mov	r0,datsiz
	mov	stksiz,r0
	ash	$6,r0
	mov	r0,stksiz
	mov	*locfpsr,r0
	bic	$!200,r0
	mov	r0,fpsr
	br	loop
9:
loop:
	clr	error
	jsr	pc,readlin
	jsr	pc,readexp
	tst	error
	bne	errexit
	mov	$1,count
	cmpb	r0,$',
	bne	2f
	movb	(r4)+,r0
	mov	addres,-(sp)
	mov	adrflg,-(sp)
	mov	sytype,-(sp)	/ chm 9:3:79
	jsr	pc,readexp
	mov	addres,count
	mov	(sp)+,sytype	/ chm 9:3:79
	mov	(sp)+,adrflg
	mov	(sp)+,addres
	tst	error
	bne	errexit
2:
	movb	(r4),r0
	jsr	pc,command
	tst	error
	beq	loop
errexit:
	sys	signal; 2; errexit
	mov	savsp,sp
	jsr	r5,mesg; <?\n\0>; .even
	br	loop

fnfnd:
	jsr	r5,mesg; <File not found.\n\0>; .even
ex:
	sys	exit

readlin:
	mov	$inbuf,r4
1:
	mov	ttyfin,r0
	sys	read; ch; 1
	tst	r0
	beq	ex
	cmpb	ch,$'\n
	beq	1f
	movb	ch,(r4)+
	br	1b
1:
	clrb	(r4)
	mov	$inbuf,r4
	rts	pc

switch:
	mov	(r5)+,r1
2:
	cmp	r0,(r1)+
	bne	1f
	tst	(sp)+
	jmp	*(r1)
1:
	tst	(r1)+
	bne	2b
	rts	r5

readexp:
	mov	$'+,lastop
	clr	addres
	clr	starmod
	clr	taddr
	clr	adrflg
nextxp:
	movb	(r4)+,r0
	cmp	r0,$'0
	blt	1f
	cmp	r0,$'9
	ble	numin
	cmp	r0,$'a
	blt	1f
	cmp	r0,$'z
	bgt	1f
	jmp	letin
1:
	cmp	r0,$'a-40
	blt	1f
	cmp	r0,$'z-40
	jle	letin
1:
	jsr	r5,switch; expsw
	tstb	-(r4)
	tst	starmod
	beq	1f
	mov	dot,taddr
	br	operand
1:
	bic	$!7,sytype	/chm 9:3:79
	rts	pc

expsw:
	'+;	opex
	'-;	opex
	' ;	nextxp
	'';	onechar		/andy mod
	'";	twochar		/andy mod
	'.;	letin
	'_;	letin
	'^;	circumf
	'*;	star
	-1;	0

star:
	mov	pc,starmod
	br	nextxp

operand:
	inc	adrflg
	tst	starmod
	beq	1f
	clr	starmod
	bis	bytemod,error
	jsr	r5,get; taddr
	tst	error
	bne	1f
	mov	r0,taddr
	mov	$3,sytype	/change type to data	(chm 9:3:79)
1:
	cmp	lastop,$'+
	beq	1f
	sub	taddr,addres
	br	2f
1:
	add	taddr,addres
2:
	mov	$'+,lastop
	br	nextxp

circumf:
	mov	dot,taddr
0:					/chm 9:3:79
	dec	taddr
	tst	bytemod
	bne	1f
	dec	taddr
1:
	cmpb	(r4),$'^		/again?
	bne	operand
	inc	r4
	br	0b			/chm 9:3:79

numin:
	clr	r1
	clr	r3
1:
	sub	$'0,r0
	asl	r1
	asl	r1
	asl	r1
	mpy	$10.,r3
	bis	r0,r1
	add	r0,r3
	movb	(r4)+,r0
	cmp	r0,$'0
	blo	1f
	cmp	r0,$'9
	blos	1b
1:
	cmp	r0,$'.
	bne	1f
	mov	r3,r1
	movb	(r4)+,r0		/chm 9:3:79	starts..
1:
	cmp	r0,$'i		/i-space reference
	bne	1f
	mov	$2,sytype
	br	2f
1:
	cmp	r0,$'d		/d-space reference
	bne	1f
	mov	$3,sytype
2:
	inc	r4			/chm 9:3:79	....ends
1:
	mov	r1,taddr
	dec	r4
	br	operand

letin:
	dec	r4
	mov	$nambuf,namstrt
letin1:
	mov	$symbol,r1
	clr	(r1)+
	clr	(r1)+
	clr	(r1)+
	clr	(r1)
	mov	$symbol,r1
	mov	$8.,-(sp)
	br	2f
1:
	tstb	(r4)+
	cmpb	(r4),$'.
	beq	2f
	cmpb	(r4),$'0
	blo	3f
	cmpb	(r4),$'9
	blos	2f
	cmpb	(r4),$'A
	blo	3f
	cmpb	(r4),$'Z
	blos	2f
	cmpb	(r4),$'_
	beq	2f
	cmpb	(r4),$'a
	blo	3f
	cmpb	(r4),$'z
	bhi	3f
2:
	dec	(sp)
	blt	1b
	movb	(r4),(r1)+
	br	1b
3:
	tst	(sp)+
	jsr	pc,lookupn
	tst	error
	bne	1f
	cmpb	(r4),$';
	beq	2f
1:
	jmp	operand
2:
	tstb	(r4)+
	mov	r1,namstrt
	br	letin1

opex:
	mov	r0,lastop
	jmp	nextxp

onechar:			/ andy mods
	movb	(r4)+,taddr	/ andy mods
	bne	0f		/ andy mods
	dec	r4		/ andy mods
	movb	-(r4),r0	/ andy mods
	inc	bytemod
	rts	pc		/ andy mods
0:				/ andy mods
	jmp	operand		/ andy mods

twochar:			/ andy mods
	movb	(r4)+,taddr	/ andy mods
	beq	0f		/ andy mods
	movb	(r4)+,taddr+1	/ andy mods
	beq	1f		/ andy mods
	jmp	operand		/ andy mods
1:				/ andy mods
	dec	r4		/ andy mods
0:				/ andy mods
	dec	r4		/ andy mods
	movb	-(r4),r0	/ andy mods
	clr	taddr		/ andy mods
	rts	pc		/ andy mods

command:
	jsr	r5,switch; comsw
	inc	error
	rts	pc

comsw:
	'/;	slash
	'\\;	bslash
	'?;	quest
	'\0;	newln
	'=;	equal
	':;	colon
	'!;	excla
	'';	squote
	'";	dquote
	'$;	dolr
	'&;	amper
	'%;	ex
	'`;	grave
	-1;	0

dolr:
	mov	sigp,r1
	bic	$!17,r1
	asl	r1
	mov	traptab(r1),r1
	jsr	pc,string
	jsr	pc,pnl
	mov	$doltab,r5
1:
	mov	(r5)+,r1
	beq	1f
	jsr	pc,*(r5)+
	br	1b
	.if	fpp
1:
	mov	(r5)+,r1
	beq	1f
	ldfps	fpsr
	jsr	pc,*(r5)+
	br	1b
1:
	mov	(r5)+,r1
	jsr	pc,*(r5)+
	.endif
1:
	rts	pc

traptab:
	1f; .data; 1:<?0\0>; .text
	1f; .data; 1:<Hangup\0>; .text
	1f; .data; 1:<Interrupt\0>; .text
	1f; .data; 1:<Quit\0>; .text
	1f; .data; 1:<Illegal instruction\0>; .text
	1f; .data; 1:<Trace/BPT\0>; .text
	1f; .data; 1:<IOT\0>; .text
	1f; .data; 1:<EMT\0>; .text
	1f; .data; 1:<FP exception\0>; .text
	1f; .data; 1:<Killed\0>; .text
	1f; .data; 1:<Bus error\0>; .text
	1f; .data; 1:<Segmentation violation\0>; .text
	1f; .data; 1:<Bad system call\0>; .text
	1f; .data; 1:<Broken pipe\0>; .text
	1f; .data; 1:<Terminate\0>; .text	/ ilj sep 77
	1f; .data; 1:<Real Time Limit\0>; .text	/ ilj sep 77
	1f; .data; 1:<CPU Time Limit\0>; .text	/ ilj sep 77
	1f; .data; 1:<Memory parity error\0>; .text	/ chm feb 79
	1f; .data; 1:<?18\0>; .text	/ ilj sep 77
	1f; .data; 1:<?19\0>; .text	/ ilj sep 77
	1f; .data; 1:<?20\0>; .text	/ ilj sep 77

locps:	regbuf+1776
locpc:	regbuf+1774
locr0:	regbuf+1772
locr1:	regbuf+1766
locr2:	regbuf+1750
locr3:	regbuf+1752
locr4:	regbuf+1754
locsp:	regbuf+1764
locr5:	regbuf+1756
locfpsr: regbuf+4+.pdp70+.mappedb
locfr0:	regbuf+6+.pdp70+.mappedb
locfr4:	regbuf+16+.pdp70+.mappedb
locfr5:	regbuf+26+.pdp70+.mappedb
locfr1:	regbuf+36+.pdp70+.mappedb
locfr2:	regbuf+46+.pdp70+.mappedb
locfr3:	regbuf+56+.pdp70+.mappedb

doltab:
	1f; prgreg; locsp; .data; 1:<sp	\0>; .text
	1f; proct; locps; .data; 1:<ps	\0>; .text
	1f; prgreg; locpc; .data; 1:<pc	\0>; .text
	1f; prgreg; locr0; .data; 1:<r0	\0>; .text
	1f; prgreg; locr1; .data; 1:<r1	\0>; .text
	1f; prgreg; locr2; .data; 1:<r2	\0>; .text
	1f; prgreg; locr3; .data; 1:<r3	\0>; .text
	1f; prgreg; locr4; .data; 1:<r4	\0>; .text
	1f; prgreg; locr5; .data; 1:<r5	\0>; .text
	0
	.if	fpp
	1f; prfreg; locfr0; .data; 1:<fr0	\0>; .text
	1f; prfreg; locfr1; .data; 1:<fr1	\0>; .text
	1f; prfreg; locfr2; .data; 1:<fr2	\0>; .text
	1f; prfreg; locfr3; .data; 1:<fr3	\0>; .text
	1f; prfreg; locfr4; .data; 1:<fr4	\0>; .text
	1f; prfreg; locfr5; .data; 1:<fr5	\0>; .text
	0
	1f; proct; locfpsr; .data; 1:<fpsr	\0>; .text
	.endif
.data
.even
.text

	.if	fpp
prfreg:
	mov	*(r5)+,r4
	movf	(r4),fr0
	cfcc
	beq	1f
	jsr	pc,string
	jsr	pc,printf
	jsr	pc,pnl
1:
	rts	pc
	.endif

prgreg:
	jsr	pc,string
	mov	*(r5)+,r4
	mov	(r4),r0
	jsr	pc,printo
	mov	(r4),r0
	jsr	pc,lookupv
	tst	r2
	beq	1f
	jsr	r5,mesg; <	\0>; .even
	mov	(r4),r0
	jsr	pc,pname
1:
	jsr	pc,pnl
	rts	pc

proct:
	jsr	pc,string
	mov	*(r5)+,r4
	mov	(r4),r0
	jsr	pc,printo
	jsr	pc,pnl
	rts	pc

string:
	movb	(r1)+,r0
	beq	1f
	jsr	pc,putc
	br	string
1:
	rts	pc

putc:
	mov	r0,och
	mov	$1,r0
	sys	write; och; 1
	rts	pc

equal:
	jsr	r5,coleq; printo
	rts	pc
amper:
	clr	bytemod
	mov	$2,incdot
	jsr	r5,cycle; asymp
	rts	pc

asymp:
	jsr	pc,pname
	jsr	pc,pnl
	rts	pc

bslash:
	inc	bytemod
	mov	$1,incdot
	br	1f

slash:
	clr	bytemod
	mov	$2,incdot
1:
	jsr	r5,cycle; octp
	rts	pc

grave:
	clr	bytemod
	mov	$2,incdot
	jsr	r5,cycle;  decp
	rts	pc

quest:
	clr	bytemod
	jsr	r5,cycle; psym
	rts	pc

decp:
	jsr	pc,printd
	jsr	r5,mesg; <.\n\0>; .even
	rts	pc

octp:
	jsr	pc,printo
	jsr	pc,pnl
	rts	pc

newln:
	tst	adrflg
	bne	1f
	add	incdot,dot
1:
	mov	nlcom,r0
	jmp	command

excla:
	tst	adrflg
	bne	1f
2:
	inc	error
	rts	pc
1:
	bit	$1,dot
	beq	1f
	tst	bytemod
	beq	2b
1:
	jsr	r5,put; dot; addres
	rts	pc


squote:
	inc	bytemod
	mov	$1,incdot
	br	2f
dquote:
	clr	bytemod
	mov	$2,incdot
2:
	jsr	r5,cycle; ascp
	rts	pc

ascp:
	mov	r0,-(sp)
	jsr	pc,ascp1
	mov	(sp)+,r0
	tst	bytemod
	bne	1f
	swab	r0
	jsr	pc,ascp1
1:
	cmp	count,$1
	bgt	1f
	jsr	pc,pnl
1:
	rts	pc

ascp1:
	bic	$!377,r0
	cmp	r0,$'\n
	beq	2f
	cmp	r0,$011
	beq	2f
	cmp	r0,$40
	blo	1f
	cmp	r0,$177
	bhis	1f
2:
	jsr	pc,putc
	rts	pc
1:
	mov	r0,r1
	jsr	r5,mesg; <\\\0>
	clr	r0
	alsc	$10.,r0
	add	$'0,r0
	jsr	pc,putc
	clr	r0
	alsc	$3,r0
	add	$'0,r0
	jsr	pc,putc
	clr	r0
	alsc	$3,r0
	add	$'0,r0
	jsr	pc,putc
	rts	pc

colon:
	jsr	r5,coleq; pname
	rts	pc

coleq:
	jsr	pc,setadr
	mov	addres,r0
	jsr	pc,*(r5)+
	jsr	pc,pnl
	rts	r5

cycle:
	mov	r0,nlcom
	jsr	pc,setadr
	mov	addres,dot
	tst	bytemod
	bne	1f
	bic	$1,dot
1:
	jsr	r5,get; dot
	tst	error
	bne	1f
	tst	bytemod
	beq	2f
	bic	$!377,r0
2:
/
/ addition by craig macgregor
/
/  purpose:	print out a label before data if available
/
	mov	r0,-(sp)	/
	mov	r1,-(sp)	/
	mov	r2,-(sp)	/
	mov	r3,-(sp)	/
	mov	r4,-(sp)	/
	mov	r5,-(sp)	/
	mov	dot,r0		/address
	jsr	pc,lookupv	/get label (if any)
	tst	r2		/any symbol?
	beq	5f		/no
	tst	r3		/must be exact address.
	bne	5f		/is not
	mov	$8.,r3		/char count
3:	movb	(r2)+,r0	/char
	beq	4f		/end of string
	jsr	pc,putc	/print it!
	dec	r3		/how goes count?
	bne	3b		/ok
4:	jsr	r5,mesg; <: \0\0>
5:	mov	(sp)+,r5	/
	mov	(sp)+,r4	/
	mov	(sp)+,r3	/
	mov	(sp)+,r2	/
	mov	(sp)+,r1	/
	mov	(sp)+,r0	/
/
/ end of addition
/
	jsr	pc,*(r5)
	tst	error
	bne	1f
	dec	count
	ble	1f
	add	incdot,dot
	br	1b
1:
	tst	(r5)+
	rts	r5

setadr:
	tst	adrflg
	bne	1f
	mov	dot,addres
1:
	rts	pc

	.if	fpp
printf:
	ldfps	$200		/ round+double
	mov	r4,-(sp)
	mov	r3,-(sp)
	movif	$10.,r3
	movif	$1,r2
	clr	r4
	tstf	r0
	cfcc
	beq	2f
	bge	1f
	negf	r0
	mov	$'-,r0
	jsr	pc,putc
1:
	cmpf	r3,r0
	cfcc
	bgt	1f
	inc	r4
	divf	r3,r0
	br	1b
1:
	cmpf	r2,r0
	cfcc
	ble	2f
	dec	r4
	mulf	r3,r0
	br	1b
2:
	modf	r2,r0
	movfi	r1,r0
	add	$'0,r0
	jsr	pc,putc
	mov	$'.,r0
	jsr	pc,putc
	mov	$8.,r3
1:
	modf	r3,r0
	movfi	r1,r0
	add	$'0,r0
	jsr	pc,putc
	dec	r3
	bgt	1b
	mov	$'E,r0
	jsr	pc,putc
	mov	r4,r0
	mov	(sp)+,r3
	mov	(sp)+,r4
	br	printd
	.endif

printd:
	mov	r1,-(sp)
	mov	r0,r1
	bpl	1f
	neg	r1
	mov	$'-,r0
	jsr	pc,putc
1:
	jsr	pc,1f
	mov	(sp)+,r1
	rts	pc
1:
	clr	r0
	dvd	$10.,r0
	mov	r1,-(sp)
	mov	r0,r1
	beq	1f
	jsr	pc,1b
1:
	mov	(sp)+,r0
	add	$'0,r0
	jsr	pc,putc
	rts	pc

