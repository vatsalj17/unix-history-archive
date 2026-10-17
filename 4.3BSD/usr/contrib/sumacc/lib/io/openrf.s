	.insrt	"h.h"

|
|function openrf(filename: osstr255| vrefnum: integer|var refnum: integer): oserr|
|
|

	.text
	.globl	_openrf
_openrf:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movl	a6@(14),a0@(iofilename)	| set ptr to filename
	movw	a6@(12),a0@(iovrefnum)	| set volume ref num
	clrb	a0@(iofiletype)	| clear type, permissions (version field)

	clrb	a0@(iopermssn)	| open for read/write
	clrl	a0@(ioownbuf)	| use system buffer
	.word	__openrf		| open resource fork
	movw	d0,a6@(18)	| return the result code
	movl	a6@(8),a1	|ptr to refnum
	movw	a0@(iorefnum),a1@	| return the refnum

	unlk	a6
	movl	sp@+,a1	| return address
	addl	#10,sp	| fix up stack
	jmp	a1@
