/ low core
	.globl trap,bustrap

br4 = 200
br5 = 240
br6 = 300
br7 = 340
wait = 1

. = 0^.
	br	1f
		br7+15

/ trap vectors
	bustrap; br7+0.		/ bus error
	trap; br7+1.		/ illegal instruction
	trap; br7+2.		/ bpt-trace trap
	trap; br7+3.		/ iot trap
	trap; br7+4.		/ power fail
	trap; br7+5.		/ emulator trap
	trap; br7+6.		/ system entry

. = 40^.
.globl	start, dump
1:	jmp	start
	jmp	dump

. = 54^.
	0;0			/count of clock tiks

. = 60^.
	klin; br4
	klou; br4

. = 100^.
	kwlp; br6+1			/set c bit
	kwlp; br6+1			/set c bit

. = 114^.
	trap; br7+7.		/ 11/70 parity

. = 174^.
	.globl	_idle
_idle:
	wait
	rts	pc

. = 200^.
	lpou; br4+0.		/ Line printer

. = 224^.
	htio; br5		/ HT tape

. = 230^.
	crdin; br6


. = 244^.
	trap; br7+8.
	trap; br7+9.
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

.globl	_crdint
crdin:	jsr	r0,call; _crdint

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
