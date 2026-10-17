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

. = 52^.
	0		/ rotating pattern for display by swr = 52

. = 54^.
	0;0		/ clock ticks since system start !!
			/ see clock service routine below
			/ see display in m70.s

. = 60^.
	klin; br4+0.		/ KL #0 input
	klou; br4+0.		/ KL #0 output

. = 100^.
	kwlp; br6+1		/ Clock ** NOTE +1 ==> c-bit set  ** SEE kwlp

. = 114^.
	code03; br7+12		/ 11/70 parity

. = 170^.
	.globl	_idle,diswflag
_idle:
	inc	diswflag
	1	/* wait *
	rts	pc

. = 200^.
	lpou; br4+0.		/ Line printer

. = 224^.
	htio; br5		/ HT tape	George - where does yours live?

. = 240^.
	pirqi;  br7		/ programmed interrupt request
	code00; br7+0		/ floating point interrupt
	code06; br7+10		/ segmentation violation

. = 254^.
	msio; br5		/ MSC controller	Again - where is yours?

.if 	.profile
. = 260^.
	.globl profile
	profile; br7+rset1		/ programmable clock for the profiling
.endif

/ floating vectors
. = 300^.

//////////////////////////////////////////////////////
/		interface code to C
//////////////////////////////////////////////////////


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

.globl	_tmintr
tmio:	jsr	r0,call; _tmintr

.globl	_hpintr
hpio:	jsr	r0,call; _hpintr
