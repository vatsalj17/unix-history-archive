/ C library -- newfstat
/ it interchanges AUSAM type uids & nlinks
/ so that uid is on a word boundary

/ error = newfstat(file, statbuf);

/
/ struct statbuf statbuf;
/

onlinks = 6.
ouid = 7.
ogid = 8.

.globl	_newfstat, cerror

_newfstat:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	6(r5),r1
	mov	r1,0f
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	movb	onlinks(r1),r0
	movb	ouid(r1),onlinks(r1)
	movb	ogid(r1),ouid(r1)
	movb	r0,ogid(r1)
	clr	r0
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	fstat; 0:..
