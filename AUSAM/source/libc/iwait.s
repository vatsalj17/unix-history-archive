/ C library -- wait for init

/ pid = waitx(&corpse);
/ pid == -1 if error
/ corpse holds zombie area and limits struct

.globl	_iwait, cerror

_iwait:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	wait
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
