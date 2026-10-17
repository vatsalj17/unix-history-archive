/ rb -- funct/subr stuff

.globl	call
.globl	callp
.globl	retrn
.globl	lino
.globl	entry
.globl	trace
.globl	pdec
.globl	temp
.globl	stsp
.globl	ent,lin,disp

callp:
	mov	(r4)+,r0
	add	r3,r0
	mov	(r0),r0
	br	1f

call:
	mov	(r4)+,r0
1:
	mov	disp,-(sp)
	mov	lin,-(sp)
	mov	ent,-(sp)
	mov	sp,disp
	mov	r3,-(sp)
	mov	(r4)+,r3
	mov	r4,-(sp)
	mov	r0,r4
	mov	(r4)+,-(sp)		/ lv of funct
	jmp	*(r4)+

retrn:
	mov	r3,r0
	mov	(sp)+,r1		/ lv of funct
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,ent
	mov	(sp)+,lin
	mov	(sp)+,disp
	mov	(r0),sp
	tst	(r4)+		/ arg count
	mov	(r4)+,r0		/ return byte count
	inc	r0
	bic	$1,r0
	add	r0,r1
1:
	sub	$2,r0
	blt	1f
	mov	-(r1),-(sp)
	br	1b
1:
	jmp	*(r4)+

stsp:
	mov	sp,*(r4)+
	jmp	*(r4)+

lino:
	mov	(r4)+,lin
	jmp	*(r4)+

entry:
	mov	(r4)+,ent
	jmp	*(r4)+

trace:
	mov	(sp)+,r2
1:
	mov	ent,r1
	jsr	r5,pstr
	mov	$2,r0
	sys	write; m1; 1
	mov	lin,r1
	jsr	r5,pdec
	mov	$2,r0
	sys	write; m2; 1
	tst	disp
	beq	1f
	mov	disp,sp
	mov	(sp)+,ent
	mov	(sp)+,lin
	mov	(sp)+,disp
	br	1b
1:
	jmp	(r2)

pstr:
	tst	r1
	bne	1f
	mov	$m3,r1
1:
	movb	(r1)+,temp
	beq	1f
	mov	$2,r0
	sys	write; temp; 1
	br	1b
1:
	rts	r5

.data
m1:	< >
m2:	<\n>
m3:	<.main.\0>
.even
.bss
disp:	.=.+2
lin:	.=.+2
ent:	.=.+2
