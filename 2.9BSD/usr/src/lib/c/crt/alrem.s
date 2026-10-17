.globl alrem
/ long assign remainder (fp)
/ called: 2(sp): ptr to LHS, 4(sp):RHS

alrem:
	setl
	mov	2(sp),r0
	movif	(r0),fr0
	movif	4(sp),fr1
	movf	fr0,fr2
	movf	fr1,fr3
	divf	fr1,fr0
	modf	$40200,fr0
	mulf	fr3,fr1
	subf	fr1,fr2
	movfi	fr2,(r0)
	mov	2(r0),r1
	mov	(r0),r0
	seti
	rts	pc
