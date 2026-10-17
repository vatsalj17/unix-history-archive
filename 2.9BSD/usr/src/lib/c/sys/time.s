/ now	= time(tloc);
/	  long *tloc;
/
/ (void) ftime(tp);
/	 struct timeb *tp;

.globl	_time


_time:
	mov	r5,-(sp)
	mov	sp,r5
	sys	time
	mov	r2,-(sp)
	mov	4(r5),r2
	beq	1f
	mov	r0,(r2)+
	mov	r1,(r2)+
1:
	mov	(sp)+,r2
	mov	(sp)+,r5
	rts	pc

.globl	_ftime

_ftime:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	sys	0; 9f
	mov	(sp)+,r5
	rts	pc

.data
9:
	sys	ftime; 0:..
