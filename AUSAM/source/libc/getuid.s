
/ C library -- getuid

/ long	getuid();
/ unsigned	getefvt(), getreal();
/ union { struct { unsigned eftv_uid, real_uid; }; long luid; } uid;
/
/ uid.luid = getuid();
/

.globl	_getuid,_getreal,_getefvt

_getuid:
_getefvt:
	mov	r5,-(sp)
	mov	sp,r5
	sys	getuid
	mov	(sp)+,r5
	rts	pc

_getreal:
	jsr	pc,_getuid
	mov	r1,r0
	rts	pc
