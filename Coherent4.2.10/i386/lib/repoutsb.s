/ $Header: $
		.unixorder
		.include	"repeat.inc"

/ repoutsb (), as per the System V DDI/DKI:
/	void repoutsb (int _port, uchar_t * _addr, int _count);
/
/ The header file <kernel/x86io.h> contains the prototype.
/ $Log: $
/
		.text
		.globl	repoutsb

port		=	4
addr		=	8
count		=	12

repoutsb:
		mov	%esi, %eax		/ save %esi

		mov	port(%esp), %edx	/ port
		mov	addr(%esp), %esi	/ addr
		mov	count(%esp), %ecx	/ count

		cld
		repeat	outsb

		mov	%eax, %esi		/ restore %esi
		ret
