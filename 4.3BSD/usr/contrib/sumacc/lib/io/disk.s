	.insrt	"h.h"

|function diskeject(drvnum: integer): oserr;
|

		.text
		.globl	_diskejec
_diskejec:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	#dskrfn,a0@(iorefnum)	| refnum of the sony disk
	movw	#ejectcode,a0@(cscode)	| code for eject
	movw	a6@(8),a0@(iodrvnum)	|the drvnum of drv to be eject

	.word	__control
	movw	d0,a6@(10)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addqw	#2,sp	| fix up stack
	jmp	a1@

|function settagbuffer(buffptr: ptr): oserr;
|

		.text
		.globl	_settagbu
_settagbu:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	#dskrfn,a0@(iorefnum)	| refnum of the sony disk
	tgbuffcode=8
	movw	#tgbuffcode,a0@(cscode)	| code for settagbuffer
	movl	a6@(8),a0@(csparam)	|csparam is the buffptr

	.word	__control
	movw	d0,a6@(12)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addqw	#4,sp	| fix up stack
	jmp	a1@

|function drivestatus(drvnum: integer; var status: drvsts): oserr;
|

		.text
		.globl	_drivesta
_drivesta:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	#dskrfn,a0@(iorefnum)	| refnum of the sony disk
	movw	#drvstscode,a0@(cscode)	| code for settagbuffer
	movw	a6@(12),a0@(iodrvnum)	|the drvnum of drv to be ejected

	.word	__status
	movw	d0,a6@(14)	| return the result code
	movl	a6@(8),a1	| ptr to where to put the status bytes
	lea	a0@(csparam),a0	|ptr from where to get status bytes
	movl	#22,d0	|number of bytes to move
	.word	__blockmove

	unlk	a6
	movl	sp@+,a1	| return address
	addqw	#6,sp	| fix up stack
	jmp	a1@

