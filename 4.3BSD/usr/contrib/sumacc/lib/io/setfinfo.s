	.insrt	"h.h"

|
|function setfinfo(filename: osstr255| vrefnum| integer|
|	fndrinfo: finfo):oserr|
|
	.text
	.globl	_setfinfo
_setfinfo:

	link	a6,#-iofqelsize	| make room on stack for cmd block
	lea	a6@(-iofqelsize),a0	| address of cmd block

	movl	a6@(14),a0@(iofilename)	| set ptr to filename
	movw	a6@(12),a0@(iovrefnum)	| set volume ref num
	clrb	a0@(iofiletype)	| (version field)
	clrw	a0@(iofdirindex)	|clear directory index

	.word	__getfileinfo	|get the previous info

	|transfer the user defined finder info words (16 bytes)
	lea	a0@(ioflusrwds),a1	|where to copy user words
	movl	a6@(8),a0	|where the user words are now
	movw	#16,d0	| 16 bytes of information
	.word	__blockmove
	lea	a6@(-iofqelsize),a0	|restore a0

	.word	__setfileinfo
	movw	d0,a6@(18)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addl	#10,sp	| fix up stack
	jmp	a1@
