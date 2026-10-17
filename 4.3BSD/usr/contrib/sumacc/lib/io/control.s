	.insrt	"h.h"

|function control(refnum: integer| cscode: integer|
|	var csparam: ptr): oserr|

	.text
	.globl	_control
_control:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	a6@(14),a0@(iorefnum)	| refnum
	movw	a6@(12),a0@(cscode)	| the control operation to be performed
	tstl	a6@(8)	| see if there is a parameter block
	beq	.L5	| if not, don't bother copying one
	lea	a0@(csparam),a1	| dest address of op specific params
	movl	a6@(8),a0	| src address of op specific params
	moveq	#22,d0	| max number of bytes to be moved
	.word	__blockmove	| copy the op specific params
	lea	a6@(-ioqelsize),a0	| put cmd block addr back into a0

.L5:	.word	__control
	movw	d0,a6@(16)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addl	#8,sp	| fix up stack
	jmp	a1@
