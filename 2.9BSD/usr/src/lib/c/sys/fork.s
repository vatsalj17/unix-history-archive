/ pid	= fork();

.globl	_fork, _par_uid
.globl	cerror

_fork:
	mov	r5,-(sp)
	mov	sp,r5
	sys	fork
		br 1f
	bec	2f
	jmp	cerror
1:
	mov	r0,_par_uid
	clr	r0
2:
	mov	(sp)+,r5
	rts	pc
.bss
_par_uid:
 	.=.+2
