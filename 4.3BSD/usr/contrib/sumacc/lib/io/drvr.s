	.insrt	"h.h"

|
| from macmisc.text
|	miscellanous macintosh interface routines
|	this version for traps based system
|	it also includes stortraps
|
|
|	written by andy hertzfeld	16-oct-82
|
|	modification history
|
|	25-jan-83	ajh	modified for new file/io system
|	12-jun-83	ajh	added opendeskacc
|
|------------------------------------------------


|
|function vinstall(vblblockptr: vblcbptr): oserr;
|

		.text
		.globl	_vinstall
_vinstall:

	movl	sp@+,a1	|get the return address
	movl	sp@+,a0	|addr of vertical retrace control blk
	movl	a1,sp@-	|restore return address

	.word	__vinstall
	movw	d0,sp@(4)	|return error

	rts

|
|function vremove(vblblockptr: vblcbptr): oserr;
|

		.text
		.globl	_vremove
_vremove:

	movl	sp@+,a1	|get the return address
	movl	sp@+,a0	|addr of vertical retrace control blk
	movl	a1,sp@-	|restore return address

	.word	__vremove
	movw	d0,sp@(4)	|return error

	rts

|
|function drvrinstall(name: osstr255;	drvrrefnum: integer): oserr;
| installdriver installs a driver of the specified name, into the specified
| unit.
|
		.text
		.globl	_drvrinst
_drvrinst:

	movw	sp@(4),d0	|get unit number
	movl	sp@(6),a0	|get the filename ptr
	.word	__drvrinstall	|install it

	movl	sp@+,a0	|get return address
	addqw	#6,sp	|strip parameters
	movw	d0,sp@	|return error
	jmp	a0@	|return to caller

|
|function drvrremove(drvrrefnum: integer): oserr;
|

		.text
		.globl	_drvrremo
_drvrremo:

	movw	sp@(4),d0	|get driver refnum
	.word	__drvrremove	|remove it

	movl	sp@+,a0	|get return address
	addqw	#2,sp	|strip parameters
	movw	d0,sp@	|return error
	jmp	a0@	|return to caller

|
| function opendriver(drivername: osstr255): integer;
|
|	opendriver opens the driver of a given name, returning its refnum
|
		.text
		.globl	_opendriv
_opendriv:

	movl	sp@(4),a0	|get a pointer to the name
	moveq	#[ioqelsize/2]-1,d0
.L1:	clrw	sp@-
	dbra	d0,.L1

	movl	a0,sp@(iofilename)	|set up name
	clrb	sp@(iopermssn)	| open for read/write
	movl	sp,a0	|point to it
	.word	__open	|open the driver
	bnes	badopen	|if an error, don't return refnum

	movw	sp@(iorefnum),d0	|get the refnum
doneopen:
	addw	#ioqelsize,sp	|pop off command buffer
	movw	d0,sp@(8)	|return the refnum

	movl	sp@+,sp@	|strip return address
	rts
badopen:
	moveq	#0,d0
	bras	doneopen
|
| procedure closedriver(refnum: integer);
|
|	closedriver closes the driver with the specified refnum
|
		.text
		.globl	_closedri
_closedri:

	movw	sp@(4),d0	|get the refnum
	subw	#30,sp	|get space for param block
	movw	d0,sp@(iorefnum)	|set up refnum
	movl	sp,a0
	.word	__close	|close it!
	addw	#30,sp	|pop off param block

	movl	sp@+,a0
	addqw	#2,sp
	jmp	a0@

|
|function initutil: oserr;
|

		.text
		.globl	_initutil
_initutil:

	.word	__initutil
	movw	d0,sp@(4)	|return error

	rts

|queue routines
|
| procedure initqueue(qheader: qhdrptr);
|

		.text
		.globl	_initqueu
_initqueu:

	movl	sp@+,a0	|return address
	movl	sp@+,a1	|queue header ptr
	.word	__initqueue
	jmp	a0@	|return

|
|procedure enqueue(qelement: qelemptr; qheader: qhdrptr);
|
|adds qelement a0@ to queue with qheader a1@

		.text
		.globl	_enqueue
_enqueue:

	movl	sp@(4),a1	|ptr to qheader
	movl	sp@(8),a0	|ptr ot qelem to be added
	.word	__enqueue
	movl	sp@+,a0	|return address
	addqw	#8,sp	|pop off params
	jmp	a0@	|return

|function dequeue(qelement: qelemptr; qheader: qhdrptr): oserr;
|
		.text
		.globl	_dequeue
_dequeue:

	movl	sp@(4),a1	|ptr to qheader
	movl	sp@(8),a0	|ptr ot qelem to be added
	.word	__dequeue
	movw	d0,sp@(12)	|return error
	movl	sp@+,a0	|return address
	addqw	#8,sp	|pop off params
	jmp	a0@	|return



|function getdctlentry(refnum: integer): dctlhandle;
| does status 1 call to get the dcehandle

		.text
		.globl	_getdctle
_getdctle:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	a6@(8),a0@(iorefnum)	| drive number
	movw	#1,a0@(cscode)	|code to get dce handle
	.word	__status
	movl	a0@(csparam),a6@(10)	|return the dce handle

	unlk	a6
	movl	sp@+,a0	| return address
	addqw	#2,sp	| fix up stack
	jmp	a0@

