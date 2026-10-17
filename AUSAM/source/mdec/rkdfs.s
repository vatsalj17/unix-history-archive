	.globl	_restart,_main,savr5,_initial,_initiam,_reboot,_base

reset	= 5

RKWC	= 177406
RKDA	= 177412

.text
	jmp	start

	.=1000^.
start:	reset
	mov	$4,r0
intvc:	mov	r0,(r0)+	/ set up interrupt vectors to (.+2, 0)
	clr	(r0)+
	cmp	r0,$start
	blo	intvc

	mov	$start,sp
	mov	$buss,*$4	/ pick up bus errors
	jsr	pc,_initial
buss:	mov	$6,*$4		/ normal bus error exit
	jsr	pc,_initiam
	reset
	mov	$start,sp
	jsr	pc,_main
_restart:
	jmp	start

_reboot:
	reset
	mov	$RKWC,r0
	mov	$-256.,(r0)
	mov	_base,*$RKDA
	mov	$5,-(r0)
back:	tstb	(r0)
	bpl	back
	tst	(r0)
	bmi	_reboot
	clr	pc

.bss
savr5:	.=.+2
