	.insrt	"h.h"

|
|function setftype(filename: osstr255| oldvers: signedbyte| vrefnum: integer|
|	newvers: signedbyte):oserr|
|
	.text
	.globl	_setftype
_setftype:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movl	a6@(16),a0@(iofilename)	| set ptr to filename
	movw	a6@(10),a0@(iovrefnum)	| set volume ref num
	movb	a6@(12),a0@(iofiletype)	| clear type, permissions (version field)
	movb	a6@(8),a0@(ionewtype)	|the new type (version field)

	.word	__setfiltype
	movw	d0,a6@(16)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addl	#12,sp	| fix up stack
	jmp	a1@
