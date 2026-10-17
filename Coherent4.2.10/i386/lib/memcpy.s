/ $Header: $
		.unixorder

/ memcpy () library routine, exactly as per the Standard C library.
/ $Log: $
/
		.text
		.globl	memcpy

dest		=	4
src		=	8
len		=	12

memcpy:
		movl	%esi, %eax		/ Save %esi
		movl	src(%esp), %esi		/ Get src

		movl	%edi, %edx		/ Save %edi
		movl	dest(%esp), %edi	/ Get dest

		movl	len(%esp), %ecx		/ Get length
		shrl	$2, %ecx		/ in longwords

		cld				/ Copy upwards
		rep movsl			/ in longwords

		jnc	?noword			/ Skip residual word copy

		movsw
?noword:
		testb	$1, len(%esp)		/ Check low byte of length
		je	?nobyte			/ Skip residual byte copy

		movsb
?nobyte:
		movl	%edx, %edi		/ Restore %edi
		movl	%eax, %esi		/ Restore %esi
		movl	dest(%esp), %eax	/ Return destination
		ret

