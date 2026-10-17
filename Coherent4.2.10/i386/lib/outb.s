/ $Header: $
		.unixorder

/ outb (), as per the System V DDI/DKI:
/ 	void outb (int _port, uchar_t _data);
/
/ The header file <kernel/x86io.h> contains the prototype.
/ $Log: $
/
		.text
		.globl	outb

port		=	4
data		=	8

outb:		movl	port(%esp), %edx	/ port
		movl	data(%esp), %eax	/ data
		outb	(%dx)
		ret
