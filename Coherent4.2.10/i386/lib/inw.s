/ $Header: $
		.unixorder

/ inw (), as per the System V DDI/DKI:
/ 	ushort_t inw (int _port);
/
/ The header file <kernel/x86io.h> contains the prototype.
/ $Log: $
/
		.text
		.globl	inw

port		=	4

inw:		mov	port(%esp), %edx	/ port
		sub	%eax, %eax		/ zero-fill
		inw	(%dx)
		ret
