	.insrt	"h.h"

|
|function status(refnum: integer| cscode: integer|
|	csparam: ptr): oserr|

	.text
	.globl	_status
_status:
	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	a6@(14),a0@(iorefnum)	| refnum
	movw	a6@(12),a0@(cscode)	| the control operation to be performed
	.word	__status
	movw	d0,a6@(16)	| return the result code
	lea	a6@(csparam-ioqelsize),a0 |ptr to the status information
	movl	a6@(8),a1	| ptr to where to put this information
	moveq	#22,d0	| max number of bytes to be moved
	.word	__blockmove	| copy the op specific params

	unlk	a6
	movl	sp@+,a1	| return address
	addl	#8,sp	| fix up stack
	jmp	a1@
