	.insrt	"h.h"

|
|function getfpos(refnum: integer| var filepos: longint):oserr|
|
	.text
	.globl	_getfpos
_getfpos:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	a6@(12),a0@(iorefnum)	| set refnum

	.word	__getfpos
	movw	d0,a6@(14)	| return the result code
	movl	a6@(8),a1	|ptr to logical length variable
	movl	a0@(ioposoffset),a1@	|return logical end of file

	unlk	a6
	movl	sp@+,a1	| return address
	addql	#6,sp	| fix up stack
	jmp	a1@


|
|function setfpos(refnum: integer| posmode: integer| posoff: longint):oserr|
|
	.text
	.globl	_setfpos
_setfpos:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	a6@(14),a0@(iorefnum)	| set refnum
	movw	a6@(12),a0@(ioposmode)	| positioning information
	movl	a6@(8),a0@(ioposoffset)	| new file position

	.word	__setfpos
	movw	d0,a6@(16)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addql	#8,sp	| fix up stack
	jmp	a1@
