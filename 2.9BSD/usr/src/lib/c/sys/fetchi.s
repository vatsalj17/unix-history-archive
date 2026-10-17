/ error = fetchi(addr);
/	  int *addr;

.globl	_fetchi
.globl	cerror

_fetchi:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5), r0
	sys	local; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	fetchi
