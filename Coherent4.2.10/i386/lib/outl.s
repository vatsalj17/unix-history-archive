/ $Header: $
		.unixorder

/ outl (), as per the System V DDI/DKI:
/ 	void outl (int _port, ulong_t _data);
/
/ The header file <kernel/x86io.h> contains the prototype.
/ $Log: $
/
		.text
		.globl	outl

port		=	4
data		=	8

outl:		movl	port(%esp), %edx	/ port
		movl	data(%esp), %eax	/ data
		outl	(%dx)
		ret


