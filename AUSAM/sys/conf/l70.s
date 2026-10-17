	.profile = 0		/ for profiling
	.data

/ low core

	.globl	dump, start, pirqi
	.globl	code00, code01, code02, code03, code04, code05
	.globl	code06, call, quickcall, powfail, powvect
br4 = 200
br5 = 240
br6 = 300
br7 = 340
rset1 = 4000

. = 0^.
	br	1f		/ Replaced later with 'code15'
	br7+15			/ Spurious interrupt through 0

/ trap vectors
	code05; br7+7		/ bus error
	code02; br7+2		/ illegal instruction
	code02; br7+3		/ bpt/trace trap
	code01; br7+5		/ iot trap
powvect:
	powfail; br7		/ power fail
	code04; br7+6		/ emt
	code02; br7+4		/ system entry

. = 40^.
1:	jmp	*$start
	jmp	*$dump

. = 50^.
	30000		/for the benefit of ei.c pseudo switch register

. = 52^.
	0		/ rotating pattern for display by swr = 52

. = 54^.
	0;0		/ clock ticks since system start !!
			/ see clock service routine below
			/ see display in m70.s

. = 60^.
	klin; br4+0.		/ KL #0 input
	klou; br4+0.		/ KL #0 output

. = 70^.
	0			/ this space used for swtch display

. = 100^.
	kwlp; br6+1		/ Clock ** NOTE +1 ==> c-bit set  ** SEE kwlp
.if	.profile-1
	kwlp; br6+1		/ Clock
.endif
.if .profile
	.globl	profile
	profile; br7+rset1		/ programmable clock for the profiling
.endif

. = 114^.
	code03; br7+12		/ 11/70 parity

. = 170^.
	.globl	_idle,diswflag
_idle:
	inc	diswflag
	1	/* wait *
	rts	pc

. = 200^.
	lpou; br4+0.		/ Line printer LP05

. = 224^.
	htio; br5		/ HT tape

. = 240^.
	pirqi;  br7		/ programmed interrupt request
	code00; br7+0		/ floating point interrupt
	code06; br7+10		/ segmentation violation

. = 254^.
	hpio; br5		/ HP disk

/ floating vectors
. = 300^.
. = 310^.
. = 320^.
	djin; br5+0.
	djou; br5+0.
. = 330^.
	djin; br5+1.
	djou; br5+1.
. = 340^.
	dzin; br5+0.
	dzou; br5+0.
. = 350^.
. = 360^.
	lpou; br4+1.		/ Line printer La180

. = 370^.
	dzin; br5+1.
	dzou; br5+1.

//////////////////////////////////////////////////////
/		interface code to C
//////////////////////////////////////////////////////

.globl	call

.globl	_klrint
klin:	jsr	r0,call; _klrint
.globl	_klxint
klou:	jsr	r0,call; _klxint

.globl	_clock
kwlp:
	adc	*$56		/  low word count of clock ticks
	adc	*$54		/ high word count of clock ticks
	jsr	r0,call; _clock


.globl	_lpint
lpou:	jsr	r0,call; _lpint

.globl	_htintr
htio:	jsr	r0,call; _htintr

.globl	_hpintr
hpio:	jsr	r0,call; _hpintr


.globl	_djrint
djin:	jsr	r0,call; _djrint
.globl	_djxint
djou:	jsr	r0,call; _djxint
.globl	_dzrint
dzin:	jsr	r0,call; _dzrint
.globl	_dzxint
dzou:	jsr	r0,call; _dzxint
