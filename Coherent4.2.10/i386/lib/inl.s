/ $Header: $
		.unixorder

/ inl (), as per the System V DDI/DKI:
/ 	ulong_t inl (int _port);
/
/ The header file <kernel/x86io.h> contains the prototype.
/ $Log: $
/
		.text
		.globl	inl

port		=	4

inl:		mov	port(%esp), %edx	/ port
		inl	(%dx)
		ret
