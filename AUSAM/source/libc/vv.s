/ C library -- vv

/ vv(&lock);
/
/ release lock

.globl	_vv

vvv = 54.

_vv:	mov     2(sp),r0         / free resource.
	sys	vvv
	rts     pc
