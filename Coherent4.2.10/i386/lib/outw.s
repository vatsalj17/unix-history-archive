/ $Header: $
		.unixorder

/ outw (), as per the System V DDI/DKI:
/ 	void outw (int _port, ushort_t _data);
/
/ The header file <kernel/x86io.h> contains the prototype.
/ $Log: $
/
		.text
		.globl	outw

port		=	4
data		=	8

outw:		movl	port(%esp), %edx	/ port
		movl	data(%esp), %eax	/ data
		outw	(%dx)
		ret
