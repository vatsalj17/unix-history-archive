
/ C register save and restore -- version 7/75
/ modified by wnj && cbh 6/79 for overlaid text registers
/ modified by wf jolitz 2/80 to work and use emt syscall
/ modified for 8-byte thunk, mjk 10/81
/

.globl	csv
.globl	cret
.globl  __ovno
.globl  _etext
.data
__ovno:	0
.text

emt= 0104000		/ overlays switched by emulator trap. ovno in r0.

/ ovhndlr1 through ovhndlr7 are called from the thunks after
/ r1 is set to the location of the first instruction in the subroutine
/ after the call to csv (~foo+4).  They in turn branch to ovhndlr
/ which sets the overlay and simulates a csv.
/ Thus, the subroutine's call to csv is by-passed.

.globl	ovhndlr1, ovhndlr2, ovhndlr3, ovhndlr4, ovhndlr5, ovhndlr6, ovhndlr7

ovhndlr1:
	mov	$1,r0
	br	ovhndlr

ovhndlr2:
	mov	$2,r0
	br	ovhndlr

ovhndlr3:
	mov	$3,r0
	br	ovhndlr

ovhndlr4:
	mov	$4,r0
	br	ovhndlr

ovhndlr5:
	mov	$5,r0
	br	ovhndlr

ovhndlr6:
	mov	$6,r0
	br	ovhndlr

ovhndlr7:
	mov	$7,r0
	br	ovhndlr

ovhndlr:

.bss
.globl	_ovswtab
_ovswtab:			/ table to collect overlay switch statistics
.=.+400				/ long ovswtab[1 + NOVL][1 + NOVL];
.text

/	_ovswtab[__ovno][r0]++;
	mov	r1, -(sp)
	mov	__ovno, r1	/ ((__ovno
	ash	$3, r1		/ * (1 + NOVL))
	add	r0, r1		/ + r0
	inc	r1		/ + 1)
	ash	$2, r1		/ * sizeof (long)
	add	$_ovswtab, r1
	add	$1, -(r1)
	adc	-(r1)
	mov	(sp)+, r1

	cmp	r0,__ovno
	beq	1f
	emt
1:	mov	sp,r5
	mov	__ovno,-(sp)
	mov	r0,__ovno
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r1)		/ jsr part is sub $2,sp

/ csv for routines called directly (in base or intra-overlay calls).
/ no overlays have been changed, so we just save the previous overlay
/ number on the stack. note that r0 isn't set to the current overlay
/ because we weren't called through a thunk.
csv:
	mov	r5,r1
	mov	sp,r5
	mov	__ovno,-(sp)	/ overlay is extra (first) word in mark
/ rest is old code common with csv
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r1)		/ jsr part is sub $2,sp
/
/ at this point, the stack frame looks like this:
/
/	_________________________
/	|  return addr to callee|
/	|_______________________|
/ r5->	| old r5	        |
/	|_______________________|
/	| previous ovnumber     |
/	|_______________________|
/	| old r4		|
/	|_______________________|
/	| old r3		|
/	|_______________________|
/ sp->	| old r2		|
/	|_______________________|
/


cret:
	mov	r5,r2
/ get the overlay out of the mark, and if it is non-zero
/ make sure it is the currently loaded one
	mov	-(r2),r4
	bne	1f		/ zero is easy
2:
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
/ not returning to root segment, so check that the right
/ overlay is loaded, and if not ask UNIX for help
1:
	cmp	r4,__ovno
	beq	2b		/ lucked out!
/ if return address is in root segment, then nothing to do
	cmp	2(r5),$_etext
	blos	2b
/ returning to wrong overlay --- do something!
	mov	r0,r3
	mov	r4,r0
	emt
	mov	r4,__ovno
	mov	r3,r0
/ intr. routines may run between these, so should force segment __ovno
	br	2b
