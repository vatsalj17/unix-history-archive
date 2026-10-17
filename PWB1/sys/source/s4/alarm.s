/ C library - alarm, pause

.globl	_alarm
.globl	_pause
alarm = 27.
pause = 29.

_alarm:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	alarm
	mov	(sp)+,r5
	rts	pc

_pause:
	mov	r5,-(sp)
	mov	sp,r5
	sys	pause
	mov	(sp)+,r5
	rts	pc
