/ double modf(x, *fp)
/ double x, *fp;
/ return fractional part
/ stash integer part (as double)

.globl	_modf
.globl	csv, cret
.globl	__fr0,__modf,__fr1
one	= 040200

_modf:
	jsr	r5,csv
	setd
	movf	4(r5),fr0
	movf	fr0,__fr0
	modf	$one,fr0
	movf	fr0,__modf
	movf	fr1,*12.(r5)
	movf	fr1,__fr1
	jmp	cret
	.data
__fr0:	0
	0
__modf:	0	
	0
__fr1:	0
	0
	.text
