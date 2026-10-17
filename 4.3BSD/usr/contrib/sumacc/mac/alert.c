/*	alert.c		1.0	06/18/84	*/

/*
 * Test alert and dialog boxes.
 */

/*
 * history
 * 06/18/84	Schilit	Created.
 * 07/05/84	Croft	Added DLOG test.
 */

#include "mac/quickdraw.h"
#include "mac/osintf.h"
#include "mac/toolintf.h"
#include "mac/packintf.h"

#define NIL 0

main()
{
	struct QDVar newqd;
	int times;
	char tstr[20];			/* big enough for any number */
	DialogPtr dp;
	int item;
	
	QD = &newqd;
	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs((ProcPtr)NIL);
	SetCursor(&newqd.arrow);
	
	times = 1;
	while (CautionAlert(257,(ProcPtr)NIL) == OK) { /* do alert dialog */	
		times++;			/* Increment number of times */
		NumToString(times,tstr);	/* make it into a string */
					/* and setup ^0..^3 */
		if (times > 4)		/* according to times here */
			ParamText("For crying out loud, that's ",tstr,
		    	      " times, give me a break! ","");
		else	      
			ParamText("You've been here ",tstr," times. ","");
        }

	dp = GetNewDialog(7, (Ptr)0, (WindowPtr)-1);
	do {
		Rect box;
		Handle itemH;
		int itemtype;

		ModalDialog((ProcPtr)NIL, &item);
		GetDItem(dp, item, &itemtype, &itemH, &box);
		switch (itemtype) {
		case ctrlItem+chkCtrl:
		case ctrlItem+radCtrl:
			SetCtlValue(itemH, GetCtlValue(itemH) ? 0 : 1);
		}
	} while (item != Cancel);
	DisposDialog(dp);
	ExitToShell();
}
