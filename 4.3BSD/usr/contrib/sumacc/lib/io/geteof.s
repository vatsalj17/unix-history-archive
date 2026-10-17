	.insrt	"h.h"

|
|function geteof(refnum: integer| var logeof: longint):oserr|
|
	.text
	.globl	_geteof
_geteof:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	a6@(12),a0@(iorefnum)	| set refnum

	.word	__geteof
	movw	d0,a6@(14)	| return the result code
	movl	a6@(8),a1	|ptr to logical length variable
	movl	a0@(ioleof),a1@	|return logical end of file

	unlk	a6
	movl	sp@+,a1	| return address
	addql	#6,sp	| fix up stack
	jmp	a1@


|
|function seteof(refnum: integer| logeof: longint):oserr|
|
	.text
	.globl	_seteof
_seteof:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	a6@(12),a0@(iorefnum)	| set refnum
	movl	a6@(8),a0@(ioleof)	| the desired end of file

	.word	__seteof
	movw	d0,a6@(14)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addql	#6,sp	| fix up stack
	jmp	a1@
