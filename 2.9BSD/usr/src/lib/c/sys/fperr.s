/ error = fperr(fpe);
/	  struct fperr *fpe;

.globl	_fperr
.globl	cerror

_fperr:
	mov	r5, -(sp)
	mov	sp, r5
	sys	local; 9f
	bec	1f
	jmp	cerror
1:
	mov	4(sp), r5
	mov	r0, (r5)+
	mov	r1, (r5)
	mov	(sp)+, r5
	rts	pc
.data
9:
	sys	fperr
