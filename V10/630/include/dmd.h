/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(dmd, @(#)dmd.h	1.1.1.3	(5/10/87));


/*
 * Model 630 Dot Mapped Display
 */


#define	WORDSHIFT	4
#define	WORDSIZE	16
#define	WORDMASK	(WORDSIZE-1)
#define	ONES		0xFFFF
#define	FIRSTBIT	((unsigned short)0x8000)
#define	LASTBIT		((unsigned short)0x1)

#define	XMAX	1024
#define	YMAX	1024

/*
 *	Graphics definitions
 */

#define	POLY_F	-32768

#define	Pt(x, y)	x, y
#define	Rpt(x, y)	x, y
#define	Rect(a, b, c, d)	a, b, c, d

typedef short Word;
typedef unsigned short UWord;

typedef struct Point {
	short	x;
	short	y;
} Point;

typedef struct Rectangle {
	Point origin;
	Point corner;
} Rectangle;

typedef struct Bitmap {
	Word	*base;		/* pointer to start of data */
	unsigned short width;	/* width in Word's of total data area */
	Rectangle rect;		/* rectangle in data area, local coords */
	char	*_null;		/* unused, always zero */
} Bitmap;

typedef struct Menu{
	char	**item;			/* string array, ending with 0 */
	short	prevhit;		/* retained from previous call */
	short	prevtop;		/* also retained from previous call */
	char	*(*generator)();	/* used if item == 0 */
} Menu;

typedef struct Texture16 {
	Word bits[16];
} Texture16;

struct Mouse {
	Point	xy, jxy;
	short	buttons;
};

extern Rectangle Jrect;

#define bttn(i)		(mouse.buttons & (8 >> i))
#define bttn1()		(mouse.buttons&4)
#define bttn2()		(mouse.buttons&2)
#define bttn3()		(mouse.buttons&1)
#define bttn12()	(mouse.buttons&6)
#define bttn13()	(mouse.buttons&5)
#define bttn23()	(mouse.buttons&3)
#define bttn123()	(mouse.buttons&7)

#define	muldiv(a,b,c)	((short)((a)*((long)b)/(c)))


/*
 * Function Codes
 */
typedef int	Code;
#define	F_STORE	((Code) 0)	/* target = source */
#define	F_OR	((Code) 1)	/* target |= source */
#define	F_CLR	((Code) 2)	/* target &= ~source */
#define	F_XOR	((Code) 3)	/* target ^= source */

#define	NULL	((char *)0)
#define	KBD	1
#define	SEND	2
#define	MOUSE	4
#define	RCV	8
#define	CPU	16
#define ALARM	32
#define PSEND	64
#define DELETE  128
#define MSG	256
#define RESHAPED	0x800	/* this must be the same as in dmdproc.h */

#include <mpx.h>
