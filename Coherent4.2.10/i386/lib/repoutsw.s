/ $Header: $
		.unixorder
		.include	"repeat.inc"

/ repoutsw (), as per the System V DDI/DKI:
/	void repoutsw (int _port, ushort_t * _addr, int _count);
/
/ The header file <kernel/x86io.h> contains the prototype.
/ $Log: $
/
		.text
		.globl	repoutsw

port		=	4
addr		=	8
count		=	12

repoutsw:
		mov	%esi, %eax		/ save %esi

		mov	port(%esp), %edx	/ port
		mov	addr(%esp), %esi	/ addr
		mov	count(%esp), %ecx	/ count

		cld
		repeat	outsw

		mov	%eax, %esi		/ restore %esi
		ret
