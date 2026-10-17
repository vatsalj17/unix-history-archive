/*	@(#)ioctl.s	2.2	SCCS id keyword	*/
/ C library -- ioctl

/ ioctl(fdes, command, arg)
/ struct * arg;
/
/ result == -1 if error

.globl	_ioctl, cerror

_ioctl:
	mov	r5,-(sp)
	mov	sp,r5
	mov	8(r5), -(sp)
	mov	6(r5), -(sp)
	mov	4(r5), -(sp)
	sys	ioctl+200
	bec	1f
	add	$6, sp
	jmp	cerror
1:
	add	$6, sp
	mov	(sp)+,r5
	clr	r0
	rts	pc
