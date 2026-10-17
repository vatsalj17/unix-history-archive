/ $Header: $
		.unixorder

/ strlen () routine, as per the Standard C Library
/ $Log: $
/
		.text
		.globl	strlen

string		=	4

strlen:
		mov	%edi, %edx		/ Preserve %edi
		movl	string(%esp), %edi	/ String start

		movl	$-1, %ecx		/ Maximum string length
		xorl	%eax, %eax		/ Looking for 0
		cld				/ Scan upwards
		repne scasb			/ Find end-of-string

		movl	$-2, %eax		/ Inital %ecx - 1
		subl	%ecx, %eax		/ Get iteration count - 1

		movl	%edx, %edi		/ Restore %edi
		ret

