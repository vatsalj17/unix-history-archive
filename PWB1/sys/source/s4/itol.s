/
/ Convert pair of integers to a long
/

.globl	_itol

_itol:
	mov	2(sp),r0
	mov	4(sp),r1
	rts	pc
