/ C library -- acct(file): turn accounting on
/    acct(0): turn it off.

.globl	_acct, csv, cret, cerror
acct	= 51.

_acct:
	jsr	r5,csv
	mov	4(r5),0f
	sys	indir; 9f
	.data
9:
	sys	acct; 0:..
	.text
	bec	1f
	jmp	cerror
1:
	clr	r0
	jmp	cret
