	.insrt	"h.h"

|
|function getfinfo(filename: osstr255| versnum: signedbyte| vrefnum: integer|
|	var fndrinfo: finfo):oserr|
|
		.text
		.globl	_getfinfo
_getfinfo:


	link	a6,#-iofqelsize	| make room on stack for cmd block
	lea	a6@(-iofqelsize),a0	| address of cmd block

	movl	a6@(14),a0@(iofilename)	| set ptr to filename
	movw	a6@(12),a0@(iovrefnum)	| set volume ref num
	clrb	a0@(iofiletype)	| (version field)
	clrw	a0@(iofdirindex)	|clear directory index

	.word	__getfileinfo	|get the info
	movw	d0,a6@(18)	| return the result code

	|transfer the user defined finder info words (16 bytes) into result
	lea	a0@(ioflusrwds),a0	|where to copy from
	movl	a6@(8),a1	|where to copy to
	movw	#16,d0	| 16 bytes of information
	.word	__blockmove

	unlk	a6
	movl	sp@+,a1	| return address
	addl	#10,sp	| fix up stack
	jmp	a1@
