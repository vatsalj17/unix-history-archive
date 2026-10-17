.globl lrem
/ long remainder (fp)
/ called: 2(sp): LHS 6(sp):RHS

lrem:
	setl
	movif	2(sp),r0
	movf	r0,r2
	movif	6(sp),r1
	movf	r1,r3
	divf	r1,r0
	modf	$40200,r0
	mulf	r3,r1
	subf	r1,r2
	movfi	r2,-(sp)
	mov	(sp)+,r0
	mov	(sp)+,r1
	seti
	rts	pc
