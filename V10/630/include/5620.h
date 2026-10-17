/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(5620, @(#)5620.h	1.1.1.9	(6/3/87));

extern Point		add();
extern Word		*addr();
extern char		*alloc();
extern Bitmap		*balloc();
extern Texture16	*cursswitch();
extern Point		div();
extern Rectangle	inset();
extern Point		jstring();
extern Point		mul();
extern struct Proc	*point2window();
extern Rectangle	raddp();
extern Rectangle	rsubp();
extern Point		string();
extern Point		sub();
extern char		*gcalloc();
extern Rectangle	newrect();
extern Point		transform();
extern Rectangle	rtransform();
extern unsigned long	realtime();
extern Rectangle	canon();
extern char		*itox();
extern long		version();
extern short		Iceil();
extern short		Ifloor();
extern long		Lsqrt();
extern char		*fgets();

#define floor		Ifloor
#define ceil		Iceil
#define sin		Isin
#define cos		Icos
#define atan2		Iatan2
#define sqrt		Lsqrt
#define defont		mediumfont
#define texture16	texture
#undef Pt
#undef Rpt
#undef Rect
#define Pt		fPt
#define Rpt		fRpt
#define Rect		fRect
extern Point		fPt();
extern Rectangle	fRpt();
extern Rectangle	fRect();

extern Texture16 C_sweep;
extern Texture16 C_crosshair;
extern Texture16 C_confirm;

/* Font properties from setup.h */
#define	CW	9	/* width of a character */
#define	NS	14	/* newline size; height of a character */
#define	CURSOR	"_"	/* By convention */

#define	XMARGIN	3	/* inset from border */
#define	YMARGIN	3
#define XCMAX ((XMAX-2*XMARGIN)/CW-1)
#define YCMAX ((YMAX-2*YMARGIN)/NS-3)

/*
 * Cast	macros
 */
#define	Cast(t,	x)	((t (*)())Sys[x])
#define	IPoint(x)	(*Cast(Point, x))
#define	IRectangle(x)	(*Cast(Rectangle, x))
#define	Iint(x)		(*Cast(int, x))
#define	Ilong(x)	(*Cast(long, x))
#define	Ishort(x)	(*Cast(short, x))
#define	IpBitmap(x)	(*Cast(Bitmap *, x))
#define	IpTexture(x)	(*Cast(Texture16 *, x))
#define	IpLayer(x)	(*Cast(Layer *,	x))
#define	IpWord(x)	(*Cast(Word *, x))
#define	Ipchar(x)	(*Cast(char *, x))
#define	Ivoid(x)	(*Cast(void, x))
#define	IpProc(x)	(*Cast(struct Proc *, x))
#define P_Titem(x)	Cast(struct Titem *, x) /* pointer to Titem struct */

#define outline(olr)	box(&physical, olr, F_XOR)
#define getrect()	newrect(3, Rect(0,0,0,0))
#define point2layer()	point2window(3)

#define MPX
