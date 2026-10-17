/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)sigretro.h	5.1 (Berkeley) 6/6/85
 */

/*
 * Define extra stuff not found in signal.h
 */

#ifndef BADSIG
#define	BADSIG		(int (*)()) -1		/* Return value on error */
#endif BADSIG

#ifndef SIG_HOLD
#define	SIG_HOLD	(int (*)()) 3		/* Phony action to hold sig */
#endif SIG_HOLD

#ifndef sigmask
#define sigmask(s)	(1<<(s-1))		/* this exists in 4.3 */
#endif sigmask

