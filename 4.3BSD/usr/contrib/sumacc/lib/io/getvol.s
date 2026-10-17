	.insrt	"h.h"

|
|function getvol(volname: osstrptr| var vrefnum: integer):oserr|
|
|
		.text
		.globl	_getvol
_getvol:

	link	a6,#-iovqelsize	| get space for control p-block
	lea	a6@(-iovqelsize),a0	| point a0 at the block
	movl	a6@(12),a0@(iovnptr)	| pointer to volume name pointer
	.word	__getvol	| make the control call
	movw	d0,a6@(16)	| return the result code
	movl	a6@(8),a1	| pointer to drive number
	movw	a0@(iovrefnum),a1@	| return drive number

	unlk	a6
	movl	sp@+,a1	| return address
	addql	#8,sp	| fix up stack
	jmp	a1@

|
|function setvolume(volname: osstrptr| vrefnum: integer): oserr|
|

		.text
		.globl	_setvol
_setvol:

	link	a6,#-iovqelsize	|get space for control p-block
	lea	a6@(-iovqelsize),a0	| point a0 at the block
	movl	a6@(10),a0@(iovnptr)	| ptr to desired volume name
	movw	a6@(8),a0@(iovrefnum)	| desired default drive number
	.word	__setvol	| make the control call
	movw	d0,a6@(14)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addql	#6,sp	| fix up stack
	jmp	a1@
