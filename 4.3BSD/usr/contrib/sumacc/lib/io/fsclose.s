	.insrt	"h.h"

|
|function fsclose(refnum: integer): oserr|
|

	.text
	.globl	_fsclose
_fsclose:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block
	movw	a6@(8),a0@(iorefnum)	| get the refnum param
	.word	__close
	movw	d0,a6@(10)	| return the result code
	unlk	a6
	movl	sp@+,a0	| return address
	addql	#2,sp	| fix up stack
	jmp	a0@
