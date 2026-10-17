	/edrecon
	/
	/ converts a file in editor format back
	/ to normal( you hope). files which have been
	/ grossly manipulated will be perhaps esoteric
	/ one argument is required. the file to be translated
	/ output is on standard output.
	/
	/
	.globl	fopen,getc,putc,flush
	.bss
obuf:	.=.+518.
ibuf:	.=.+518.
	.text
	mov	$1,obuf
	cmp	$2,(sp)+
	beq	0f
	mov	$2,r0
	sys	write;m1;m1l
	sys	exit
	.data
m1:	<Give one filename only\n>
m1l=.-m1
	.text
0:	mov	2(sp),r0
	jsr	r5,fopen; ibuf
	bec	0f
	mov	$2,r0
	sys	write;m2;m2l
	sys	exit
	.data
m2:	<cannot open file\n>
m2l = .-m2
	.text
0:	jsr	r5,getc;ibuf
	bes	9f
	inc	r3
	tst	r0
	bne	4f
1:	bit	$7,r3
	beq	1f
	jsr	r5,getc;ibuf
	inc	r3
	br	1b
1:	mov	$12,r0
4:	jsr	r5,putc;obuf
	br	0b
9:	jsr	r5,flush;obuf
	sys	exit
	/there is also a problem if the file has some text
	/in core
