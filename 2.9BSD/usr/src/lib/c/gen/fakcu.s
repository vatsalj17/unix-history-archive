/*	@(#)fakcu.s	2.1	SCCS id keyword	*/
/
/ dummy cleanup routine if none supplied by user.

.globl	__cleanup

__cleanup:
	rts	pc
