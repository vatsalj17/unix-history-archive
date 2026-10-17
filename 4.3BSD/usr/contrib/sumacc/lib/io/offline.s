	.insrt	"h.h"

|
|function offline(volname: osstrptr| vrefnum: integer): oserr|
| removed 12 apr 84

	.text
	.globl	_offline
_offline:

	link	a6,#-iovqelsize	| get space for control p-block
	lea	a6@(-iovqelsize),a0	| point a0 at the block
	movw	a6@(8),a0@(iovrefnum)	| drive to be unmounted
	movl	a6@(10),a0@(iovnptr)	| volume name pointer
	.word	__offline
	movw	d0,a6@(14)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addql	#6,sp	| fix up stack
	jmp	a1@
