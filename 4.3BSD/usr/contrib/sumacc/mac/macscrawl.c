#include "mac/quickdraw.h"
#include "mac/osintf.h"
#include "mac/toolintf.h"

int	pensz	= 	5;
GrafPort *myPort;
int	wasevent,i,mouse;
Point	p,pold;
EventRecord er;
CursHandle plusH;
int	mode;		/* not an example of mode-less programming! */
#define	NORMAL	0
#define	MAGNIFY	1
#define	ASCII	2

main()
{
	struct QDVar QDVar;
	GrafPort gp;

	QD = &QDVar;
	InitGraf(&thePort);
	InitCursor();
	InitFonts();
	myPort = &gp;
	OpenPort(myPort);
	PenSize(pensz,pensz);
	MoveTo(256,256);
	samptext();
	plusH = GetCursor(3);
	for (;;) {
		SystemTask();
		wasevent = GetNextEvent(everyEvent, &er);
		switch (mode) {
		case NORMAL:
			normal();
			break;
		case MAGNIFY:
			magnify();
			break;
		case ASCII:
			ascii();
			break;
		}
	}
}

checkevent()		/* look for normal events */
{
	if (!wasevent)
		return;
	switch (er.what) {
	case keyDown:
	case autoKey:
		switch (er.message & 0xff) {
		case 'q':
			ExitToShell();

		case 'f': pensz += 1;  goto setpen;
		case 't': pensz -= 1;  goto setpen;
		case 'F': pensz += 16;  goto setpen;
		case 'T': pensz -= 16;  goto setpen;

		setpen:
			PenSize(pensz,pensz);
			break;

		case 'b':
			PenPat(&QD->black);
			break;

		case 'w':
			PenPat(&QD->white);
			break;

		case 'g':
			PenPat(&QD->gray);
			break;

		case 'm':
			mode = MAGNIFY;
			SetCursor(*plusH);
			break;

		case 'a':
			mode = ASCII;
			TextSize(pensz<12 ? 0 : pensz);
			break;

		case '\r':
			mode = NORMAL;
			SetCursor(&QD->arrow);
			break;
		}
		break;

	case mouseDown:
		mouse = 1;
		break;

	case mouseUp:
		mouse = 0;
		break;
	}
}

normal()
{
	checkevent();
	GetMouse(&p);
	if (EqualPt(&p, &pold))
		return;
	pold = p;
	if (mouse)
		LineTo(p.h,p.v);
	else
		MoveTo(p.h,p.v);
}

magnify()	
{
	Rect sr,dr;

	checkevent();
	GetMouse(&p);
	if (wasevent == 0 && EqualPt(&p, &pold))
		return;
	pold = p;
	SetRect(&dr, 0, 0, 128, 128);
	if (pensz <= 0)
		pensz = 1;
	SetRect(&sr, p.h, p.v, p.h + pensz, p.v + pensz);
	CopyBits(&myPort->portBits, &myPort->portBits,
		&sr, &dr, srcCopy, (RgnHandle)0);
}

ascii()
{
	int key;

	if (wasevent == 0 || er.what != keyDown)
		return;
	key = (er.message&0xff);
	if (key == '\r') {
		mode = NORMAL;
		return;
	}
	DrawChar(key);
}

samptext()	/* tests auto string conversion */
{
	char fontname[32];
	char numstring[32];
	int fontnum;

	GetFontName(0,fontname);
	GetFNum(fontname,&fontnum);
	DrawString("font: ");
	DrawString(fontname);
	NumToString(fontnum,numstring);
	DrawString(numstring);
}
