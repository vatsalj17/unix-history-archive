/ rk05 disk driver

rkda = 177412
	mov	dska,r1
	clr	r0
/	div	$12.,r0
0:
	sub	$12.,r1
	bmi	0f
	inc	r0
	br	0b
0:
	add	$12.,r1
/	ash	$4.,r0
	asl	r0
	asl	r0
	asl	r0
	asl	r0
	bis	r1,r0
	mov	$rkda,r1
	mov	r0,(r1)
	mov	ba,-(r1)
	mov	wc,-(r1)
	mov	$iocom,-(r1)
1:
	tstb	(r1)
	bpl	1b
	rts	pc
