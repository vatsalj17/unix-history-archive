/ microcomputer systems controller bootstrap
/ to a unix file system entry
/
/ Copyright 1978  C. Maltby.  UNSW.

/ entry is made by jsr pc,*$0
/ so return can be rts pc

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

/ if flag is not clear then previous boot failed

	inc	$-1
	bne	3f

/ set up default file name

	mov	$names+4,r2
	mov	$"ix,-(r2)
	mov	$"un,-(r2)		/ default: boot /unix
	mov	*$SWR,r0
1:
	cmp	*$SWR,r0
	beq	1b
	bhi	8f

/ prompt
/ read in path name
/ breaking on '/' into 14 ch names

3:
	mov	$'?,r0
	jsr	pc,putc
1:
	mov	r2,r1
2:
	jsr	pc,getc
	cmp	r0,$'\n
	beq	1f
	cmp	r0,$'/
	beq	3f
	movb	r0,(r1)+
	br	2b
3:
	cmp	r2,r1
	beq	2b
	add	$14.,r2
	br	1b
1:
	mov	*$SWR,dskc		/ boot from cyl addr in switch reg.
	jsr	pc,getc
8:

/ start of path name decoding
/ start with first name  and root ino

	mov	$names,r2
	mov	$1,r0

/ get next inode

1:
	clr	*$bno
	jsr	pc,iget
	tst	(r2)
	beq	1f

/ read next directory looking for next name

2:
	jsr	pc,rmblk
		br start
	mov	$buf,r1
3:
	mov	r2,r3
	mov	r1,r4
	add	$16.,r1
	tst	(r4)+
	beq	5f
4:
	cmpb	(r3)+,(r4)+
	bne	5f
	cmp	r4,r1
	blo	4b
	mov	-16.(r1),r0
	add	$14.,r2
	br	1b
5:
	cmp	r1,$buf+512.
	blo	3b
	br	2b

/ last entry was found
/ read into 0.

1:
	clr	r2
1:
	jsr	pc,rmblk
		br callout
	mov	$buf,r1
2:
	mov	(r1)+,(r2)+
	cmp	r1,$buf+512.
	blo	2b
	br	1b

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


/ subroutine will read in inode
/ number specified in r0
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

/ routine to read in block
/ number specified by bno
/ after applying file system
/ mapping algorithm in inode.
/ bno is incremented, success
/ return is a skip, error (eof)
/ is direct return.
rmblk:
	add	$2,(sp)
	mov	*$bno,r0
	inc	*$bno
	bit	$LRG,mode
	bne	1f
	asl	r0
	mov	addr(r0),r0
	bne	rblk
2:
	sub	$2,(sp)
	rts	pc
/ large algorithm
/ huge algorithm is not implemented
1:
	clr	-(sp)
	movb	r0,(sp)
	clrb	r0
	swab	r0
	asl	r0
	mov	addr(r0),r0
	beq	2b
	cmp	r0,*$ibno
	beq	9f
	inc	*$iflg
	mov	r0,*$ibno
	jsr	pc,rblk
9:
	mov	(sp)+,r0
	asl	r0
	mov	ibuf(r0),r0
	beq	2b


rblk:
MSEND = 176712
	mov	$ba,r3
	mov	$20,*$rcom
	mov	$buf,(r3)
	tst	*$iflg
	beq	1f
	mov	$ibuf,(r3)
	clr	*$iflg
1:
	mov	$256.,-(r3)		/ positive word count for this device.
	mov	r0,r1
	clr	r0
	div	$21.,r0
	mov	r1,-(r3)		/ sector
	mov	r0,r1
	clr	r0
	div	$19.,r0
	mov	r1,-(r3)		/ head
	add	dskc,r0			/ correct cylinder
	mov	r0,-(r3)		/ cylinder
	mov	$MSEND,r1
	mov	$cblk,-(r1)
	clr	-(r1)
	bis	$400,-(r1)
1:
	tst	(r1)
	bpl	1b
	bis	$1000,(r1)
	rts	pc

/ read and echo character from tty.
/ perform normal cr/lf mapping.
tks = 177560
tkb = 177562
getc:
	tstb	*$tks
	bge	getc
	mov	tkb,r0
	bic	$!177,r0
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

dskc:	214.				/ default boot from cyl 230.

.bss
end:
inod = ..-1536.
mode = inod
addr = inod+8.
buf = inod+32.
bno = buf+514.
ibuf = bno+2
ibno = ibuf+514.
iflg = ibno+2
cblk = iflg+2				/ disk command block
rcom = cblk+8.
cyl = cblk+12.
head = cblk+14.
sect = cblk+16.
wc = cblk+18.
ba = cblk+20.
bae = cblk+22.
names = cblk+24.
LRG = 10000
SWR = 177570
