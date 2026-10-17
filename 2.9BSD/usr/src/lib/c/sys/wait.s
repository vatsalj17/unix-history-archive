/ pid	= wait(status);
/	  int *status;

.globl	_wait
.globl	cerror

_wait:
	mov	r5,-(sp)
	mov	sp,r5
	sys	wait
	bec	1f
	jmp	cerror
1:
	tst	4(r5)
	beq	1f
	mov	r1,*4(r5)
1:
	mov	(sp)+,r5
	rts	pc
