/ boots a file from TU16 tape unit
/


core = 28.	/ first core loc (in KW) not used
reset = 5
.. = [core*2048.]-512.
start:

/ copy self to 'core' - 512. bytes
/ strip off UNIX execute header if present

	mov	$..,sp
	mov	sp,r1
	clr	r0
	cmp	pc,r1
	bhis	2f
	reset
	cmp	(r0),$407
	bne	1f
	mov	$20,r0
1:
	mov	(r0)+,(r1)+
	cmp	r1,$core*2048.
	blo	1b
	jmp	(sp)

/ clear all of core

2:
	clr	(r0)+
	cmp	r0,sp
	blo	2b

/ now read tape into mem till tapemark
	clr	mtma
	clr	*$htcs2
	mov	$P800,*$httc
/	mov	$REW,*$htcs1
/	jsr	pc,rrec
/	clr	mtma
/	jsr	pc,rrec
/	clr	mtma
0:
	jsr	pc,rrec
	br	0b
tapemk:
	jsr	pc,*$0
	jbr	start

htcs1 = 172440
htba  = 172444
htfc  = 172446
htcs2 = 172450
htds  = 172452
httc  = 172472

P800 = 1300
P1600 = 2300
PIP = 20000
TM  =     2
MOL = 10000
ERR = 40000
REV = 33
READ = 71
REW = 7
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
	bit	$TM,*$htds
	jeq	tapemk
	bit	$ERR,*$htds
	bpl	1f
	mov	$-1,*$htfc
	mov	$REV,(r0)
	br	rrec
1:
	add	$512.,mtma
	rts	pc


mtma:	0
