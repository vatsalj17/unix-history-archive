/ (void) times(buffer);
/	 struct tbuffer *buffer;

.globl	_times

_times:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	sys	0; 9f
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	times; 0:..
