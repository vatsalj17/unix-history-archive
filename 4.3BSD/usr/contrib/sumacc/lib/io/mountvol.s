	.insrt	"h.h"

|
|function mountvol(drvnum: integer| var vrefnum):oserr|
|
|
|		.text
|		.globl	_mountvo
|_mountvo:
|
|	link	a6,#-iovqelsize	|get space for control p-block
|	lea	a6@(-iovqelsize),a0	| point a0 at the block
|	movw	a6@(12),a0@(iovdrvnum)	| drive to be mounted
|	.word	__mountvol	| make the control call
|	movw	d0,a6@(14)	| return the result code
|	movl	a6@(8),a1
|	movw	a0@(iovrefnum),a1@	| return volume reference number
|
|	unlk	a6
|	movl	sp@+,a1	| return address
|	addql	#6,sp	| fix up stack
|	jmp	a1@

|
|function unmountvol(volname: osstrptr| vrefnum: integer):oserr|
|

		.text
		.globl	_unmountv
_unmountv:

	link	a6,#-iovqelsize	|get space for control p-block
	lea	a6@(-iovqelsize),a0	| point a0 at the block
	movw	a6@(8),a0@(iovrefnum)	| drive to be unmounted
	movl	a6@(10),a0@(iovnptr)	| volume name pointer
	.word	__unmountvol
	movw	d0,a6@(14)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addql	#6,sp	| fix up stack
	jmp	a1@
