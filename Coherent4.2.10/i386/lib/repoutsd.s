/ $Header: $
		.unixorder
		.include	"repeat.inc"

/ repoutsd (), as per the System V DDI/DKI:
/	void repoutsd (int _port, ulong_t * _addr, int _count);
/
/ The header file <kernel/x86io.h> contains the prototype.
/ $Log: $
/
		.text
		.globl	repoutsd

port		=	4
addr		=	8
count		=	12

repoutsd:
		mov	%esi, %eax		/ save %esi

		mov	port(%esp), %edx	/ port
		mov	addr(%esp), %esi	/ addr
		mov	count(%esp), %ecx	/ count

		cld
		repeat	outsl

		mov	%eax, %esi		/ restore %esi
		ret


