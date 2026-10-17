	.insrt	"h.h"

|
|function rename(oldname: osstr255| vrefnum: integer|
|	newname: osstr255):oserr|
|
		.text
		.globl	_rename
_rename:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movl	a6@(14),a0@(iofilename)	| set ptr to filename
	movw	a6@(12),a0@(iovrefnum)	| set volume ref num
	clrb	a0@(iofiletype)	| clear type, permissions (version field)
	movl	a6@(8),a0@(ionewname)	|new name to give to file

	.word	__rename
	movw	d0,a6@(18)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addl	#10,sp	| fix up stack
	jmp	a1@
