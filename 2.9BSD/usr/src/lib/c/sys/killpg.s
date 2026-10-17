/ error	= killpg(pgrp, sig);

.globl	_killpg
.globl	cerror

_killpg:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(sp),0f
	mov	6(sp),0f+2
	sys	local; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

.data
9:
	sys	killpg; 0:..; ..
