.globl almul
/ long assign multiply (fp)
/ called: 2(sp): ptr to LHS, 4(sp):RHS

almul:
	setl
	mov	2(sp),r0
	movif	(r0),fr0
	movif	4(sp),fr1
	mulf	fr1,fr0
	movfi	fr0,(r0)
	mov	2(r0),r1
	mov	(r0),r0
	seti
	rts pc
