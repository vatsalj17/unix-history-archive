/*
 * unctrl.h
 *
 * 2/17/82 (Berkeley) @(#)unctrl.h	1.2
 */

extern	char	*_unctrl[];	/* defined in /usr/src/lib/curses/unctrl.c */

#define	unctrl(ch)	(_unctrl[ch & 0177])
