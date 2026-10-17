/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(keycodes, @(#)keycodes.h	1.1.1.2	(5/12/87));

#ifndef	KEYCODE_H
#define	KEYCODE_H	1

/*
 * The following four #defines are masks used to distinguish 
 * between function keys, pad keys and cursor keys in the NO 
 * EXPAND mode.  For example,
 *
 *	c = (unsigned char)kbdchar();
 *	if ((c & BITS765) == F_KEY)
 *		function_key();
 *	else if ((c & BITS765) == PAD_KEY)
 *		padk_key();
 */
#define BITS765         (unsigned char)0xE0
#define F_KEY           (unsigned char)0x80
#define PAD_KEY         (unsigned char)0xC0
#define CURS_KEY        (unsigned char)0xE0

/* Values returned for function keys in NOPFEXPAND mode */
#define FUNC1KEY	(unsigned char)0x80	/* f1 */
#define FUNC2KEY	(unsigned char)0x81	/* f2 */
#define FUNC3KEY	(unsigned char)0x82	/* f3 */
#define FUNC4KEY	(unsigned char)0x83	/* f4 */
#define FUNC5KEY	(unsigned char)0x84	/* f5 */
#define FUNC6KEY	(unsigned char)0x85	/* f6 */
#define FUNC7KEY	(unsigned char)0x86	/* f7 */
#define FUNC8KEY	(unsigned char)0x87	/* f8 */
#define FUNC9KEY        (unsigned char)0x88	/* f9 */
#define FUNC10KEY       (unsigned char)0x89	/* f10 */
#define FUNC11KEY       (unsigned char)0x8a	/* f11 */
#define FUNC12KEY       (unsigned char)0x8b	/* f12 */
#define FUNC13KEY       (unsigned char)0x8c	/* f13 */
#define FUNC14KEY       (unsigned char)0x8d	/* f14 */

/* Values returned for pad keys in NOPADEXPAND mode */
#define PAD_ENTER	(unsigned char)0xC0
#define PAD_EQUALS	(unsigned char)0xC1
#define PAD_ASTERISK	(unsigned char)0xC2
#define PAD_SLASH	(unsigned char)0xC3
#define PAD_PLUS	(unsigned char)0xC4
#define PAD_7		(unsigned char)0xC5
#define PAD_8		(unsigned char)0xC6
#define PAD_9		(unsigned char)0xC7
#define PAD_MINUS	(unsigned char)0xC8
#define PAD_4		(unsigned char)0xC9
#define PAD_5		(unsigned char)0xCA
#define PAD_6		(unsigned char)0xCB
#define PAD_COMA	(unsigned char)0xCC
#define PAD_1		(unsigned char)0xCD
#define PAD_2		(unsigned char)0xCE
#define PAD_3		(unsigned char)0xCF
#define PAD_0		(unsigned char)0xD0
#define PAD_DOT		(unsigned char)0xD1

/* Values returned for cursor keys in NOCURSEXPAND mode */
#define UP_ARROW        (unsigned char)0xE0
#define DOWN_ARROW      (unsigned char)0xE1
#define RIGHT_ARROW     (unsigned char)0xE2
#define LEFT_ARROW      (unsigned char)0xE3
#define HOME_KEY        (unsigned char)0xE4

/* Special constants used in the NOTRANSATE mode */
#define DOWNSTROKE      (unsigned char)0x80
#define KEYSTROKE       (unsigned char)0x00
#define BEFOREID        (unsigned char)0x01
#define SWITCHCHAR      (unsigned char)0x02
#define KEY98ID         (unsigned char)0x01

#endif
