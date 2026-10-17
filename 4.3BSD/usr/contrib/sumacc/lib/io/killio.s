	.insrt	"h.h"

|
|function killio(refnum: integer): oserr|
|
		.text
		.globl	_killio
_killio:

	link	a6,#-32	|get space for control p-block
	lea	a6@(-32),a0	|point a0 at the block
	movw	a6@(8),a0@(iorefnum)	| refnum
	.word	__killio	| make the control call
	movw	d0,a6@(10)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addl	#2,sp	| fix up stack
	jmp	a1@
