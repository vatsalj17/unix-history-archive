/ tape boot program to load and transfer
/ to a 'tp' entry

/ entry is made by jsr pc,*$0
/ so return can be rts pc
/ jsr pc,(r5) is putc
/ jsr pc,2(r5) is getc
/ jsr pc,4(r5) is mesg

core = 24.
.. = [core*2048.]-512.
start:
	mov	$..,sp
	mov	sp,r1
	cmp	pc,r1
	bhis	2f
	clr	r0
	cmp	(r0),$407
	bne	1f
	mov	$20,r0
1:
	mov	(r0)+,(r1)+
	cmp	r1,$core*2048.
	blo	1b
	jmp	(sp)

2:
	mov	$trvect,r5
	mov	$name,r4
	jsr	pc,rew
	mov	$'=,r0
	jsr	pc,(r5)
2:
	mov	r4,r1
1:
	jsr	pc,getc
	cmp	r0,$'\n
	beq	1f
	cmp	r0,$'@
	beq	2b
	movb	r0,(r1)+
	cmp	r0,$'#
	bne	1b
	sub	$2,r1
	cmp	r1,r4
	blo	2b
	br	1b
1:
	clrb	(r1)
	cmp	r1,r4
	blos	start
	mov	$1,tapa
	mov	$-6144.,wc
	jsr	pc,tread
	clr	r1
1:
	mov	r1,r2
	mov	r4,r0
2:
	cmpb	(r0)+,(r1)
	bne	2f
	tstb	(r1)+
	bne	2b
	br	1f
2:
	mov	r2,r1
	add	$64.,r1
	cmp	r1,$12288.
	blo	1b
	jsr	pc,rew
	br	start
1:
	mov	44.(r2),tapa
	mov	38.(r2),r0
	inc	r0
	clc
	ror	r0
	neg	r0
	mov	r0,wc
	clr	r0
1:
	clr	(r0)+
	cmp	r0,sp
	blo	1b
	jsr	pc,tread
	jsr	pc,rew
	br	callout

tapa:	0
wc:	0
ba:	0
name	= ..-32.
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
	jsr	pc,(r5)
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
	jsr	pc,(r5)
	inc	(sp)
	br	mesg
1:
	add	$2,(sp)
	bic	$1,(sp)
	rts	pc

callout:
	clr	r0
	cmp	(r0),$407
	bne	2f
1:
	mov	20(r0),(r0)+
	cmp	r0,sp
	blo	1b
2:
	mov	$start,-(sp)
	clr	pc

trvect:
	br	putc
	br	getc
	br	mesg
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
	jsr	pc,rrrec
	br	1b
2:
	jsr	pc,rew
	br	1b
1:
	mov	wc,r1
1:
	jsr	pc,rrrec
	add	$256.,r1
	bmi	1b
	rts	pc

rrrec:
	tst	tapa
	beq	rrec	/ block 0 is really block 0
	inc	tapa	/ block 1->n is really block 2->n+1
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
