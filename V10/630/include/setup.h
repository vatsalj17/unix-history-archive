/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(setup, @(#)setup.h	1.1.1.3	(5/12/87));

#ifndef	SETUP_H
#define	SETUP_H	1

/*
 * pf key sizes
*/
#define NUMPFKEYS	8
#define PFKEYSIZE	80

/* possible values for the first argument to setupval() */
/* are the return value of whathost() and these two below */
#define S_PREF	0x100
#define S_PRINT 0x101

/* preference (display, keyboard) options */
/* The first number is the second argument to setupval() */
/* The numbers below it are possible return values */

#define S_PREF_CTRL	1
#define S_PREF_CTRL_VIS		0
#define S_PREF_CTRL_INVIS	1
#define S_PREF_CTRL_SPACE	2

#define S_PREF_KCLK	2
#define S_PREF_KCLK_OFF		0
#define S_PREF_KCLK_ON		1

#define S_PREF_KVOL	3
#define S_PREF_KVOL_0		0
#define S_PREF_KVOL_1		1
#define S_PREF_KVOL_2		2
#define S_PREF_KVOL_3		3
#define S_PREF_KVOL_4		4
#define S_PREF_KVOL_5		5
#define S_PREF_KVOL_6		6
#define S_PREF_KVOL_7		7

#define S_PREF_CURS	4
#define S_PREF_CURS_NOBLK	0
#define S_PREF_CURS_BLK		1

#define S_PREF_KRPT	5
#define S_PREF_KRPT_15		0
#define S_PREF_KRPT_20		1
#define S_PREF_KRPT_30		2
#define S_PREF_KRPT_60		3

#define S_PREF_WBUF	8
#define S_PREF_WBUF_OFF		0
#define S_PREF_WBUF_ON		1

#define S_PREF_WTYP	9
#define S_PREF_WTYP_BASIC	0
#define S_PREF_WTYP_ENH		1

#define S_PRINT_TAB	0
#define S_PRINT_TAB_NO		0
#define S_PRINT_TAB_YES		1

#define S_PRINT_ESC	1
#define S_PRINT_ESC_NO		0
#define S_PRINT_ESC_YES		1

/* host options */
/* These can only be used when the return value of whathost() */
/* is the first argument to setupval() */

#define S_HOST_ENC	6
#define S_HOST_ENC_OFF		0
#define S_HOST_ENC_ON		1

#define S_HOST_RTN	9
#define S_HOST_RTN_CR		0
#define S_HOST_RTN_LF		1
#define S_HOST_RTN_CRLF		2

#define S_HOST_NL	10
#define S_HOST_NL_LF		0
#define S_HOST_NL_CRLF		1

#define S_HOST_FONT	11
#define S_HOST_FONT_SMALL	0
#define S_HOST_FONT_MEDIUM	1
#define S_HOST_FONT_LARGE	2

/* The return value for these is the setting */
#define S_HOST_COL	12
#define S_HOST_ROW	14
#define S_HOST_NCOL	16
#define S_HOST_NROW	18

#define S_HOST_FIXED	20
#define S_HOST_FIXED_NO		0
#define S_HOST_FIXED_YES	1


#endif SETUP_H
