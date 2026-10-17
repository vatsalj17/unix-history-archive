/ $Header: $
		.unixorder
		.include	"repeat.inc"

/ repinsd (), as per the System V DDI/DKI:
/	void repinsd (int _port, ulong_t * addr, int _count);
/
/ The header file <kernel/x86io.h> contains the prototype.
/ $Log: $
/
		.text
		.globl	repinsd

port		=	4
address		=	8
count		=	12

repinsd:
		mov	%edi, %eax		/ save %edi

		mov	port(%esp), %edx	/ port
		mov	address(%esp), %edi	/ address
		mov	count(%esp), %ecx	/ count

		cld
		repeat	insl

		mov	%eax, %edi		/ restore %edi
		ret
