/
/	random number generator for machines with FP11 flt. pt.
/
/	usage:
/		long srand(seed)/* set initial seed */
/		long seed;	/* return old seed */
/
/		int rand()	/* return a pseudo random number */
/				/* in the range 0 thru 32767 */
/
/		long rand()	/* return a pseudo random number */
/				/* in the range 0 thru 2147483647 */
/
/		double frand()	/* return a pseudo random number */
/				/* in the range 0.0 thru 1.0 */
/
/

	.globl	_srand,_frand,_lrand,_rand
.text

_srand:
	mov	seed+2,r1
	mov	seed,r0
	mov	2(sp),seed
	mov	4(sp),seed+2
	rts	pc

_frand:
	setl
	movif	seed,fr2
	mulf	randa,fr2
	addf	randc,fr2
	modf	randim,fr2	/ fr3 integral and fr2 fractional
	movf	fr2,fr0
	mulf	randm,fr2
	movfi	fr2,seed
	rts	pc

_lrand:
	jsr	pc,_frand	/ get random number in range 0.0 to 1.0
	mulf	maxlong,fr0
	movfi	fr0,-(sp)
	mov	(sp)+,r0
	mov	(sp)+,r1
	rts	pc

_rand:
	jsr	pc,_frand	/ get random number in range 0.0 to 1.0
	mulf	maxint,fr0
	seti
	movfi	fr0,-(sp)
	mov	(sp)+,r0
	rts	pc

/
/	Initialization of random number "constants"
/
.data

seed:
	166; 121043			/ long seed	7774755l
randa:
	44164; 106400; 0; 0		/ double randc	113218009.
randc:
	46727; 171073; 20000; 0		/ double randa	62605.
randm:
	47400; 0; 0; 0			/ double randm	536870912.
randim:
	31000; 0; 0; 0			/ double randim 1./randm
maxlong:
	47777; 177777; 177000; 0	/ largest long 2147483647.
maxint:
	43777; 177000; 0; 0		/ largest int 32767.
