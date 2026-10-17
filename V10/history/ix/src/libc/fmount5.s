# C library -- fmount with explicit ceiling label

# error = fmount5(type, fd, file, flag, labp)

	.set	fmount,26
.globl	_fmount5
.globl  cerror

_fmount5:
	.word	0x0000
	bisl2	$128,4(ap)
	chmk	$fmount
	bcc 	noerror
	jmp 	cerror
noerror:
	clrl	r0
	ret
