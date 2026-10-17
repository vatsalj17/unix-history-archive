/ $Header: $
		.unixorder

/ memset () routines, exactly as per the Standard C Library.
/ $Log: $
/
		.text
		.globl	memset

dest		=	4
ch		=	8
len		=	12

memset:
		movzxb	ch(%esp), %eax		/ Fill pattern to %eax
		movl	$0x01010101, %ecx	/ Replicate fill byte into
		mull	%ecx, %eax		/ all of %eax (4 copies)
						/ (overwrites %edx too)

		movl	%edi, %edx		/ Save %edi
		movl	dest(%esp), %edi	/ Get dest to %edi

		movl	len(%esp), %ecx		/ Get length
		shrl	$2, %ecx		/ in longwords

		cld				/ Fill upwards
		rep stosl			/ Perform the fill

		jnc	?noword			/ Skip over residual word copy

		stosw
?noword:
		testb	$1, len(%esp)		/ Check low part of count
		je	?nobyte			/ Skip over residual byte copy

		stosb
?nobyte:
		movl	%edx, %edi		/ Restore %edi
		movl	dest(%esp), %eax	/ Return destination
		ret


