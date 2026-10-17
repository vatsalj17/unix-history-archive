/ C library -- tell
/
/ long tell();
/
/ l = tell(fd)
/
/ Makes use of fix034.

	.globl	_tell, cerror

tell = 27.

_tell:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	tell		/ current position
	bec	0f
	mov	$-1,r1		/ force ((long) -1)
	jmp	cerror
0:
	mov	(sp)+,r5
	rts	pc
