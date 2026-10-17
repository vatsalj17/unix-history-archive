/ disk boot program to load and transfer
/ to a unix entry

/ entry is made by jsr pc,*$0
/ so return can be rts pc

csw = 177570
core = 28.
.. = [core*2048.]-512.
start:
	mov	$..,sp
	mov	sp,r1
	cmp	pc,r1
	bhis	2f
	reset
	clr	r0
	cmp	(r0),$407
	bne	1f
	mov	$20,r0
1:
	mov	(r0)+,(r1)+
	cmp	r1,$end
	blo	1b
	jmp	(sp)

2:
	mov	$inod,r0
1:
	clr	(r0)+
	cmp	r0,sp
	blo	1b
	mov	$names,r1
	tst	*$csw
	beq	1f
	mov	$"un,(r1)+
	mov	$"ix,(r1)+
	clrb	(r1)+
	br	9f
1:
	mov	$'?,r0
	jsr	pc,putc
1:
	mov	r1,r2
2:
	jsr	pc,getc
	cmp	r0,$'\n
	beq	1f
	cmp	r0,$'/
	beq	3f
	movb	r0,(r2)+
	br	2b
3:
	cmp	r1,r2
	beq	2b
	add	$14.,r1
	br	1b
1:
9:
	mov	$names,r1
	mov	$1,r0
1:
	clr	bno
	clr	ibno
	clr	iflg
	jsr	pc,iget
	tst	(r1)
	beq	1f
2:
	jsr	pc,rmblk
		br start
	mov	$buf,r2
3:
	mov	r1,r3
	mov	r2,r4
	add	$16.,r2
	tst	(r4)+
	beq	5f
4:
	cmpb	(r3)+,(r4)+
	bne	5f
	cmp	r4,r2
	blo	4b
	mov	-16.(r2),r0
	add	$14.,r1
	br	1b
5:
	cmp	r2,$buf+512.
	blo	3b
	br	2b
1:
	clr	r1
1:
	jsr	pc,rmblk
		br 1f
	mov	$buf,r2
2:
	mov	(r2)+,(r1)+
	cmp	r2,$buf+512.
	blo	2b
	br	1b
1:
	clr	r0
	cmp	(r0),$407
	bne	2f
1:
	mov	20(r0),(r0)+
	cmp	r0,sp
	blo	1b
2:
	jsr	pc,*$0
	br	start

iget:
	add	$31.,r0
	mov	r0,r5
	ash	$-4.,r0
	jsr	pc,rblk
	bic	$!17,r5
	ash	$5.,r5
	add	$buf,r5
	mov	$inod,r4
1:
	mov	(r5)+,(r4)+
	cmp	r4,$addr+16.
	blo	1b
	rts	pc

rmblk:
	add	$2,(sp)
	mov	bno,r0
	inc	bno
	bit	$LRG,mode
	bne	1f
	asl	r0
	mov	addr(r0),r0
	bne	rblk
2:
	sub	$2,(sp)
	rts	pc
1:
	clr	-(sp)
	movb	r0,(sp)
	clrb	r0
	swab	r0
	asl	r0
	mov	addr(r0),r0
	beq	2b
	cmp	r0,ibno
	beq	9f
	inc	iflg
	mov	r0,ibno
	jsr	pc,rblk
9:	mov	(sp)+,r0
	asl	r0
	mov	ibuf(r0),r0
	beq	2b

rblk:
	mov	r1,-(sp)
	tst	r0
	beq	1f
	add	$2435.,r0
	cmp	r0,$4872.
	blt	1f
	sub	$4871.,r0
1:
	mov	r0,r1
	clr	r0

rkda = 177412
rkblk:
	div	$12.,r0
	ash	$4.,r0
	bis	r1,r0
	mov	$rkda+2,r1

1:
	mov	r0,-(r1)
	mov	$buf,-(r1)
	tst	iflg
	beq	9f
	add	$ibuf-buf,(r1)
	clr	iflg
9:
	mov	$-256.,-(r1)
	mov	$5,-(r1)
1:
	tstb	(r1)
	bge	1b
	mov	(sp)+,r1
	rts	pc

tks = 177560
tkb = 177562
getc:
	mov	$tks,r0
	inc	(r0)
1:
	tstb	(r0)
	bge	1b
	mov	tkb,r0
	bic	$!177,r0
	cmp	r0,$'A
	blo	1f
	cmp	r0,$'Z
	bhi	1f
	add	$40,r0
1:
	cmp	r0,$'\r
	bne	putc
	mov	$'\n,r0

tps = 177564
tpb = 177566
putc:
	tstb	tps
	bge	putc
	cmp	r0,$'\n
	bne	1f
	mov	$'\r,r0
	jsr	pc,putc
	mov	$'\n+200,r0
	jsr	pc,putc
	clr	r0
	jsr	pc,putc
	mov	$'\n,r0
	rts	pc
1:
	mov	r0,tpb
	rts	pc

end:
inod = ..-1536.
mode = inod
addr = inod+8.
buf = inod+32.
bno = buf+514.
ibuf = bno+2
ibno = ibuf+514.
iflg = ibno+2
names = iflg+2
LRG = 10000
reset = 5
