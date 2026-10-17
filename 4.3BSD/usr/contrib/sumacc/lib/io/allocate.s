	.insrt	"h.h"

|
|function allocate(refnum: integer| var count: longint):oserr|
|

	.text
	.globl	_allocate
_allocate:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movl	a6@(8),a1	|ptr to requested byte count
	movl	a1@,a0@(ioreqcount)	|
	movw	a6@(12),a0@(iorefnum)	| set refnum

	.word	__allocate
	movw	d0,a6@(14)	| return the result code
	movl	a0@(ioactcount),a1@	| return actual byte count

	unlk	a6
	movl	sp@+,a1	| return address
	addql	#6,sp	| fix up stack
	jmp	a1@
