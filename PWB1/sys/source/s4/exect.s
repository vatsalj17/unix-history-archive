/ C library -- exect

/ exect(file, argv);
/
/ where argv is a vector argv[0] ... argv[x], 0
/ last vector element must be 0
/
/ The same as execv except that it sets the TBIT causing
/ a trace trap on the first instruction of the executed instruction
/ to give a chance to set breakpoints.

.globl	_exect, cerror
rtt	= 6

_exect:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),0f+2
	mov	$170000,-(sp)	/in case it fails
	mov	$cerror,-(sp)
	mov	$170020,-(sp)	/ t-bit
	mov	$1f,-(sp)
	rtt
1:
	sys	0; 9f
	rtt
.data
9:
	sys	exec; 0:..; ..
