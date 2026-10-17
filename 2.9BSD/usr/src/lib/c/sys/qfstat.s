/ error	= qfstat(fildes, buf);
/	  struct qstat *buf;

.globl	_qfstat
.globl	cerror

_qfstat:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	6(r5),0f
	sys	local; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	qfstat; 0:..
