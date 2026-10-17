/ $Header: $
		.unixorder

/ inb (), as per the System V DDI/DKI:
/ 	uchar_t inb (int _port);
/
/ The header file <kernel/x86io.h> contains the prototype.
/ $Log: $
/
		.text
		.globl	inb

port		=	4

inb:		mov	port(%esp), %edx	/ port
		sub	%eax, %eax		/ cheap zero-fill
		inb	(%dx)
		ret
