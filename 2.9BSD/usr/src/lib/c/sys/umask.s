/ omask	= umask(complmode);

.globl	_umask
.globl	cerror

_umask:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	umask; 0:..
