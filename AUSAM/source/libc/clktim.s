clktim = 40.
.globl	_clktim

_clktim:
	mov	2(sp),r0
	sys	clktim
	rts	pc
