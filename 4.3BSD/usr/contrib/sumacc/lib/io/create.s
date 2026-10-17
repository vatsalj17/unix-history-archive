	.insrt	"h.h"

|
|function create(filename: osstr255| vrefnum: integer| creator: ostype|
|	filetype: ostype):oserr|

	.text
	.globl	_create
_create:


	link	a6,#-iofqelsize	| make room on stack for cmd block
	lea	a6@(-iofqelsize),a0	| address of cmd block

	movl	a6@(18),a0@(iofilename)	| set ptr to filename
	movw	a6@(16),a0@(iovrefnum)	| set volume ref num
	clrb	a0@(iofiletype)	| clear type, permissions (version field)

	.word	__create
	tstw	d0	|error?
	bne	crdone	|yes, don't bother setting type

	clrw	a0@(iofdirindex)	|clear directory index
	.word	__getfileinfo	|get the previous info

	|transfer the user defined finder info words (16 bytes)
	movl	a0,d0	|_save ptr to param block
	lea	a0@(ioflusrwds),a1	|where to copy user words
	movl	a6@(8),a1@+	|enter in the new file type
	movl	a6@(12),a1@	|enter in the new file creator
	.word	__setfileinfo

crdone:
	movw	d0,a6@(22)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addl	#14,sp	| fix up stack
	jmp	a1@
