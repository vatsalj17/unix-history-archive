	.globl getc,fopen,putc,flush
	mov	(sp)+,r4	/r4 is argc
	cmp	*(sp)+,$'s	/ invoked via speed command ??
	beq	0f		/ -> yes
	inc	fflag		/ form feed at end please !!
0:
	dec	r4
	ble	0f
	br	eof
0:	jsr	r5,getc;ibuf
	bes	eof
	cmp	r0,$177
	bpl	0b
	cmp	r0,$40
	bpl	2f
	bne	1f
	inc	r5
	br	0b
1:
	sub	$10,r0
	bmi	0b
	cmp	r0,$6
	bpl	0b
	asl	r0
	jsr	pc,*atab(r0)
	br	0b
2:
	tstb	line0(r5)
	bne	2f
	movb	r0,line0(r5)
	inc	r5
	cmp	r5,l0max
	bmi	0b
	mov	r5,l0max
	br	0b
2:
	tstb	line1(r5)
	bne	2f
	movb	r0,line1(r5)
	inc	r5
	cmp	r5,l1max
	bmi	0b
	mov	r5,l1max
	br	0b
2:
	tstb	line2(r5)
	bne	2f
	movb	r0,line2(r5)
	inc	r5
	cmp	r5,l2max
	bmi	0b
	mov	r5,l2max
	br	0b
2:
	jsr	pc,lflush
	movb	r0,line0(r5)
	inc	r5
	mov	r5,l0max
	mov	$15,r0
	jsr	r5,putc; obuf
	br	0b
eof:
	tst	r4
	ble	0f
	dec	r4
	mov	ibuf,r0
	sys	close
	mov	(sp)+,r0
	jsr	r5,fopen;ibuf
	bcc	0b
	mov	$2,r0
	sys	write;filerr;filend-filerr
	br	eof
0:
	tst	fflag		/ form feed wanted ??
	beq	0f
	mov	$14,r0
	jsr	r5,putc; obuf	/ output a form feed
	mov	$2,r0
	sys	write; finmsg ; finmsgl
0:
	jsr	pc,lflush
	jsr	r5,flush;obuf
	sys	exit
.data
atab:	bs; tab; nl; vt; ff; cr
.text
bs:
	dec	r5
	rts	pc
tab:
	add	$10,r5
	bic	$7,r5
	rts	pc
nl:
	jsr	pc,lflush
	clr	r5
	mov	$12,r0
	jsr	r5,putc; obuf
	rts	pc
vt:
ff:
	jsr	pc,lflush
	clr	r5
	mov	$14,r0
	jsr	r5,putc; obuf
	rts	pc
cr:
	clr	r5
	rts	pc

lflush:
	mov	l0max,r1
	bne	1f
	rts	pc
1:
	clr	r3
1:
	movb	line0(r3),r0
	bne	2f
	mov	$40,r0
2:
	clrb	line0(r3)
	jsr	r5,putc; obuf
	inc	r3
	cmp	r3,r1
	bmi	1b
	clr	l0max
	mov	$15,r0
	jsr	r5,putc; obuf
	mov	l1max,r1
	bne	1f
	rts	pc
1:
	clr	r3
1:
	movb	line1(r3),r0
	bne	2f
	mov	$40,r0
2:
	clrb	line1(r3)
	jsr	r5,putc; obuf
	inc	r3
	cmp	r3,r1
	bmi	1b
	clr	l1max
	mov	$15,r0
	jsr	r5,putc; obuf
	mov	l2max,r1
	bne	1f
	rts	pc
1:
	clr	r3
1:
	movb	line2(r3),r0
	bne	2f
	mov	$40,r0
2:
	clrb	line2(r3)
	jsr	r5,putc; obuf
	inc	r3
	cmp	r3,r1
	bmi	1b
	clr	l2max
	rts	pc
	.bss
ibuf:	.=.+518.
line0:	.=.+160.
line1:	.=.+160.
line2:	.=.+160.
l0max:	.=.+2
l1max:	.=.+2
l2max:	.=.+2
	.data
obuf:	1
	.=.+516.
fflag:	0
filerr:	<file not found\n>
filend:
finmsg: <FINISHED\n>
finmsgl = . - finmsg
