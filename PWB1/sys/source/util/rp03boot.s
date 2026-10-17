/ disk boot program to load and transfer
/ to a unix file system entry

/ entry is made by jsr pc,*$0
/ so return can be rts pc
/ jsr pc,(r5) is putc
/ jsr pc,2(r5) is getc
/ jsr pc,4(r5) is mesg

core = 28.
.rp. = 1
.hp. = 0
.. = [core*2048.]-512.
reset	= 5

.if .rp.
read	= 5
daddr	= 176724
sct	= 10.
trk	= 20.
.endif
.if .hp.
read	= 71
daddr	= 176710
sct	= 22.
trk	= 19.
.endif

start:
	mov	$..,sp
	mov	sp,r1
	mov	r1,-(sp)
	cmp	pc,sp
	bhis	main
move:
	clr	r0
	mov	r1,-(sp)
	cmp	(r0),$407
	bne	1f
	mov	$20,r0
1:
	cmp	r0,r1
	beq	2f
1:
	mov	(r0)+,(r1)+
	cmp	r1,buf
	blo	1b
2:
	rts	pc
main:
	reset
	clr	r0
1:
	clr	(r0)+
	cmp	r0,sp
	blo	1b
	mov	$tvec,r5
	mov	$'#,r0
	jsr	pc,(r5)
	jsr	pc,2(r5)
	bic	$!7,r0
.if .rp.	/ set read command to select unit
	movb	r0,drive+3
.endif
.if .hp.	/ select unit, read-in preset and set FMT22
	mov	r0,*$daddr
	mov	$21,*$daddr-10
	mov	$10000,*$daddr+22
.endif
	jsr	pc,2(r5)
	bic	$!7,r0
	asl	r0
	mov	cyl(r0),cyloff
	mov	$'=,r0
	jsr	pc,(r5)
	mov	$names,r2
1:
	mov	r2,r1
2:
	jsr	pc,2(r5)
	cmp	r0,$'\n
	beq	1f
	cmp	r0,$'@
	beq	1b
	cmp	r0,$'#
	beq	3f
	cmp	r0,$'/
	bne	4f
	clr	r0
4:
	movb	r0,(r1)+
	br	2b
3:
	cmp	r2,r1
	beq	2b
	dec	r1
	br	2b
1:
	clrb	(r1)+
	movb	$-1,(r1)
	mov	$1,r0
1:
	jsr	pc,iget
	clr	buf
2:
	tstb	(r2)+
	beq	2b
	bmi	1f
	dec	r2
2:
	jsr	pc,rmblk
		br	start
	clr	r1
3:
	mov	r2,r3
	mov	r1,r4
	add	$16.,r1
	tst	(r4)+
	beq	5f
4:
	cmpb	(r3)+,(r4)
	bne	5f
	tstb	(r4)+
	beq	4f
	cmp	r4,r1
	blo	4b
6:
	tstb	(r3)+
	bne	6b
4:
	mov	-16.(r1),r0
	mov	r3,r2
	br	1b
5:
	cmp	r1,$512.
	blo	3b
	br	2b
1:
	cmp	$117777,mode
	blo	start
1:
	jsr	pc,rmblk
		br	1f
	add	$512.,buf
	br	1b
1:
	clr	r1
	br	move
iget:
	clr	bno
	add	$31.,r0
	mov	r0,r1
	ash	$-4.,r0
	mov	$inod,buf
	bic	$!17,r1
	ash	$5,r1
	sub	r1,buf
	jsr	pc,rblk
	bit	$LRG,mode
	beq	2f
	mov	$addr,buf
	mov	addr,r0
	br	rblk
rmblk:
	mov	bno,r0
	asl	r0
	mov	addr(r0),r0
	beq	2f
	add	$2,(sp)
	inc	bno
rblk:
	mov	r0,-(sp)
	mov	r0,r1
	clr	r0
	div	$sct,r0
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	div	$trk,r0
	bisb	r1,1(sp)
	mov	$daddr,r1
.if .rp.
	mov	(sp)+,(r1)
.endif
	add	cyloff,r0
.if .rp.
	mov	r0,-(r1)
.endif
.if .hp.
	mov	r0,24(r1)
	mov	(sp)+,-(r1)
.endif
	mov	buf,-(r1)
	mov	$-256.,-(r1)
drive:
	mov	$read,-(r1)
	mov	(sp)+,r0
1:
	tstb	(r1)
	bge	1b
2:
	rts	pc

tvec:
	br	putc
	br	getc
	br	mesg

tks = 177560
tkb = 177562
getc:
	mov	$tks,r0
	inc	(r0)
1:
	tstb	(r0)
	bge	1b
	mov	*$tkb,r0
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
	tstb	*$tps
	bge	putc
	cmp	r0,$'\n
	bne	1f
	jsr	pc,4(r5)
	.byte	'\r, '\n+200, 0
	.even
	mov	$'\n,r0
	rts	pc
1:
	mov	r0,*$tpb
	rts	pc
mesg:
	movb	*(sp),r0
	inc	(sp)
	jsr	pc,(r5)
	bne	mesg
	inc	(sp)
	bic	$1,(sp)
	rts	pc
cyl:
.if .rp.
	0; 313; 0; 13; 26; 313; 600; 0
.endif
.if .hp.
	0; 27.; 155.; 283.; 27.; 184.; 341.; 0
.endif
buf:	end
end:
inod	= ..-1024.
mode	= inod
addr	= inod+8.
bno	= inod+32.+512.
cyloff	= bno+2.
names	= cyloff+2.
LRG	= 10000
