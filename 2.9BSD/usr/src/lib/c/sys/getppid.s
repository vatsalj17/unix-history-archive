/ pid	= getppid();

.globl _getppid

_getppid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	getpid
	mov	r1,r0
	mov	(sp)+,r5
	rts	pc
