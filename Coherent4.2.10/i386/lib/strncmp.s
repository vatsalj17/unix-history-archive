/ $Header: $
		.unixorder

/ strncmp () routine, exactly as per the Standard C Library
/ $Log: $
/
		.text
		.globl	strncmp

left		=	4
right		=	8
len		=	12

strncmp:
		movl	len(%esp), %ecx		/ Maximum length
		jecxz	?strncmp_empty

		movl	%esi, %edx		/ Preserve %esi
		movl	left(%esp), %esi	/ Get left

		pushl	%edi			/ Preserve %edi
		movl	right+4(%esp), %edi	/ Get right, adjusting the
						/ stack offset for push

		cld				/ Scan upwards
		xorl	%eax, %eax		/ Zero top bytes in %eax

?strncmp_loop:	lodsb				/ %al = * %ds:%esi ++
		scasb				/ * %es:%edi ++ - %al ?
		jne	?strncmp_noteq
		orb	%al, %al		/ %al == 0 ?
		loopne	?strncmp_loop

/ Either %al == 0 or %ecx == 0, and we have a match. Zero %eax
		xorl	%eax, %eax
		popl	%edi			/ Restore %edi
		movl	%edx, %esi		/ Restore %esi
		ret

?strncmp_noteq:
		movb	$1, %al
		ja	?strncmp_less		/ Branch if %al was > * right
						/ hence return %eax == 1
		negl	%eax			/ Here if %al < * right,
						/ hence return %eax == -1
?strncmp_less:
		popl	%edi			/ Restore %edi
		movl	%edx, %esi		/ Restore %esi
		ret

?strncmp_empty:	xorl	%eax, %eax		/ Count == 0 => left == right
		ret


