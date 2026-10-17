/ $Header: $
		.unixorder

/ Relation-compare abstract processor priority levels; use in debugging
/ assertion code only.
/	int splcmp (pl_t l, pl_t r)
/ Prototyped in <sys/inline.h>
/ $Log: $
/
		.text
		.globl	splcmp

splcmp:		movzxb	4(%esp), %eax			/ left
		sub	4(%esp), %eax			/ - right
		jz	l_eq_r				/ if eq, %eax == 0

/ take advantage of 80x86 mov not changing condition codes.

		mov	$-1, %eax
		jl	l_lt_r				/ if <, %eax == -1
		mov	$1, %eax			/ if >, %eax == 1
l_eq_r:
l_lt_r:
		ret

