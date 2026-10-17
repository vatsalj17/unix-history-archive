/*
 * Test the (in)sane functions.  Probably the reason things are
 * so slow is that Sane does all calculations in Extended
 * (80 bit) IEEE floating point.
 * Rough timings of the inner loop below show about 5 milliseconds
 * per Sane operation.  This is a bit high since a sin and cos
 * are included;  the manual gives a figure of about 1 ms per multiply.
 *
 * For most applications that don't need 80 bits of precision, you
 * might be better off with the 32 bit Fixed-Point arithmetic discussed
 * in the "Toolbox Utilities" section (FixMul,FixRatio,FixRound).
 *
 *	--bc
 */

#include "mac/quickdraw.h"
#include "mac/toolintf.h"
#include "mac/sane.h"

int	pensz	= 	4;
GrafPort *myPort;

int	rI = 170;
Extended r,x0,y0,rinc;
int	ciradsI = 314159 * 2;
int	ciradscaleI = 100000;
Extended cirads,ciradscale;
int	segsI = 40;
int	zeroI = 0;
Extended theta,thetainc,thetaconv,zero,segs;
int	color;
DecForm	dform;
char	dstr[DECSTRLEN];


main()
{
	struct QDVar QDVar;
	GrafPort gp;
	int i;
	Rect txrect;

	QD = &QDVar;
	InitGraf(&thePort);
	InitCursor();
	myPort = &gp;
	OpenPort(myPort);
	PenSize(pensz,pensz);
	MoveTo(rI+rI,rI);

	dform.style = FixedDecimal;  dform.digits = 10;
	SetRect(&txrect,300,300,440,320);
	L2X(&rI,&r);
	L2X(&pensz,&rinc);
	x0 = r;  y0 = r;
	L2X(&ciradsI,&cirads);
	L2X(&ciradscaleI,&ciradscale);
	DivX(&ciradscale,&cirads);
	L2X(&segsI,&segs);
	L2X(&zeroI,&zero);
	thetainc = cirads;
	DivX(&segs,&thetainc);
	theta = zero;

	for ( ; rI > 6 ; rI -= pensz) {
		L2X(&rI,&r);
		if (color)
			PenPat(&QD->white);
		else
			PenPat(&QD->black);
		color = !color;
		for (i = 0 ; i <= segsI ; i++) {
			Extended x,y;
			int xi,yi;

			x = theta;
			CosX(&x);
			MulX(&r,&x);
			y = theta;
			SinX(&y);
			MulX(&r,&y);
			AddX(&x0,&x);
			AddX(&y0,&y);
			X2L(&x,&xi);
			X2L(&y,&yi);
			LineTo(xi,yi);
			/* show current theta */
			EraseRect(&txrect);
			MoveTo(txrect.left+2,txrect.bottom-2);
			if (Button()) {		/* double convert it */
				X2Str(&dform,&theta,dstr);
				theta = zero;	/* ensure it's really working */
				Str2X(dstr,&theta);
			}
			X2Str(&dform,&theta,dstr);
			DrawString(dstr);
			MoveTo(xi,yi);

			AddX(&thetainc,&theta);
		}
	}
	while (!Button());
}
