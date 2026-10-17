/ $Header: $
		.unixorder
		.include	"repeat.inc"

/ repinsw (), as per the System V DDI/DKI:
/	void repinsw (int _port, ushort_t * _addr, int _count);
/
/ The header file <kernel/x86io.h> contains the prototype.
/ $Log: $
/
		.text
		.globl	repinsw

port		=	4
address		=	8
count		=	12

repinsw:
		mov	%edi, %eax		/ save %edi

		mov	port(%esp), %edx	/ port
		mov	address(%esp), %edi	/ address
		mov	count(%esp), %ecx	/ count

		cld
		repeat	insw

		mov	%eax, %edi		/ restore %edi
		ret
