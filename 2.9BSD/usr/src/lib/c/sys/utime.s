/ error	= utime(file, timep);
/	  char *file;
/	  time_t timep[2];

.globl	_utime
.globl	cerror

_utime:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),0f+2
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	utime; 0:..; ..
