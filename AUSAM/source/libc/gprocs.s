gprocs = 61.
.globl	_gprocs
_gprocs:
	mov	2(sp),r0
	sys	gprocs
	rts	pc
