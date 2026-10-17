/*	mfinder.c	1.0	07/08/84	*/

/*
 * Mini Finder.
 */

/*
 * history
 * 07/08/84	Croft	Created.
 * 11/01/84	Croft	now can be used as 'startup' appl; allow
 *			launch from other drive.
 */

#include "mac/quickdraw.h"
#include "mac/osintf.h"
#include "mac/toolintf.h"
#include "mac/packintf.h"

#define NIL 0
#define	FINDERNAME 0x2e0

short	where[] = { 40, 40 };
char	types[] = "APPL";
char	myname[] = "mFinder";

struct launchparams {
	char	*lnamep;
	long	lunknown;
	short	lsound;
} lparams;

SFReply sfr;

main()
{
	struct QDVar newqd;
	char *old,*new;
	
	QD = &newqd;
	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(main);
	SetCursor(&newqd.arrow);

	/* install myself as new finder */
	strcpy(FINDERNAME, myname);
	c2pstr(FINDERNAME);
	SFGetFile((Point *)where, "",(ProcPtr)NIL, 1,types, (ProcPtr)NIL, &sfr);
	if (sfr.good)
		Launch(sfr.fName);

	/* If cancel was selected, deinstall myself and exit to real Finder. */
	strcpy(FINDERNAME, "Finder");
	c2pstr(FINDERNAME);
}


Launch(name)
	char *name;
{
	lparams.lnamep = name;
	SetVol((char *)0, sfr.vRefNum);
	asm("lea lparams,a0");
	asm(".word 0xa9f2");
	ExitToShell();	/* if Launch failed */
}

/* avoid loading entire package library */

asm(".globl _sfgetfil");
asm("_sfgetfil:");
asm("moveq #2,d2");
asm("movl sp@+,a0 |get user's rts");
asm("movw d2,sp@- |the routine selector");
asm("movl a0,sp@- |push the rts back on");
asm(".word 0xadea |__pack3 w/autopop");
