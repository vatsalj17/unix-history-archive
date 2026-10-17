
	.globl	_runerr
	.globl	_ckadd
	.globl	_cksub
	.globl	_ckmul
	.globl	lmul

	.text
_ckadd:
	push	bp
	mov	bp,sp
	mov	dx,*6(bp)
	mov	ax,*4(bp)
	add	ax,*8(bp)
	adc	dx,10(bp)
	jo	oflow
	pop	bp
	ret
_cksub:
	push	bp
	mov	bp,sp
	mov	dx,*6(bp)
	mov	ax,*4(bp)
	sub	ax,*8(bp)
	sbb	dx,*10(bp)
	jo	oflow
	pop	bp
	ret
_ckmul:
	push	bp
	mov	bp,sp
	push	*10(bp)
	push	*8(bp)
	push	*6(bp)
	push	*4(bp)
	call	lmul
	jo	oflow
	pop	bp
	ret
oflow:
	mov	cx,#0
	push	cx
	mov	cx,#203
	push	cx
        call	_runerr
