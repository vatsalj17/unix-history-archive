/ C interface to spider status call

.globl	_snstat
.globl	cerror

_snstat:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	6(r5),0f
	mov	8(r5),0f+2
	sys	stty; 0f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

.data
0:	.=.+6
