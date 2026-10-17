
/********* Frilly bits **********/

#include "cplot.h"

urect(x1,y1,x2,y2)
float x1,y1,x2,y2;
{
	int temg_px,temg_py;
	register int tx1,ty1;

	temg_px=g_spx;
	temg_py=g_spy;
	g_chtscr(x1,y1);
	tx1=g_spx;ty1=g_spy;
	g_chtscr(x2,y2);
	srect(tx1,ty1,g_spx,g_spy);
	g_chtusr(g_spx=temg_px,g_spy=temg_py);
}




srect(x1,y1,x2,y2)
int x1,y1,x2,y2;
{
	int tx,ty;
	tx=g_spx; ty=g_spy;
	smovea(x1,y2);
	sdrawa(x2,y2);
	sdrawa(x2,y1);
	sdrawa(x1,y1);
	sdrawa(x1,y2);
	g_chtusr(g_spx=tx,g_spy=ty);
	g_status =& ~ONSCREEN;
}

scircle(xcent,ycent,radius)
/*
This routine draws circles quickly, without the use of sine
and cosine functions, and leaves the cursor position undisturbed.
It was written by Adrian Freed.
*/
int xcent,ycent,radius;
{
	register steps;
	float a, b;
	float x;
	float y 0.0;
	float temp;
	register i;
	register t;
	int savex,savey;

	x=radius;
	savex=g_spx;
	savey=g_spy;

	if(radius >200)
	{
		a=0.9980268;
		b=0.06279059;
		steps=100;
	}
	else	if(radius >100)
		{
			a=0.987688341;
			b=0.15643447;
			steps=40;
		}
		else
		{
			a=0.951056516;
			b=0.30901699;
			steps=20;
		}

	smovea(xcent+radius,ycent);

	for(i=0;i < steps;i++)
	{
		temp= a*y + b*x;
		x= a*x - b*y;
		y=temp;
		sdrawa(t=x+xcent,t=y+ycent);
	}
	g_chtusr(g_spx=savex,g_spy=savey);
}
