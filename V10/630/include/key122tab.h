/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(key122tab, @(#)key122tab.h	1.1.1.3	(5/12/87));

/*******************************************************************************

MODULE NAME:    kbd122()

DESCRIPTION:    The lines in the table are set up in the following way: 
		key.traits, key.code[] = { unshift, control, shift, }

*******************************************************************************/
struct key key122tab[] = {
REPEAT | PADKEY, SPACE, SPACE, 0xc4,		/* SPACE (pad) 	position 48	code 0x00 */
REPEAT, '4', NU, '$',				/* 4 $		position 31	code 0x01 */
ALPHA | REPEAT, 'n', SO, 'N',			/* N		position 102	code 0x02 */
SFKEY, 0x14, 0x14, 0x14,			/* PF21		position 9 	code 0x03 */
NOCODE, NU, NU, NU,				/* FIELD MARK	position 43	code 0x04 */
PFKEY, 0x07, 0x07, 0x07,			/* PF8		position 20	code 0x05 */
PFKEY, 0x04, 0x04, 0x04,			/* PF5		position 17	code 0x06 */
ESCKEY, 0x00, 0x00, 0x00,			/* CLR		position 117	code 0x07 */
ALPHA | REPEAT, 'm', CR, 'M',			/* M		position 103	code 0x08 */
ALPHA | REPEAT, 'r', DC2, 'R',			/* R		position 55	code 0x09 */
REPEAT, '[', ESC, '!',				/* [ !		position 62	code 0x0a */
SFKEY, 0x17, 0x17, 0x17,			/* PF24		position 12	code 0x0b */
ALPHA | REPEAT, 'l', FF, 'L',			/* L		position 83	code 0x0c */
PADKEY | REPEAT, '0', '0', 0xd0,		/* 0 (pad)	position 124 	code 0x0d */
CURSKEY | REPEAT, 0x02, 0x06, NU,		/* right arrow	position 111	code 0x0e */
ALPHA | REPEAT, 'f', ACK, 'F',			/* F		position 78	code 0x0f */
/**********************************************************************************/
REPEAT, '{', NU, '}',				/* { }		position 86	code 0x10 */
REPEAT | PADKEY, '2', '2', 0xce,		/* 2 (pad)	position 113	code 0x11 */
REPEAT | PADKEY, '5', '5', 0xca,		/* 5 (pad)	position 90	code 0x12 */
PFKEY, 0x05, 0x05, 0x05,			/* PF6		position 18	code 0x13 */
ALPHA | REPEAT, 'i', HT, 'I',			/* I		position 59	code 0x14 */
ALPHA | REPEAT, 'c', ETX, 'C',			/* C		position 99	code 0x15 */
REPEAT, '\\', FS, '|',				/* \\ |		position 63	code 0x16 */
REPEAT | ESCKEY, 0x02, 0x03, NU,		/*del char/line position 67	code 0x17 */
CURSKEY, NU | 0x04, 0x04, NU | 0x04,		/* HOME		position 110	code 0x18 */
ALPHA | REPEAT, 'p', DLE, 'P',			/* P		position 61	code 0x19 */
ALPHA | REPEAT, 'k', VT, 'K', 			/* K		position 82	code 0x1a */
REPEAT | PADKEY, '7', '7', 0xc5,		/* 7 (pad)	position 68	code 0x1b */
ALPHA | REPEAT, 'd', EOT, 'D',			/* D		position 77	code 0x1c */
SPECIAL, 0x01, 0x01, 0x01,			/* CONTROL	position 121	code 0x1d */
SPECIAL, 0x00, 0x00, 0x00,			/* SHIFT	position 95	code 0x1e */
REPEAT, ';', NU, ':',				/* ; :		position 84	code 0x1f */
/**********************************************************************************/
PFKEY, 0x03, 0x03, 0x03,			/* PF4		position 16	code 0x20 */
REPEAT, '9', NU, '(',				/* 9 (		position 36	code 0x21 */
ALPHA | REPEAT, 'x', CAN, 'X',			/* X 		position 98	code 0x22 */
REPEAT | PADKEY, '9', '9', 0xc7,		/* 9 (pad)	position 70	code 0x23 */
REPEAT, ',', NU, ',',				/* , ,		position 104	code 0x24 */
REPEAT, 0x27, NU, '"',				/* ' "		position 85	code 0x25 */
SPECIAL, 0x01, 0x01, 0x01,			/* CONTROL	position 119	code 0x26 */
REPEAT, '1', GS, ']',				/* 1 ]		position 28	code 0x27 */
NOCODE, NU, NU, NU,				/* DUP PA1	position 42	code 0x28 */
SFKEY, 0x0e, 0x0e, 0x0e,			/* PF15		position 3	code 0x29 */
REPEAT, '.', NU, '.',				/* . .		position 105	code 0x2a */
SPECIAL, 0x00, 0x00, 0x00,			/* SHIFT	position 107	code 0x2b */
PFKEY, 0x06, 0x06, 0x06,			/* PF7		position 19	code 0x2c */
REPEAT, DEL, DEL, DEL,				/* DELETE	position 94	code 0x2d */
REPEAT, BS, NU, BS,				/* BS/ERASE&TAB	position 40	code 0x2e */
REPEAT | PADKEY, '6', '6', 0xcb,		/* 6 (pad)	position 91	code 0x2f */
/**********************************************************************************/
REPEAT, '/', US, '?',				/* / ?		position 106	code 0x30 */
NOCODE, NU, NU, NU,				/* BLANK KEY	position 44	code 0x31 */
REPEAT | PADKEY, '4', '4', 0xc9,		/* 4 (pad)	position 89	code 0x32 */
REPEAT, '8', NU, '*',				/* 8 *		position 35	code 0x33 */
REPEAT, BS, NU, BS,				/* BS/ERASE&TAB	position 41	code 0x34 */
REPEAT | PADKEY, HT, HT, 0xc8,			/* TAB (pad)	position 71	code 0x35 */
SFKEY, 0x16, 0x16, 0x16,			/* PF23		position 11	code 0x36 */
ALPHA | REPEAT, 'o', SI, 'O',			/* O            position 60	code 0x37 */
ESCKEY, 0x01, 0x01, 0x01,			/* RESET	position 118	code 0x38 */
REPEAT, HT, HT, HT,				/* TAB		position 51	code 0x39 */
CURSKEY | REPEAT, 0x03, 0x05, NU,		/* left arrow	position 109	code 0x3a */
NOCODE, NU, NU, NU,				/* ERASE EOF	position 73	code 0x3b */
NOCODE, NU, NU, NU,				/* ????		position 72	code 0x3c */
ESCKEY | REPEAT, 0x06, 0x07, NU,		/* ^a		position 66	code 0x3d */
REPEAT, SPACE, SPACE, SPACE,			/* SPACE        position 120 	code 0x3e */
ALPHA | REPEAT, 'u', NAK, 'U',			/* U            position 58	code 0x3f */
/**********************************************************************************/
SFKEY, 0x09, 0x09, 0x09,			/* PF10		position 22	code 0x40 */
REPEAT, '3', NU, '#',				/* 3 #		position 30	code 0x41 */
SFKEY, 0x15, 0x15, 0x15,	 		/* PF22         position 10	code 0x42 */
SFKEY, 0x0c, 0x0c, 0x0c,			/* PF13		position 1	code 0x43 */
ALPHA | REPEAT, 'q', DC1, 'Q',			/* Q		position 52	code 0x44 */
REPEAT, '7', NU, '&',				/* 7 &		position 34	code 0x45 */
ESCKEY | REPEAT, 0x05, 0x05, 0x05,		/* |<-		position 65	code 0x46 */
PFKEY, 0x01, 0x01, 0x01,			/* PF2		position 14	code 0x47 */
NOCODE, NU, NU, NU,				/* CURSR SEL	position 50	code 0x48 */
REPEAT | PADKEY, '1', '1', 0xcd,		/* 1 (pad)	position 112	code 0x49 */
ALPHA | REPEAT, 'a', SOH, 'A',			/* A		position 75	code 0x4a */
ALPHA | REPEAT, 'y', EM, 'Y',			/* Y            position 57	code 0x4b */
REPEAT | ALPHA, 'e', ENQ, 'E',			/* E            position 54	code 0x4c */
FLOWKEY, 0x00, 0x03, NU,			/* BREAK DISCON	position 93	code 0x4d */
FLOWKEY, 0x01, 0x01, 0x01,			/* SCROLL LOCK	position 116	code 0x4e */
SFKEY, 0x13, 0x13, 0x13,			/* PF20		position 8	code 0x4f */
/**********************************************************************************/
SFKEY, 0x0a, 0x0a, 0x0a,			/* PF11         position 23	code 0x50 */
REPEAT, '6', RS, '^',				/* 6 ^		position 33	code 0x51 */
REPEAT, '=', NU, '+',				/* = +		position 39	code 0x52 */
REPEAT, ESC, ESC, ESC,				/* ESC		position 26	code 0x53 */
SFKEY, 0x0b, 0x0b, 0x0b,			/* PF12		position 24	code 0x54 */
CURSKEY | REPEAT, 0x00, 0x00, 0x00,		/* up arrow	position 88	code 0x55 */
REPEAT | FLOWKEY, 0x02, 0x02, 0x02,		/* S            position 76	code 0x56 */
ALPHA | REPEAT, 'h', BS, 'H',			/* H            position 80	code 0x57 */
REPEAT, '-', US, '_',				/* - _		position 38	code 0x58 */
NOCODE, NU, NU, NU,				/* ????		position 49	code 0x59 */
ENTERKEY | REPEAT, 0x00, 0x00, 0x00,		/* ENTER	position 122	code 0x5a */
SFKEY, 0x10, 0x10, 0x10,			/* PF17		position 5	code 0x5b */
ALPHA | REPEAT, 'w', ETB, 'W',			/* W            position 53	code 0x5c */
ALPHA | REPEAT, 'g', BEL, 'G',			/* G            position 79	code 0x5d */
RETURNKEY | REPEAT, 0x00, 0x00, 0x00,		/* RETURN KEY	position 87	code 0x5e */
REPEAT, '`', NU, '~',				/* ` ~		position 27	code 0x5f */
/**********************************************************************************/
SFKEY, 0x08, 0x08, 0x08,			/* PF9		position 21	code 0x60 */
REPEAT | PADKEY, '3', '3', 0xcf,		/* 3 (pad)	position 114	code 0x61 */
PFKEY, 0x02, 0x02, 0x02,			/* PF3          position 15	code 0x62 */
ALPHA | REPEAT, 'v', SYN, 'V',			/* V		position 100	code 0x63 */
SPECIAL, 0x00, 0x00, 0x00,			/* SHIFT	position 108	code 0x64 */
NOCODE, NU, NU, NU,				/* ATTN SYSREQ	position 25	code 0x65 */
CAPSKEY, 0x00, 0x00, 0x00,			/* CAPS LOCK	position 74	code 0x66 */
PADKEY | REPEAT, '0', '0', 0xd0,		/* 0 (pad)	position 125	code 0x67 */
REPEAT, '<', NU, '>',				/* < >          position 96	code 0x68 */
ALPHA | REPEAT, 't', DC4, 'T',			/* T            position 56	code 0x69 */
RETURNKEY | REPEAT, 0x00, 0x00, 0x00,		/* RETURN KEY	position 64	code 0x6a */
REPEAT | PADKEY, '+', '+', 0xc2,		/* + (pad)      position 46	code 0x6b */
CURSKEY | REPEAT, 0x01, 0x01, 0x01,		/* down arrow	position 123	code 0x6c */
PADKEY, NU | 0xc1, NU | 0xc1, 0xc1,		/* BLANK KEY (pad) position 45	code 0x6d */
ALPHA | REPEAT, 'z', SUB, 'Z',			/* Z		position 97	code 0x6e */
REPEAT, '5', NU, '%',				/* 5 %          position 32	code 0x6f */
/**********************************************************************************/
REPEAT, '0', NU, ')',				/* 0 )		position 37	code 0x70 */
PFKEY, 0x00, 0x00, 0x00,			/* PF1		position 13	code 0x71 */
REPEAT | PADKEY, 0x00, 0x00, 0xc0,		/* ENTER (pad)	position 127	code 0x72 */
REPEAT | PADKEY, '-', '-', 0xcc,		/* - (pad)	position 92	code 0x73 */
ALPHA | REPEAT, 'b', STX, 'B',			/* B		position 101	code 0x74 */
REPEAT | PADKEY, '8', '8', 0xc6,		/* 8 (pad)	position 69	code 0x75 */
REPEAT, '2', NUL, 0x40,				/* 2 @          position 29	code 0x76 */
SFKEY, 0x0d, 0x0d, 0x0d,			/* PF14		position 2	code 0x77 */
REPEAT | PADKEY, ',', ',', 0xc3,		/* , (pad)      position 47	code 0x78 */
SFKEY, 0x11, 0x11, 0x11,			/* PF18		position 6	code 0x79 */
ALPHA | REPEAT, 'j', LF, 'J',			/* J            position 81	code 0x7a */
SFKEY, 0x12, 0x12, 0x12,			/* PF19		position 7	code 0x7b */
SFKEY, 0x0f, 0x0f, 0x0f,			/* PF16		position 4	code 0x7c */
PADKEY | REPEAT, '.', '.', 0xd1, 		/* . (pad)	position 126	code 0x7d */
NOCODE, NOKEY, NOKEY, NOKEY, 			/* 		position 	code 0x7e */
REPEAT | PADKEY, 0x00, 0x00, 0xc0,		/* ENTER (pad)	position 115	code 0x7f */
};
