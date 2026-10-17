/ Startup code for two-stage bootstrap

/ Modified for use with I space only CPUs
/ (11/23, 11/34, 11/40, & 11/60)
/ as well as I & D space CPUs
/ (11/44, 11/45, & 11/70).
/ Fred Canter 8/26/80

/ non-UNIX instructions
mfpi	= 6500^tst
stst	= 170300^tst
mtpi	= 6600^tst
mfpd	= 106500^tst
mtpd	= 106600^tst
spl	= 230
ldfps	= 170100^tst
stfps	= 170200^tst
wait	= 1
rtt	= 6
reset	= 5
/ trap	= 104400

PS	= 177776

.globl	_end
.globl	_main, __rtt
.globl	_edata
	jmp	start

/
/ trap vectors
/
	trap;340	/ bus error
	trap;341	/ illegal instruction
	trap;342	/ BPT
	trap;343	/ IOT
	trap;344	/ POWER FAIL
	trap;345	/ EMT
tvec:
	start;346	/ TRAP

tpc:	0	/ Save PS & PC from trap, used to start
tps:	0	/ program, and use for return to boot.

.=400^.
.text


start:
	mov	$340,*$PS
	mov	(sp)+,tpc
	mov	(sp)+,tps
	mov	$trap,tvec

/ Clear bss area of program.

	mov	$157776,sp
	mov	$_edata,r0
	mov	$_end,r1
	sub	r0,r1
	inc	r1
	clc
	ror	r1
1:
	clr	(r0)+
	sob	r1,1b
	jsr	pc,_main

/ fix up stack to point at trap ps-pc pair
/ so we can return to the bootstrap
__rtt:
	mov	$157776,sp
	mov	tps,-(sp)
	mov	tpc,-(sp)
	rtt				/ we hope!
	br	.


.globl	_trap
trap:
	mov	r0,-(sp)
	mov	r1,-(sp)
	mov	*$PS,-(sp)
	jsr	pc,_trap
	tst	(sp)+
	mov	(sp)+,r1
	mov	(sp)+,r0
	rtt
