	.insrt	"h.h"

|
|function fsread(refnum: integer| var count: longint| buffptr: ptr): oserr|
|
|function fswrite(refnum: integer| var count: longint| buffptr: ptr): oserr|
|
|

		.text
		.globl	_fsread
_fsread:
	.globl	_fswrite

	clrl	d1	| set up read
	bra	doblk

_fswrite:
	moveq	#1,d1	| set up write

doblk:
	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movl	a6@(8),a0@(iobuffer)	| buffer address
	movw	a6@(16),a0@(iorefnum)	| refnum
	movl	a6@(12),a1	| address of the count
	movl	a1@,a0@(iobytecount)	| read count
	clrw	a0@(ioposmode)	| no special mode
	clrl	a0@(ioposoffset)	| clear offset

	tstb	d1	| look for read or write
	bne	.L3
	.word	__read
	bra	.L4
.L3:	.word	__write
.L4:
	movw	d0,a6@(18)	| return the result code

	movl	a6@(12),a1	| address of the count
	movl	a0@(ionumdone),a1@	| return count read

	unlk	a6
	movl	sp@+,a1	| return address
	addl	#10,sp	| fix up stack
	jmp	a1@
