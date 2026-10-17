/ setpgrp -- set process group

setpgrp	= 39.

.globl	_setpgrp

_setpgrp:
	mov	r5,-(sp)
	mov	sp,r5
	sys	setpgrp
	mov	(sp)+,r5
	rts	pc

