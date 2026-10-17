/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(font, @(#)font.h	1.1.1.4	(5/10/87));

/*
 * <font.h> - Jerq font format
 *
 * given char c, Font *f, Fontchar *i, and Point p, one says
 *	i = f->info+c;
 *	bitblt(f->bits,Rect(i->x,i->top,(i+1)->x,i->bottom),
 *		b,Pt(p.x+left,p.y),fc);
 *	p.x += i->width;
 *
 */

#ifndef	FONT_H
#define	FONT_H

#define FONTWIDTH(font)		((int)(font.info[' '].width))
#define FONTHEIGHT(font)	((int)(font.height))

typedef struct Fontchar
{
	short x;		/* left edge of bits */
	unsigned char top;	/* first non-zero scan-line */
	unsigned char bottom;	/* last non-zero scan-line */
	char left;		/* offset of baseline */
	unsigned char width;	/* width of baseline */
} Fontchar;

typedef struct Font
{
	short n;		/* number of chars in font */
	char height;		/* height of bitmap */
	char ascent;		/* top of bitmap to baseline */
	long unused;		/* in case we think of more stuff */
	Bitmap *bits;		/* where the characters are */
	Fontchar info[1];		/* n+1 character descriptors */
} Font;

#define ISIZE(n)	((n+1)*sizeof(Fontchar))
#define FCHARSIZE	8
Font *	getfont();	/* (fname) reads in a font file */
Font *	infont();	/* (func) makes a font */
Font *	fontsave();	/* (name,f) copy a font into the available list */
Font *  fontrequest();	/* (name) request a font from the available list */
Font *	fontavail();	/* (name) test if a font is available */

#endif	FONT_H
