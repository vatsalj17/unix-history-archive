/ $Header: $
		.unixorder
		.include	"repeat.inc"

/ repinsb (), as per the System V DDI/DKI:
/ 	void repinsb (int _port, uchar_t * _addr, int _count);
/
/ The header file <kernel/x86io.h> contains the prototype.
/ $Log: $
/
		.text
		.globl	repinsb

port		=	4
address		=	8
count		=	12

repinsb:
		mov	%edi, %eax		/ save %edi

		mov	port(%esp), %edx	/ port
		mov	address(%esp), %edi	/ address
		mov	count(%esp), %ecx	/ count

		cld
		repeat	insb

		mov	%eax, %edi		/ restore %edi
		ret
