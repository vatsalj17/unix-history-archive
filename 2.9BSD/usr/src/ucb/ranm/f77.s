f4p = 0
f77 = 1
.globl	_ranm_
.globl _ransav_
.globl _ranres_
.globl	csv, cret
.globl	_iran_
_iran_:
	mov	sp,r0
	mov	12.(r0),r1; mov (r1),-(sp)
	mov	10.(r0),r1; mov (r1),-(sp)
	mov	8.(r0),r1; mov (r1),-(sp)
	mov	6(r0),r1; mov (r1),-(sp)
	mov	4(r0),r1; mov (r1),-(sp)
	mov	2(r0),r1; mov (r1),-(sp)
	jsr	pc,_iran
	add	$12.,sp
	rts	pc
.globl	_randomi,_rndmze_
