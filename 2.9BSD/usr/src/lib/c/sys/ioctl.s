/ error	= ioctl(fildes, request, argp);
/	  struct sgttyb *argp;

.globl	_ioctl, cerror

_ioctl:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),0f+2
	mov	8(r5),0f+4
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	clr	r0
	rts	pc
.data
9:
	sys	ioctl; 0:..; ..; ..
