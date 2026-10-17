	.insrt	"h.h"

|
|functon getvinfo(drvnum: integer| volname: osstrptr| var vrefnum: integer|
|	var freebytes: longint): oserr|
|

		.text
		.globl	_getvinfo
_getvinfo:

	link	a6,#-iovqelsize	| get space for control p-block
	lea	a6@(-iovqelsize),a0	| point a0 at the block
	movl	a6@(16),a0@(iovnptr)	| volume name pointer
	movw	a6@(20),a0@(iovdrvnum)	| drive number
	clrw	a0@(iovolindex)	| no volume queue index
	.word	__getvolinfo	| make the control call
	movw	d0,a6@(22)	| return the result code
	movl	a6@(12),a1	| pointer to vrefnum
	movw	a0@(iovrefnum),a1@	| return volume ref num

	|now calculate the number of free bytes on the volume by multiplying
	|allocation block size * free allocation blocks
	|would ideally like to multiply long * integer but since mulu only
	|multiplies two ints, to allow for a larger than 16 bit allocation
	|block size we shift it right by 9 (since we know it to be a multiple
	|of 512 and then shift the result back.
	movl	a0@(iovalblksiz),d0	|num bytes in an allocation block
	asrl	#8,d0	|shift over by 8
	asrl	#1,d0	|and one more
	mulu	a0@(iovfrblk),d0	|multiply by number of free blocks
	asll	#1,d0	|shift back to the left
	asll	#8,d0
	movl	a6@(8),a1	|pointer to free bytes
	movl	d0,a1@	|return the value


	unlk	a6
	movl	sp@+,a1	| return address
	addl	#14,sp	| fix up stack
	jmp	a1@
