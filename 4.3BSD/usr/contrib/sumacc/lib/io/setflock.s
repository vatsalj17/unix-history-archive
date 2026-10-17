	.insrt	"h.h"

|
|function setflock(filename: osstr255| vrefnum: integer):oserr|
|
|
		.text
		.globl	_setflock
_setflock:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movl	a6@(10),a0@(iofilename)	| set ptr to filename
	movw	a6@(8),a0@(iovrefnum)	| set volume ref num
	clrb	a0@(iofiletype)	| clear type, permissions (version field)

	.word	__setfillock
	movw	d0,a6@(14)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addql	#6,sp	| fix up stack
	jmp	a1@
