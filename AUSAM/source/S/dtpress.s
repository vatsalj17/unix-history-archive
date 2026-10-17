/
/ designed exclusively for compressing and expanding
/ outputs from 'dtp' with the verbose flag on.
/
/ Syntax:	dtpress [file | -] [file]
/
	.globl	getc,putc,flush
/
	getp = r3
	putp = r4
/
	mov	(sp)+,r1
	cmp	$3,r1
	jlo	uerr
	tst	(sp)+
	cmp	$1,r1
	beq	1f
	mov	(sp)+,r2		/ name of standard input
	cmpb	(r2),$'-
	bne	2f
	tstb	1(r2)
	beq	3f
2:
	mov	r2,2f+2
	sys	indir;2f
	.data
2:	sys	open; 0; 0
	.text
	jes	oerr
	mov	r0,ib
3:
	cmp	$3,r1
	bne	1f
	mov	(sp)+,2f+2
	sys	indir;2f
	.data
2:	sys	creat; 0; 604
	.text
	jes	cerr
	mov	r0,ob
	br	0f
1:
	mov	$1,ob
0:
	mov	$getc,getp
	mov	$putc,putp
	mov	$line,r1
	clr	r2			/ line repetition counter
	jsr	r5,*getp; ib		/ look for leading null
	jes	eof
	tst	r0
	jeq	expand
	jbr	4f
0:
	mov	$line,r1
	clr	r2
1:
	jsr	r5,*getp; ib
	jes	eof
	cmpb	r0,(r1)+
	bne	1f
	inc	r2
	br	1b
1:
	dec	r1
4:
	mov	r0,-(sp)
	mov	r2,r0
	jsr	r5,*putp; ob		/ output similarity count
	mov	(sp)+,r0
1:
	cmp	r0,$'\n
	beq	1f
	movb	r0,(r1)+
	jsr	r5,*putp; ob
	jsr	r5,*getp; ib
	bec	1b
	jbr	eof1
1:
	jsr	r5,*putp; ob
	jbr	0b
eof:
	mov	r2,r0
	jsr	r5,*putp; ob
eof1:
	jsr	r5,flush; ob
	clr	r0
	sys	exit
uerr:
	mov	$usage,r0
	mov	$usl,r1
	jbr	error
	.data
usage:	<Usage: dtpress [file | -] [file]\n\0>
usl = . - usage
	.text
oerr:
	mov	$openf,r0
	mov	$opl,r1
	jbr	error
	.data
openf:	<Cannot open input\n\0>
opl = . - openf
	.text
cerr:
	mov	$crate,r0
	mov	$crl,r1
	jbr	error
	.data
crate:	<Cannot create output\n\0>
crl = . - crate
	.text
error:
	mov	r0,2f+2
	mov	r1,2f+4
	mov	$2,r0
	sys	indir;2f
	.data
2:	sys	write; 0; 0
	.text
	mov	$1,r0
	sys	exit
/
/ code for expansion
/ called when first char is a 'null'
/
expand:
	tst	r2
	beq	1f
0:
	movb	(r1)+,r0
	jsr	r5,*putp; ob
	sob	r2,0b
1:
	jsr	r5,*getp; ib
	jes	eof1
	cmpb	r0,$'\n
	beq	2f
	movb	r0,(r1)+
	jsr	r5,*putp; ob
	jbr	1b
2:
	jsr	r5,*putp; ob
	jsr	r5,*getp; ib
	jes	eof1
	mov	$line,r1
	mov	r0,r2
	bne	0b
	jbr	1b
	.bss
ib:	.=.+518.
ob:	.=.+518.
line:	.=.+256.
