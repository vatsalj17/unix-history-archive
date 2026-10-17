/ error	= alarm(seconds);
/	  unsigned seconds;

.globl	_alarm
.globl	cerror

_alarm:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	alarm
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
