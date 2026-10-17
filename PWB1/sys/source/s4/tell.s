/ C library -- tell
/
/ long tell(f) -- returns offset of file f.

tell = 40.

.globl	_tell
.globl	_errno

_tell:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	tell
	bec	1f
	mov	r0,_errno
	mov	$-1,r0
	mov	r0,r1
1:
	mov	(sp)+,r5
	rts	pc
