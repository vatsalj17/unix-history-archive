/ C library -- limits

/ limits ( lnode-address , function )
/
/  limits >= 0 means success
/  limits == -1 means error

.globl	_limits, cerror

_limits:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),0f+2
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	60. ; 0:.. ; ..
