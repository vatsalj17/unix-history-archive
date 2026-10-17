#
#  BMOVE.S -- optimized block move routine.
#
#	@(#)bmove.VAX.s	8.1	12/31/84
#
.globl	_bmove
_bmove:
	.word	0x0030
	movc3	12(ap),*4(ap),*8(ap)
	ret
