/ error	= login(flag, crn)
/	  char crn[4];

.globl	_login, cerror

_login:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),0f+2
	mov	10(r5),0f+4
	sys	local; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	login; 0:..; ..; ..;
