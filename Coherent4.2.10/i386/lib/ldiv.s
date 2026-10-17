/ $Header: $
		.unixorder

/ ldiv () library routine, exactly as per the Standard C library. Note
/ that we have to use the PCC structure-return convention!
/ $Log: $
/
		.text
		.globl	ldiv

numerator	=	4
denominator	=	8

ldiv:
		movl	numerator(%esp),%eax
		cltd				/ sign-extend %eax into %edx
		movl	denominator(%esp),%ecx
		div	%ecx			/ %eax -> quotient
						/ %edx -> remainder
		movl	%eax, quotient
		movl	%edx, remainder
		movl	$quotient, %eax		/ return ptr to structure
		ret

		.data

quotient:	.long	0
remainder:	.long	0

