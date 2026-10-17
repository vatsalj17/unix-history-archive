/ C library -- newstat
/ interchanges AUSAM type uid and nlinks so
/ that uid is on a word boundary

/ error = newstat(string, statbuf);

/
/ struct statbuf statbuf;
/

nlinks = 6.
ouid = 7.
ogid = 8.


.globl	_newstat, cerror

_newstat:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),r1
	mov	r1,0f+2
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	movb	nlinks(r1),r0
	movb	ouid(r1),nlinks(r1)
	movb	ogid(r1),ouid(r1)
	movb	r0,ogid(r1)
	clr	r0
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	stat; 0:..; ..
