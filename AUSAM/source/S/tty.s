/ tty -- get tty number

.globl	_ttyn

	mov	$2,-(sp)
	jsr	pc,_ttyn
	movb	r0,nam
	mov	$1,r0
	sys	write; name; 5
	sys	exit

.data
name:
	<tty>
nam:
	<x\n>
.even
