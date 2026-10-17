/ makdir -- make a directory
makdir = 14.
access = 33.

W = 2

.AUSAM = 1

	sys	getuid
.if	.AUSAM-1
	mov	r0,uid
.endif
.if	.AUSAM
	mov	r1,uid
.endif
	mov	sp,r5
	tst	(r5)+
/ set signals to prevent any foul ups
	sys	signal;	2;	1
	sys	signal;	3;	1

loop:
	tst	(r5)+
	dec	(sp)
	bgt	1f
	mov	err,r0
	sys	exit
1:
	mov	(r5),r0
	mov	$buf1,r1
	mov	$buf2,r2
	clr	r3
1:
	movb	(r0)+,r4
	beq	2f
	movb	r4,(r1)+
	movb	r4,(r2)+
	cmpb	r4,$'/
	bne	1b
	mov	r2,r3
	br	1b
2:
	movb	$'/,(r1)+
	movb	$'.,(r1)+
	clrb	(r1)
	mov	$dot,dir
	tst	r3
	beq	1f
	mov	$buf2,dir
	clrb	(r3)
	cmp	r3,$buf2+1
	beq	1f
	clrb	-(r3)		/ ???
1:
	tst	uid
	beq	2f
/
/	access system call makes this code redundant
/
/	sys	stat; dir:..; stbuf	/ status of parent dir
/	bes	error
/	mov	stbuf+4,r0
/	cmpb	uid,stbuf+7
/	bne	1f
/.if	.AUSAM
/	cmpb	uid+1,stbuf+8.
/	bne	1f
/.endif
/	ash	$-6,r0
/1:
/	ror	r0
/	ror	r0
/	bcc	error			/ no write permission in parent
	sys	access; dir: ..; W	/ check for write permission in parent
	bes	error
2:
	mov	(r5),0f
	sys	makdir; 0:..; 140700; 0
	bes	error			/ prob already exists
	mov	(r5),0f
	sys	chown; 0:..; uid:..
	mov	(r5),0f
	sys	link; 0:..; buf1
	bes	error
	movb	$'.,(r1)+
	clrb	(r1)
	mov	dir,0f
	sys	link; 0:..; buf1
	bec	loop

error:
	mov	(r5),r0
	mov	r0,0f
	clr	0f+2
1:
	tstb	(r0)+
	beq	1f
	inc	0f+2
	br	1b
1:
	mov	$1,r0
	sys	write; 0:..; ..
	mov	$1,r0
	sys	write; ques; 3
	inc	err
	br	loop

dot:	<.\0>
ques:	< ?\n>
	.even

.bss
buf1:	.=.+100.
buf2:	.=.+100.
stbuf:	.=.+40.
err:	.=.+2
