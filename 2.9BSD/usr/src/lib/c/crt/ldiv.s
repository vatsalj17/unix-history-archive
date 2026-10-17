.globl ldiv
/ long divide (fp)
/ called: 2(sp): LHS 6(sp):RHS

ldiv:
	setl
	movif	2(sp),r0
	movif	6(sp),r1
	divf	r1,r0
	movfi	r0,-(sp)
	mov	(sp)+,r0
	mov	(sp)+,r1
	seti
	rts	pc
