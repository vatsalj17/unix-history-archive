/ C library -- pp

/ pp(&lock);
/
/ request lock

.globl	_pp

ppp = 53.
_pp:				   / try for resource.
	mov	2(sp),r0
	sys	ppp
	bec	1f		/ if ok
	clr	r0		/ error
1:
     	rts     pc
