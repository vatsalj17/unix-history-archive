.globl _ckadd
.globl _cksub
.globl _ckmul

.text
.align	1
_ckadd:	.word	0
	addl3	4(ap),8(ap),r0	# Perform addition
	jvs	oflow		# Branch if overflow
	ret			# Return result in r0

.align	1
_cksub: .word	0
	subl3	8(ap),4(ap),r0	# Perform subtraction
	jvs	oflow		# Branch if overflow
	ret			# Return result in r0

.align	1
_ckmul:	.word	0
	mull3	4(ap),8(ap),r0	# Perform multiplication
	jvs	oflow		# Branch if overflow
	ret			# Return result in r0

oflow:				# Got overflow on an operation
	pushl   $0
	pushl	$203
	calls	$1,_runerr	# runerr(203,0)
