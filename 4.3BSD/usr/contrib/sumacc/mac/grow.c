/*	grow.c	1.0	05/25/84		*/

/*
 * grow
 *
 * scroll bars and a resizable window added to "edit".
 * by Cary Clark, Mac Tech Support
 *
 * translation of the pascal sample program "grow".
 *
 * NOTE: even the Pascal version of this doesnt work properly with
 * the notepad/scrapbook desk accessories, so this example must not be
 * saving proper state from the TextEdit package...
 */

/*
 * history
 * 05/25/84	Croft	translated from Pascal
 */


#include "mac/quickdraw.h"
#include "mac/osintf.h"
#include "mac/toolintf.h"

#define	numMenu		3	/* number of menus */
#define	appleMenu	1	/* menu IDs */
#define	fileMenu	256
#define	editMenu	257

#define	TRUE	1
#define	VIS	1
#define	FALSE	0
#define	NIL	0


MenuHandle myMenus[numMenu];
Rect	growRect, dragRect, pRect, tRect;
EventRecord myEvent;
int	code, refNum, myControl, t;
WindowRecord wRecord;
WindowPtr theWindow, whichWindow;
int	theMenu, theItem;
char	theChar;
int	ticks;
TEHandle hTE;
CursHandle hCurs;
Cursor	iBeam;
ControlHandle hScroll, vScroll, whichControl;
Point	theOrigin;


main()
{
	struct QDVar QDVar;	/* or we could call NewPtr */

	QD = &QDVar;
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent,0);
	InitWindows();
	SetupMenus();
	TEInit();
	InitDialogs((ProcPtr)NIL);
	SetCursor(&QD->arrow);
	SetRect(&dragRect,4,24,508,338);
	SetRect(&growRect,100,60,512,302);

	theWindow = GetNewWindow(256, &wRecord, (WindowPtr)-1);
	SetPort(theWindow);
	theWindow->txFont = 2;

	ResizePRect();
	hTE = TENew(&pRect,&pRect);
	hCurs = GetCursor(256); iBeam = **hCurs;

	vScroll =   GetNewControl(256, theWindow);
	hScroll =   GetNewControl(257, theWindow);
	theOrigin.h = 0; theOrigin.v = 0;

	for (;;) {
		CursorAdjust();
		SystemTask();
		TEIdle(hTE);
		t = GetNextEvent(everyEvent,&myEvent);
		switch (myEvent.what) {
		case mouseDown:
			code = FindWindow(&myEvent.where,&whichWindow);
			switch (code) {
			case inMenuBar:
				DoCommand(MenuSelect(&myEvent.where));
				break;

			case inSysWindow:
				SystemClick(&myEvent,whichWindow);
				break;

			case inDrag:
				DragWindow(whichWindow,
				    &myEvent.where,&dragRect);
				break;

			case inGoAway:
				if (TrackGoAway(whichWindow,&myEvent.where))
					Exit();
				break;
			
			case inGrow:
				if (whichWindow == FrontWindow())
					GrowWnd(whichWindow);
				else
					SelectWindow(whichWindow);
				break;

			case inContent:
				if (whichWindow != FrontWindow())
					SelectWindow(whichWindow);
				else
					mouseme();
			}
			break;

		case keyDown:
		case autoKey:
			if (theWindow != FrontWindow())
				break;
			theChar = (myEvent.message & 0xff);
			if (myEvent.modifiers & cmdKey)
				DoCommand(MenuKey(theChar));
			else
				TEKey(theChar,hTE);
			break;

		case activateEvt:
			DrawGrowIcon(theWindow);
			if (myEvent.modifiers & activeFlag) {
				TEActivate(hTE);
				ShowControl (vScroll);
				ShowControl (hScroll);
			} else {
				TEDeactivate(hTE);
				HideControl (vScroll);
				HideControl (hScroll);
			}
			break;

		case updateEvt:
			BeginUpdate(theWindow);
			DrawWindow(theWindow);
			EndUpdate(theWindow);
			break;
		}
	}
}


SetupMenus()
{
	int i;

	InitMenus();
	myMenus[0] = NewMenu(appleMenu, "\024");
	AddResMenu(myMenus[0], "DRVR");
	myMenus[1] = GetMenu(fileMenu);
	myMenus[2] = GetMenu(editMenu);
	for (i = 0 ; i < numMenu ; i++) 
		InsertMenu(myMenus[i], 0);
	DrawMenuBar();
}


/*
 * Makes cursor be I-beam inside the (active) application window's
 * content region (except for size box and scroll bar areas).
 */
CursorAdjust()
{
	Point mousePt;

	GetMouse(&mousePt);
	if (theWindow != FrontWindow())
		return;
	if (PtInRect(&mousePt,&pRect))
		SetCursor(&iBeam);
	else
		SetCursor(&QD->arrow);
}


DoCommand(mResult)
{
	char name[64];
	char *DAname();

	theMenu = mResult >> 16;
	theItem = (mResult & 0xffff);
	switch (theMenu) {
	case appleMenu:
		GetItem(myMenus[0], theItem, name);
		refNum = OpenDeskAcc(DAname(name));
		break;

	case fileMenu:
		Exit();
		break;

	case editMenu:
		/* 
		 * SystemEdit expects 0=undo, 1=----, 2=cut, 3=copy, etc.,
		 */
		if (SystemEdit(theItem-1))
			break;
		SetPort(theWindow);
		ClipRect(&pRect);
		/* delay so title stays lit if cmd key typed */
		ticks = TickCount() + 30;
		while (TickCount() <= ticks);
		switch (theItem) {
		case 3:
			TECut(hTE);
			break;

		case 4:
			TECopy(hTE);
			break;
		
		case 5:
			TEPaste(hTE);
			break;
		}
		break;
	}
	HiliteMenu(0);
}


MoveScrollBars()
{
	register Rect *r;

	r = &theWindow->portRect;
	HideControl(vScroll);
	MoveControl(vScroll,r->right - 15, r->top - 1);
	SizeControl(vScroll,16, r->bottom - r->top - 13);
	ShowControl(vScroll);
	HideControl(hScroll);
	MoveControl(hScroll,r->left - 1, r->bottom - 15);
	SizeControl(hScroll,r->right - r->left - 13, 16);
	ShowControl(hScroll);
}


ResizePRect()
{
	pRect = thePort->portRect;
	pRect.left = pRect.left+4; pRect.right = pRect.right-15;
	pRect.bottom = pRect.bottom-15;
}


/*
 * Handles growing and sizing the window and manipulating
 * the update region.
 */
GrowWnd(whichWindow)
	WindowPtr whichWindow;
{
	int	longResult;
	int	height,width;


	longResult = GrowWindow(whichWindow, &myEvent.where, &growRect);
	if (longResult == 0)
		return;
	height = HiWord(longResult); width = LoWord(longResult);

	/* Add the old "scroll bar area" to the update region so it will */
	/* be redrawn (for when the window is enlarged). */
	tRect = whichWindow->portRect; tRect.left = tRect.right-16;
	InvalRect(&tRect);
	tRect = whichWindow->portRect; tRect.top = tRect.bottom-16;
	InvalRect(&tRect);

	/* Now draw the newly sized window. */
	SizeWindow(whichWindow,width,height,TRUE);
	MoveScrollBars();
	ResizePRect();

	/* Adjust the view rectangle for TextEdit. */
	(*hTE)->viewRect = pRect;

	/* Add the new "scroll bar area" to the update region so it will */
	/* be redrawn (for when the window is made smaller). */
	tRect = whichWindow->portRect; tRect.left = tRect.right-16;
	InvalRect(&tRect);
	tRect = whichWindow->portRect; tRect.top = tRect.bottom-16;
	InvalRect(&tRect);
}


/* Draws the content region of the given window, after erasing whatever */
/* was there before. */
DrawWindow(whichWindow)
	WindowPtr whichWindow;
{
	ClipRect(&whichWindow->portRect);
	EraseRect(&whichWindow->portRect);
	DrawGrowIcon(whichWindow);
	DrawControls(whichWindow);
	TEUpdate(&pRect,hTE);
}


ScrollBits()
{
	Point oldOrigin;
	int dh,dv;

	oldOrigin = theOrigin;
	theOrigin.h = 4*GetCtlValue(hScroll);
	theOrigin.v = 4*GetCtlValue(vScroll);
	dh = oldOrigin.h - theOrigin.h;
	dv = oldOrigin.v - theOrigin.v;
	TEScroll (dh, dv, hTE);
}


/*
 * since ScrollUp and ScrollDown are called from Pascal, the
 * arguments must be obtained with "getpargs".
 */
struct tcargs {		/* args passed from Pascal TrackControl */
	short	theCode;
	ControlHandle whichControl;
};


ScrollUp()
{
	struct tcargs a;

	getpargs(&a, sizeof a);
	if (a.theCode != inUpButton)
		return;
	SetCtlValue(a.whichControl,GetCtlValue(a.whichControl)-1);
	ScrollBits();
}


ScrollDown()
{
	struct tcargs a;

	getpargs(&a, sizeof a);
	if (a.theCode != inDownButton)
		return;
	SetCtlValue(a.whichControl,GetCtlValue(a.whichControl)+1);
	ScrollBits();
}


PageScroll(code, amount)
{
	Point myPt;

	do {
		GetMouse(&myPt);
		if (TestControl(whichControl, &myPt) != code)
			continue;
		SetCtlValue(whichControl,GetCtlValue(whichControl)+amount);
		ScrollBits();
	} while (StillDown());
}


Exit()
{
	ExitToShell();
}


mouseme()
{
	GlobalToLocal(&myEvent.where);
	if (PtInRect (&myEvent.where, &pRect)) {	/* in text */
		if (myEvent.modifiers & shiftKey)
			TEClick(&myEvent.where,TRUE,hTE);
		else
			TEClick(&myEvent.where,FALSE,hTE);
		return;
	}
	/* else was a control */
	myControl = FindControl(&myEvent.where,whichWindow,&whichControl);
	switch (myControl) {
	case inUpButton:
		t = TrackControl(whichControl,&myEvent.where,ScrollUp);
		break;

	case inDownButton:
		t = TrackControl(whichControl,&myEvent.where,ScrollDown);
		break;

	case inPageUp:
		PageScroll(myControl,-10);
		break;

	case inPageDown:
		PageScroll(myControl,10);
		break;

	case inThumb:
		t = TrackControl(whichControl,&myEvent.where,(ProcPtr)NIL);
		ScrollBits();
		break;
	}
}


/*
 * Desk accessories have weird names that begin with a leading zero.
 * Since this fools the automatic C/Pascal string conversion stuff, we
 * have this little gem;  given a C string, it returns a Pascal string
 * with leading zero.  Luckily I know of no other dependencies on embedded
 * zeros in Macintosh strings.
 */
char *DAname(s)
	char *s;
{
	static char ps[32];
	int i = 1;
	register char *cp,*dp;

	cp = s;
	dp = &ps[2];
	while ((*dp++ = *cp++))
		i++;
	ps[0] = i;
	return (isapstr(ps));
}

