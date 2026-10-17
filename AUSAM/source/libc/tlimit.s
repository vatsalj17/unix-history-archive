/ C library -- tlimit

/ tlimit(limit);

tlimit = 39.
.globl	_tlimit

_tlimit:
	mov	2(sp),r0
	sys	tlimit
	rts	pc
