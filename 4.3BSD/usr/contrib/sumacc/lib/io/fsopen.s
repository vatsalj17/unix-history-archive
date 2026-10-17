	.insrt	"h.h"

|simple interface to io routines which does not require caller to set up the
|parameter block
|
|function fsopen(filename: osstr255| versnum: signedbyte| vrefnum: integer|
|	var refnum: integer): oserr|
|
	.text
	.globl	_fsopen
_fsopen:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movl	a6@(14),a0@(iofilename)	| set ptr to filename
	movw	a6@(12),a0@(iodrvnum)	| drive number
	clrb	a0@(iofiletype)	| file type
	clrb	a0@(iopermssn)	| open for read/write
	clrl	a0@(ioownbuf)	| use system buffer

	.word	__open

	movl	a6@(8),a1	| return the refnum
	movw	a0@(iorefnum),a1@
	movw	d0,a6@(18)	| return the result code

	unlk	a6
	movl	sp@+,a0	| return address
	addl	#10,sp	| fix up stack
	jmp	a0@
