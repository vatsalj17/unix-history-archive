/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(kbd_proc, @(#)kbd_proc.h	1.1.1.2	(5/12/87));

#define BEFOREID	0x01		/* character in buffer before the ID */
#define BELLTOKBD	0x40		/* Bell code to keyboard */
#define BITS210		0x07		/* Mask for lower 3 bits of proc_index */
#define CAP_LED		0x01		/* LEDs */
#define CLICKBIT	0x20		/* keyboard keyclick bit */
#define CLRCAP		0xfe		/* mask for clearing cap lock */
#define	CONTROL		0x01		/* Control key down */
#define CONTROLKEY	0x01		/* Index used for Control key */
#define CTRL1BIT	0x80		/* bit set to indicate control word 1 */
#define CURS		0xe0		/* mask for NOCURSEXPAND */
#define DOWN		1
#define ESC98		0xe2		/* downstroke code for 98 escape key */
#define ESC122		0x87		/* downstroke code for 122 escape key */
#define EXTENDED	0x0080		/* bit 7 of a char is the extended bit */
#define FKEY		0x80		/* NOPFEXPNAD mask */
#define KBD98		0x02		/* keyboard ID of  98 keyboard */
#define KBD122		0x11		/* keyboard ID of 122 keyboard */
#define KBD_ID		-1		/* flag for indicating found ID	*/ 
#define KEYSTROKE	0x00		/* keystroke identifier */
#define KEYDOWN		0x80		/* Test for extended bit*/
#define LOCALPROC	0x00		/* Index for a local process */
#define MASK		0x007f		/* clearing uper 9bits of an int */
#define MUXPROC		0x02		/* Index for a mux process */
#define NOKEY		0xff
#define NONMUXPROC	0x01		/* Index for a non-mux process */
#define NU		0x80		/* key or key combination Not Used */
#define OFF		0x00
#define ON		0x01
#define PAD		0xc0		/* mask for NOPADEXPAND */
#define PAD_ENTER	0x00		/* Enter key in the right pad cluster */
#define PFK1		0x00		/* Programmable function key #1 */
#define PFK2		0x01		/* Programmable function key #2 */
#define PFK3		0x02		/* Programmable function key #3 */
#define PFK4		0x03		/* Programmable function key #4 */
#define PFK5		0x04		/* Programmable function key #5 */
#define PFK6		0x05		/* Programmable function key #6 */
#define PFK7		0x06		/* Programmable function key #7 */
#define PFK8		0x07		/* Programmable function key #8 */
#define REFRESH_TIME	63		/* 1 sec between LED refresh cycles */
#define REQUESTID	0x88		/* Request ID */
#define RESET98		0xdb		/* downstroke code for 98 reset key */
#define RESET122	0xb8		/* downstroke code for 122 reset key */
#define SCR_LED		0x04
#define SCRBIT		0x08		/* NUM LOCK on */
#define SEQTIMEOUT	63		/* time to wait for second ID in ID sequence */
#define SFK9		0x08		/* Static function key #9 */
#define FIRST_SFK	SFK9
#define SFK10		0x09		/* Static function key #10 */
#define SFK11		0x0a		/* Static function key #11 */
#define SFK12		0x0b		/* Static function key #12 */
#define SFK13		0x0c		/* Static function key #13 */
#define SFK14		0x0d		/* Static function key #14 */
#define SFK15		0x0e		/* Static function key #15 */
#define SFK16		0x0f		/* Static function key #16 */
#define SFK17		0x10		/* Static function key #17 */
#define SFK18		0x11		/* Static function key #18 */
#define SFK19		0x12		/* Static function key #19 */
#define SFK20		0x13		/* Static function key #20 */
#define SFK21		0x14		/* Static function key #21 */
#define SFK22		0x15		/* Static function key #22 */
#define SFK23		0x16		/* Static function key #23 */
#define SFK24		0x17		/* Static function key #24 */
#define LAST_SFK	SFK24
#define SHIFT		0x02		/* Shift key down */
#define SHIFTKEY	0x00		/* Index used for Shift key */
#define SWITCH		-2		/* Switch char received */
#define SWITCHCHAR	0x02		/* char on queue before proc_index */
#define UNSHIFT		0x00		/* Neither control or shift depressed */
#define UP		0
#define WAITFORPT	31		/* Half second delay to repeat */
/****************************************************************************
NAME: traits.h
****************************************************************************/
#define NOCODE		(unsigned char) 0x00
#define SPECIAL		(unsigned char) 0x01
#define PADKEY		(unsigned char) 0x02
#define FLOWKEY		(unsigned char) 0x03
#define CURSKEY		(unsigned char) 0x04
#define ESCKEY		(unsigned char) 0x05
#define PFKEY		(unsigned char) 0x06
#define SFKEY		(unsigned char) 0x07
#define RETURNKEY	(unsigned char) 0x08
#define ENTERKEY	(unsigned char) 0x09
#define CAPSKEY		(unsigned char) 0x0a
#define ALPHA		(unsigned char) 0x0b
#undef REPEAT
#define REPEAT		(unsigned char) 0x80
/***************************************************************************
NAME:      asciidefs
FUNCTION:  equates for ASCII control codes
****************************************************************************/
#define  NUL     0x00
#define  SOH     0x01
#define  STX     0x02
#define  ETX     0x03
#define  EOT     0x04
#define  ENQ     0x05
#undef	 ACK
#define  ACK     0x06
#define  BEL     0x07
#define  BS      0x08
#define  HT      0x09
#define  LF      0x0a
#define  VT      0x0b
#define  FF      0x0c
#define  CR      0x0d
#define  SO      0x0e
#define  SI      0x0f
#define  DLE     0x10
#define  DC1     0x11
#define  DC2     0x12
#define  DC3     0x13
#define  DC4     0x14
#undef   NAK
#define  NAK     0x15
#define  SYN     0x16
#define  ETB     0x17
#define  CAN     0x18
#define  EM      0x19
#define  SUB     0x1a
#define  ESC     0x1b
#define  FS      0x1c
#define  GS      0x1d
#define  RS      0x1e
#define  US      0x1f
#define  SPACE   0x20
#define  PIPE    0x7c
#define  DEL     0x7f

struct key {
	char traits;
	char code[3];
};
