/	delay
/
/	delay( tics )
/	  unsigned tics;

delay	=	51.
.globl	_delay

_delay:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	delay
	bes	9f
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
9:
	jmp	cerror

.globl	cerror
