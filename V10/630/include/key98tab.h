/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(key98tab, @(#)key98tab.h	1.1.1.2	(5/12/87));

/*******************************************************************************

MODULE NAME:    key98tab()

FUNCTION:	Defines the traits and raw code translation for each key on the
		98 key keyboard.

DESCRIPTION:    The lines in this table are set up in the following way: 
		key.traits, key.code[] = { unshift, control, shift, }
		The key.code given for unshift, control and shift may be
		indexes into other smaller keyboard tables e.g., function
		keys, cursor control keys, and pad keys.

*******************************************************************************/
struct key key98tab[] = {
REPEAT | PADKEY, 0x00, 0x00, 0xc0,		/* ENTER (pad) 	position 114	code 0x00 */
ALPHA | REPEAT, 'z', SUB, 'Z',			/* Z		position 85	code 0x01 */
REPEAT, '7', NU, '&',				/* 7 &		position 32	code 0x02 */
PFKEY, 0x06, 0x06, 0x06,			/* F7		position 13 	code 0x03 */
ENTERKEY | REPEAT, 0x00, 0x00, 0x00,		/* ENTER	position 96	code 0x04 */
PFKEY, 0x02, 0x02, 0x02,			/* F3		position 5	code 0x05 */
SFKEY, 0x08, 0x08, 0x08,			/* F9		position 15	code 0x06 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x07 */
REPEAT, '8', NU, '*',				/* 8 *		position 33	code 0x08 */
ALPHA | REPEAT, 'd', EOT, 'D',			/* D		position 68	code 0x09 */
REPEAT, ';', NU, ':',				/* ; :		position 75	code 0x0a */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x0b */
ALPHA | REPEAT, 'i', HT, 'I',			/* I		position 53	code 0x0c */
SFKEY, 0x09, 0x09, 0x09,			/* F10		position 16	code 0x0d */
FLOWKEY, 0x00, 0x03, NU,			/* BREAK DISCON	position 40	code 0x0e */
ALPHA | REPEAT, 'e', ENQ, 'E',			/* E		position 48	code 0x0f */
/**********************************************************************************/
REPEAT, '[', ESC, '{',				/* [ {		position 56	code 0x10 */
REPEAT | PADKEY, '*', ')', 0xc2,		/* * ) (pad)	position 43	code 0x11 */
REPEAT | PADKEY, '8', '8', 0xc6,		/* 8 (pad)	position 61	code 0x12 */
CURSKEY | REPEAT, 0x03, 0x03, 0x03,		/* left arrow	position 97	code 0x13 */
ALPHA | REPEAT, 'j', LF, 'J',			/* J		position 72	code 0x14 */
REPEAT, '4', NU, '$',				/* 4 $		position 29	code 0x15 */
REPEAT, 0x27, NU, '"',				/* ' "		position 76	code 0x16 */
CURSKEY | REPEAT, 0x00, 0x00, 0x00,		/* up arrow	position 79	code 0x17 */
REPEAT, DEL, DEL, DEL,				/* Dlete key	position 39	code 0x18 */
ALPHA | REPEAT, 'l', FF, 'L',			/* L		position 74	code 0x19 */
ALPHA | REPEAT, 'u', NAK, 'U', 			/* U		position 52	code 0x1a */
REPEAT | PADKEY, '4', '4', 0xc9,		/* 4 (pad)	position 80	code 0x1b */
ALPHA | REPEAT, 'w', ETB, 'W',			/* W		position 47	code 0x1c */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x1d */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x1e */
ALPHA | REPEAT, 'o', SI, 'O',			/* O		position 54	code 0x1f */
/**********************************************************************************/
PFKEY, 0x00, 0x00, 0x00,			/* F1		position 3	code 0x20 */
ALPHA | REPEAT, 'n', SO, 'N',			/* N		position 90	code 0x21 */
REPEAT, '3', NU, '#',				/* 3 #		position 28	code 0x22 */
REPEAT | PADKEY, '6', '6', 0xcb,		/* 6 (pad)	position 82	code 0x23 */
REPEAT, '9', NU, '(',				/* 9 (		position 34	code 0x24 */
ALPHA | REPEAT, 'p', DLE, 'P',			/* P		position 55	code 0x25 */
PFKEY, 0x01, 0x01, 0x01,			/* F2		position 4	code 0x26 */
FLOWKEY, 0x01, 0x01, 0x01,			/* SCROLL LOCK	position 83	code 0x27 */
ALPHA | REPEAT, 'm', CR, 'M',			/* M		position 91	code 0x28 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x29 */
REPEAT, '0', NU, ')',				/* 0 )		position 35	code 0x2a */
REPEAT, '=', NU, '+',				/* = +		position 37	code 0x2b */
PFKEY, 0x07, 0x07, 0x07,			/* F8		position 14	code 0x2c  */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x2d */
REPEAT | PADKEY, '.', '.', 0xd1,		/* . (pad)	position 109	code 0x2e */
REPEAT | PADKEY, '9', '9', 0xc7,		/* 9 (pad)	position 62	code 0x2f */
/**********************************************************************************/
REPEAT, '-', US, '_',				/* - _		position 36	code 0x30 */
REPEAT | CURSKEY, 0x04, 0x04, 0x04,		/* HOME		position 98	code 0x31 */
REPEAT | PADKEY, '7', '7', 0xc5,		/* 7 (pad)	position 60	code 0x32 */
ALPHA | REPEAT, 'b', STX, 'B',			/* B		position 89	code 0x33 */
SPECIAL, 0x00, 0x00, 0x00,			/* SHIFT	position 95	code 0x34 */
REPEAT | PADKEY, ',', ',', 0xcc,		/* , (pad)	position 113	code 0x35 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x36 */
ALPHA | REPEAT, 'k', VT, 'K',			/* K            position 73	code 0x37 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x38 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x39 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x3a */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x3b */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x3c */
REPEAT, '\\', FS, '|',				/* \\ |         position 78	code 0x3d */
PFKEY, 0x03, 0x03, 0x03,			/* F4           position 8 	code 0x3e */
ALPHA | REPEAT, 'h', BS, 'H',			/* H            position 71	code 0x3f */
/**********************************************************************************/
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x40 */
SPECIAL, 0x00, 0x00, 0x00,			/* SHIFT        position 84	code 0x41 */
PFKEY, 0x05, 0x05, 0x05, 			/* F6           position 12	code 0x42 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x43 */
SPECIAL, 0x01, 0x01, 0x01,			/* CTRL         position 65	code 0x44 */
ALPHA | REPEAT, 'v', SYN, 'V',			/* V            position 88	code 0x45 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x46 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x47 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x48 */
REPEAT | PADKEY, '=', '(', 0xc1,		/* = ( (pad)    position 42	code 0x49 */
ESCKEY | REPEAT, 0x04, 0x05, 0x05,		/* TAB          position 45	code 0x4a */
ALPHA | REPEAT, 'g', BEL, 'G',			/* G            position 70	code 0x4b */
REPEAT | FLOWKEY, 0x02, 0x02, 0x02,		/* S            position 67	code 0x4c */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x4d */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x4e */
CURSKEY | REPEAT, 0x02, 0x02, 0x02,		/* right arrow  position 99	code 0x4f */
/**********************************************************************************/
PFKEY, 0x04, 0x04, 0x04,			/* F5           position 9	code 0x50 */
ALPHA | REPEAT, 'c', ETX, 'C',			/* C            position 87	code 0x51 */
REPEAT, '/', US, '?',				/* / ?          position 94	code 0x52 */
REPEAT | PADKEY, '0', '0', 0xd0,		/* 0 (pad)      position 108	code 0x53 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x54 */
REPEAT, '`', NU, '~',				/* ` ~          position 59	code 0x55 */
ALPHA | REPEAT, 'q', DC1, 'Q',			/* Q            position 46	code 0x56 */
ALPHA | REPEAT, 't', DC4, 'T',			/* T            position 50	code 0x57 */
REPEAT, '.', RS, '>',				/* . >          position 93	code 0x58 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x59 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x5a */
ESCKEY, 0x00, 0x01, NU,			/* CLEAR RESET  position 41	code 0x5b */
ALPHA | REPEAT, 'a', SOH, 'A',			/* A            position 66	code 0x5c */
ALPHA | REPEAT, 'r', DC2, 'R',			/* R            position 49	code 0x5d */
REPEAT, ']', GS, '}',				/* ] }          position 57	code 0x5e */
REPEAT, SPACE, SPACE, SPACE,			/* SPACE        position 104	code 0x5f */
/**********************************************************************************/
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x60 */
REPEAT | PADKEY, '/', '/', 0xc3,		/* // (pad)     position 44	code 0x61 */
REPEAT, ESC, ESC, ESC,				/* ESC          position 25	code 0x62 */
REPEAT, '5', NU, '%',				/* 5 %          position 30	code 0x63 */
REPEAT, BS, BS, BS,				/* BACKSPACE    position 38	code 0x64 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x65 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x66 */
SFKEY, 0x0a, 0x0a, 0x0a,			/* F11          position 17	code 0x67 */
REPEAT, '1', NU, '!',				/* 1 !          position 26	code 0x68 */
ALPHA | REPEAT, 'f', ACK, 'F',			/* F            position 69	code 0x69 */
RETURNKEY | REPEAT, 0x00, 0x00, 0x00,		/* RETURN       position 77	code 0x6a */
REPEAT | PADKEY, '2', '2', 0xce,		/* 2 (pad)      position 101	code 0x6b */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x6c */
REPEAT | PADKEY, '1', '1', 0xcd,		/* 1 (pad)      position 100	code 0x6d */
REPEAT, '2', 0x00, 0x40,			/* 2 @          position 27	code 0x6e */
ALPHA | REPEAT, 'x', CAN, 'X',			/* X            position 86	code 0x6f */
/**********************************************************************************/
REPEAT, ',', NUL, '<',				/* , <          position 92	code 0x70 */
CAPSKEY, 0x00, 0x00, 0x00,			/* CAPS LOCK    position 64	code 0x71 */
SFKEY, 0x0c, 0x0c, 0x0c,			/* F13          position 19	code 0x72 */
REPEAT | PADKEY, '-', '-', 0xc8,		/* - (pad)      position 112	code 0x73 */
REPEAT, '6', RS, '^',				/* 6 ^          position 31	code 0x74 */
REPEAT | PADKEY, '5', '5', 0xca,		/* 5   (pad)    position 81	code 0x75 */
CURSKEY | REPEAT, 0x01, 0x01, 0x01,		/* down arrow   position 107	code 0x76 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x77 */
REPEAT | PADKEY, '3', '3', 0xcf,		/* 3 (pad)      position 102	code 0x78 */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x79 */
ALPHA | REPEAT, 'y', EM, 'Y',			/* Y            position 51	code 0x7a */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x7b */
NOCODE, NOKEY, NOKEY, NOKEY,			/*		position	code 0x7c */
SFKEY, 0x0b, 0x0b, 0x0b, 			/* F12          position 18	code 0x7d */
SFKEY, 0x0d, 0x0d, 0x0d, 			/* F14          position 110	code 0x7e */
REPEAT | PADKEY, '+', '+', 0xc4,		/* + (pad)      position 111	code 0x7f */
};
