.globl	_sleep
alarm = 27.
pause = 29.
rti = 2

_sleep:
	mov	r5,-(sp)
	mov	sp,r5
	sys	signal; 14.; 1f
	mov	4(r5),r0
	sys	alarm
	sys	pause
	clr	r0
	sys	alarm
	mov	(sp)+,r5
	rts	pc

1:
	rti
