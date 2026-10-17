/ uid	= getuid();

.globl	_getuid

_getuid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	getuid
	mov	(sp)+,r5
	rts	pc


/ uid	= geteuid();

.globl	_geteuid

_geteuid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	getuid
	mov	r1,r0
	mov	(sp)+,r5
	rts	pc
