/ copy dec tape to rk05
/ (drive 0) --> (drive 0)

	jsr	pc,rew
2:
	jsr	pc,4(r5)
		<disk offset\n\0>
		.even
	jsr	pc,numb
	mov	r0,dska
	jsr	pc,4(r5)
		<tape offset\n\0>
		.even
	jsr	pc,numb
	mov	r0,tapa
	jsr	pc,4(r5)
		<count\n\0>
		.even
	jsr	pc,numb
	mov	r0,r2
1:
	jsr	pc,tread
	jsr	pc,wblk
	inc	tapa
	inc	dska
	dec	r2
	bne	1b
	jsr	pc,rew
	rts	pc

numb:
	clr	r1
1:
	jsr	pc,2(r5)
	cmp	r0,$'\n
	beq	1f
	sub	$'0,r0
	cmp	r0,$9
	bhi	2f
	mul	$10.,r1
	add	r0,r1
	br	1b
1:
	mov	r1,r0
	rts	pc
2:
	jsr	pc,4(r5)
		<illegal digit\n\0>
		.even
	tst	(sp)+
	rts	pc

ba:	buf
wc:	-256.
.bss
buf:	.=.+512.
dska:	.=.+2
tapa:	.=.+2
.text
/
/---------------------------------------------------------------

tcdt = 177350
tccm = 177342
tread:
	mov	$tcdt,r0
	mov	$tccm,r1
for:
	mov	$3,(r1)			/ rbn for
1:
	tstb	(r1)
	bge	1b
	tst	(r1)
	blt	rev
	cmp	tapa,(r0)
	beq	rd
	bgt	for

rev:
	mov	$4003,(r1)		/ rbn bac
1:
	tstb	(r1)
	bge	1b
	tst	(r1)
	blt	for
	mov	(r0),r2
	add	$5,r2
	cmp	tapa,r2
	blt	rev
	br	for

rd:
	mov	ba,-(r0)			/ bus addr
	mov	wc,-(r0)			/ wc
	mov	$5,-(r0)			/ read
1:
	tstb	(r1)
	bge	1b
	tst	(r1)
	blt	tread
	rts	pc

rew:
	mov	$4003,tccm
	rts	pc
/
/---------------------------------------------------------------

/ rk05 disk driver

rkda = 177412

/ write rk

wblk:
iocom = 3

	mov	dska,r1
	clr	r0
	div	$12.,r0
	ash	$4.,r0
	bis	r1,r0
	mov	$rkda,r1
	mov	r0,(r1)
	mov	ba,-(r1)
	mov	wc,-(r1)
	mov	$iocom,-(r1)
1:
	tstb	(r1)
	bpl	1b
	rts	pc
