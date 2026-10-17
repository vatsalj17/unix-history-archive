ldfps = 170100^tst
rtt = 6
/
/ oldsigill = ldfps(number);
/
/  This is a hacked version of ldfps that is designed to work
/  in all three of the following situations:
/	1.  PDP's with hardware floating point.
/	2.  PDP's without an FPU using the floating point simulator.
/	3.  PDP's without an FPU without the floating point simulator
/	    (in programs not otherwise using floating point).
/  It returns the previous signal value for SIGILL, since it isn't
/  otherwise safe to test that with signal() when using the floating point
/  interpreter.  This is used in main() to see whether it should catch SIGILL.

.globl	_ldfps
_ldfps:
	mov	r5,-(sp)
	mov	sp,r5
	sys	signal; 4; 3f		/ see if SIGILL is being caught
	mov	r0, 0f
	beq	1f			/ if not, use our catcher
	sys	indir; 8f		/ if so, reset the old catch value now
1:
	ldfps	4(r5)
2:
	sys	indir; 8f		/ restore the old signal (if we haven't)
	mov	0f,r0			/ return old signal value
	mov	(sp)+,r5
	rts	pc

3:					/ trap here if no fp hardware
	mov	$2b,(sp)		/ adjust return address past ldfps
	rtt

	.data
8:
	sys	signal; 4; 0: 0
