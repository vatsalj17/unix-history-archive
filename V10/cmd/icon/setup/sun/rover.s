	.globl	_runerr
	.globl	_ckadd
	.globl	_cksub
	.globl	_ckmul


	.text
	.even
_ckadd:
	link	a6,#0
	movl	a6@(8),d0	| Perform addition
	addl	a6@(12),d0	|  " " "
	jvs	oflow		| Branch if overflow
	unlk	a6
	rts			| Return result in d0

	.even

_cksub:
	link	a6,#0
	movl	a6@(8),d0	| Perform subtraction
	subl	a6@(12),d0	|  " " "
	jvs	oflow		| Branch if overflow
        unlk    a6
	rts			| Return result in d0

	.even

_ckmul:
	link	a6,#0
	movl	a6@(8),d0	| Perform multiplication
	movl	a6@(12),d1
	jsr	lmult
	jvs	oflow		| Branch if overflow
        unlk    a6
	rts			| Return result in d0

oflow:				| Got overflow on an operation
	pea	0
	pea	203
	jbsr	_runerr		| runerr(203,0)
