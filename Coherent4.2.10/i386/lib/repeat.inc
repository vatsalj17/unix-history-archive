/ This file encapsulates a tentative change made to deal with potential
/ motherboard problems with use of the REP instruction prefix in conjunction
/ with the x86 I/O instructions; due to PC bus timing problems, some
/ peripheral boards may be unable to supply data within the required bus
/ times. It may be that using a LOOP instruction rather than a REP prefix
/ may help with this.
/
/ In the absence of real diagnostic tools to analyse this problem, here we
/ provide a macro which expands to either form so we can build kernels for
/ experimental purposes.
/	Use "-D_USE_REP_PREFIX=1" to compile for the REP mode,
/	and nothing to compile for the LOOP mode.

repeat	.macro	instr
	.if	.defined _USE_REP_PREFIX
		rep
		instr
	.else
repeat_\.macno:
		instr
		loop	repeat_\.macno
	.endi
	.endm

