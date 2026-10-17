	.globl	_runerr
	.globl	_ckadd
	.globl	_cksub
	.globl	_ckmul

	.globl	csv, cret
	.text
_ckadd:
	mov	r5,-(sp)
	mov	sp,r5
	mov     6(r5),r1
        mov	4(r5),r0
        add	12(r5),r1
        adc	r0
	bvs	oflow
        add	10(r5),r0
	bvs	oflow
        br      return

_cksub:
	mov	r5,-(sp)
	mov	sp,r5
        mov   	6(r5),r1
        mov	4(r5),r0
        sub	12(r5),r1
        sbc	r0
	bvs	oflow
        sub	10(r5),r0
	bvs	oflow
return:
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc

_ckmul:
	jsr	r5,csv
	mov	6(r5),r2
	sxt	r1
	sub	4(r5),r1
	mov	10.(r5),r0
	sxt	r3
	sub	8.(r5),r3
	mul	r0,r1
	mul	r2,r3
	add	r1,r3
	mul	r2,r0
	sub	r3,r0
	jmp	cret

oflow:
	clr     -(sp)
	mov	$203.,-(sp)
	jsr	pc,_runerr
