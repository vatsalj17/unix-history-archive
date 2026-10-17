/
/

/ catch runtime errors:
/ if (ierror(errno)) goto place
/ on call, returns 0
/ on occurrence of error, returns non-zero

ESIZE = 14.
.globl	ierror.
.globl	erret

.globl	rerr
.globl	ent,lin,disp
.globl	retrn

ierror.:zero
	.+2
	mov	2(r3),r0
	mov	2(r0),r1
	mov	$errbuf,r0
1:
	cmp	r0,cerrp
	blo	2f
	cmp	cerrp,$eerbuf
	blo	3f
	jsr	r5,rerr; 120.
3:
	add	$ESIZE,cerrp
	br	1f
2:
	cmp	r1,(r0)
	beq	1f
	add	$ESIZE,r0
	br	1b
1:
	mov	r1,(r0)+
	mov	r3,(r0)+
	mov	2(sp),(r0)+
	mov	4(sp),(r0)+
	mov	6(sp),(r0)+
	mov	10(sp),(r0)+
	mov	12(sp),(r0)+
	cmp	r1,$113.
	bne	1f
	sys	signal; 2; intrp
1:
	jmp	retrn

intrp:
	sys	signal; 2; intrp
	jsr	r5,rerr; 113.

.data
erret:
	jmp	9f
.text
9:
	mov	$errbuf,r0
1:
	cmp	(r0),r1
	beq	1f
	add	$ESIZE,r0
	cmp	r0,cerrp
	blo	1b
	rts	pc
1:
	add	$ESIZE,r0
	mov	-(r0),-(sp)
	mov	-(r0),-(sp)
	mov	-(r0),-(sp)
	mov	-(r0),-(sp)
	mov	-(r0),-(sp)
	mov	$one,-(sp)
	mov	-(r0),r3
	jmp	retrn

.data
one:	77777; 177777
zero:	0; 0

cerrp:	errbuf

.bss
errbuf:	.=.+[5*ESIZE]
eerbuf:
