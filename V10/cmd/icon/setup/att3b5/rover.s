	.file "rover.s"

	.globl	ckadd
	.globl	cksub
	.globl	ckmul

 	.text
 	.align	1
ckadd:	save	&0
	addw3	4(%ap),0(%ap),%r0	# Perform addition
	BVSH	oflow		# Branch if overflow
	ret	&0		# Return result in r0

 	.align	4
cksub:	save	&0
	subw3	4(%ap),0(%ap),%r0 #	Perform subtraction
	BVSH	oflow		# Branch if overflow
	ret	&0		# Return result in r0

 	.align	4
ckmul:	save 	&0
	mulw3	4(%ap),0(%ap),%r0 #	Perform multiplication
	BVSH	oflow		# Branch if overflow
	ret	&0		# Return result in r0

oflow:				# Got overflow on an operation
	pushw	&203
	pushw	&0
	call	&2,runerr		# runerr(203,0)



