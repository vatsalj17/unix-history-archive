.text
	/sleep
	/suspend execution for an interval
	/
	/ modified 30-3-78 GGR to sleep "forever" if no args given-
	/ but always can be rubbed out in this case.
	cmp	*sp,$2
	bmi	2f
	mov	4(sp),r3
	clr	r1
0:	movb	(r3)+,r2
	beq	1f
	sub	$'0,r2
	bmi	2f
	cmp	$9.,r2
	bmi	2f
	mov	r1,r0
	mul	$10.,r0
	add	r2,r1
	adc	r0
	beq	0b
	br	2f
1:	mov	r1,r0
	sys	35.	/sleep
	clr	r0
	bec	3f
2:
	sys	signal;	2;	0	/ guarantee rub-out-able
4:
	mov	$77777,r0
	sys	35.	/sleep
	br	4b
3:	sys	exit
