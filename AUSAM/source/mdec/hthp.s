/ copy mag tape to disk
/ load with proper tape and disk drivers

begin:
	5			/////  reset
	mov	$100000,sp
	jsr	pc,rew

	jsr	pc,mesg
		<\n***\n\0>
		.even
2:
	jsr	pc,mesg
		<filesys cyl= \0>
		.even
	jsr	pc,numb
	mov 	r0,dskc
	jsr	pc,mesg
		<block no= \0>
		.even
	jsr	pc,numb
	mov	r0,dska
	jsr	pc,mesg
		<tape offset= \0>
		.even
	jsr	pc,numb
	mov	r0,tapa
	jsr	pc,mesg
		<count= \0>
		.even
	jsr	pc,numb
	mov	r0,r2
1:
	jsr	pc,tread
	jsr	pc,wblk
	inc	tapa
	inc	dska
	dec	r2
	bne	1b
	jsr	pc,rew
	jbr	begin

numb:
	clr	r1
1:
	jsr	pc,getc
	cmp	r0,$'\n
	beq	1f
	sub	$'0,r0
	cmp	r0,$9
	bhi	2f
	mul	$10.,r1
	add	r0,r1
	br	1b
1:
	mov	r1,r0
	rts	pc
2:
	jsr	pc,mesg
		<??\n\0>
		.even
	jbr	begin

ba:	buf
wc:	-256.
.bss
buf:	.=.+512.
dskc:	.=.+2		/////  filesystem start address
dska:	.=.+2		/////  block address in the filesystem
tapa:	.=.+2
.text
////////////////////////////////////////////////// ht.s ////////////
htcs1 = 172440
htba  = 172444
htfc  = 172446
htcs2 = 172450
htds  = 172452
httc  = 172472

P800 = 1300
P1600 = 2300
PIP = 20000
MOL = 10000
ERR = 40000
REV = 33
READ = 71
REW = 7

tread:
1:
	mov	ba,mtma
	cmp	mtapa,tapa
	beq	1f
	bhi	2f
	jsr	pc,rrec
	br	1b
2:
	jsr	pc,rew
	br	1b
1:
	mov	wc,r1
1:
	jsr	pc,rrec
	add	$256.,r1
	bmi	1b
	rts	pc

rrec:
	mov	$htds,r0
	tstb	(r0)
	bpl	rrec
	bit	$PIP,(r0)
	bne	rrec
	bit	$MOL,(r0)
	beq	rrec
	mov	$htfc,r0
	mov	$-512.,(r0)
	mov	mtma,-(r0)
	mov	$-256.,-(r0)
	mov	$READ,-(r0)
1:
	tstb	(r0)
	bpl	1b
	bit	$ERR,*$htds
	bpl	1f
	mov	$-1,*$htfc
	mov	$REV,(r0)
	br	rrec
1:
	add	$512.,mtma
	inc	mtapa
	rts	pc

rew:
	clr	*$htcs2
	mov	$P800,*$httc
	mov	$REW,*$htcs1
	clr	mtapa
	rts	pc

mtapa:	0
mtma:	0
///////////////////////////////////////////////// hp.s /////////////
/ rp04 disk driver

hpcs1 = 176700
hpda  = 176706
hpcs2 = 176710
hpof  = 176732
hpca  = 176734

fmt22 = 010000
preset = 021
clear = 040

wblk:

first = .+2	/ dirty, but i need the space
	tst	$0
	bne	1f
	mov	$clear,*$hpcs2
	mov	$preset,*$hpcs1
	mov	$fmt22,*$hpof
	inc	first
1:
	mov	dska,r1
	clr	r0
	div	$22.,r0
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	div	$19.,r0
	bisb	r1,1(sp)
	add	dskc,r0		////  relocate this cylinder address, for desired filesystem
	mov	r0,*$hpca
	mov	$hpda,r1
	mov	(sp)+,(r1)
	mov	ba,-(r1)
	mov	wc,-(r1)
	mov	$61,-(r1)		////  write
1:
	tstb	(r1)
	bpl	1b
	rts	pc
///////////////////////////////////////////////// tty.s ////////////
/ read and echo character from tty.
/ perform normal cr/lf uc/lc mapping.
tks = 177560
tkb = 177562
getc:
	tstb	*$tks
	bge	getc
	mov	tkb,r0
	bic	$!177,r0
	cmp	r0,$'A
	blo	1f
	cmp	r0,$'Z
	bhi	1f
	add	$'a-'A,r0
1:
	cmp	r0,$'\r
	bne	putc
	mov	$'\n,r0

/ put a character on the tty.
/ also performs delay.
tps = 177564
tpb = 177566
putc:
	cmp	r0,$'\n
	bne	1f
	mov	$'\r,r0
	jsr	pc,putc
	mov	$'\n,r0
1:
	tstb	tps
	bpl	1b
	mov	r0,tpb
	rts	pc

/ write a string to tty
/ jsr pc, mesg; <string\0>; .even
mesg:
	movb	*(sp),r0
	beq	1f
	jsr	pc,putc
	inc	(sp)
	br	mesg
1:
	add	$2,(sp)
	bic	$1,(sp)
	rts	pc
