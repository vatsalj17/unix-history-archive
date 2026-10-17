/*
 * Boot - Reboot the Mac.  Put up a Modal dialog box, ask for confirmation,
 * flush and eject the mounted volumes (won't eject newly inserted disks)
 * reset the machine, and jump to the ROM boot address.  Best as a startup
 * application, don't you think?
 *
 * Bill Schilit, November 7, 1984.
 *
 */

#include "mac/quickdraw.h"
#include "mac/osintf.h"
#include "mac/toolintf.h"

#define NIL 0
#define NILPROC (ProcPtr) NIL
#define NILPTR (Ptr) NIL
#define FALSE 0
#define TRUE 1

struct QDVar QDVar;

/*
 * ejectdisk - given a drive number, flush the volume and eject.
 *
 */

ejectdisk(dn)
int dn;
{
 char volname[255];
 VolumeParam vparam;
 int err;

 vparam.ioNamePtr = volname;		/* return name here */
 vparam.ioVolIndex = 0;			/* no volume index */
 vparam.ioVRefNum = dn;			/* our drive number */
 err = PBGetVInfo(&vparam,FALSE);	/* do it synchronously */
 if (err != noErr)			/* some error? */
  return;				/* probably nothing there... */
 err = Eject(NILPTR,vparam.ioVRefNum);	/* otherwise eject the disk... */
}

/*
 * reboot - use this newly learned feature of C which allows inline
 * assembler code.  Do a hardware reset (seems to prevent oddness
 * later on) and then jump to the reboot address.
 *
 */

Reboot()
{
 ejectdisk(2);				/* and disk 2 */    
 ejectdisk(1);				/* and disk 1 */  
 asm("	reset");			/* reset the machine */
 asm("	jmp /40000A");			/* boot address */
}

main()
{
 DialogPtr theDialog;
 int itemHit;

 QD = &QDVar;				/* setup all... */
 InitGraf(&thePort);
 InitWindows();
 InitFonts();
 InitDialogs(NILPROC);
 TEInit();

 theDialog = GetNewDialog(257,NILPTR,(Ptr) -1);	/* from rsrc file */
 ShowWindow(theDialog);			/* here is the dialog box */
 InitCursor();				/* show our cursor */
 
 do					/* do modal dialog */
 {
  ModalDialog(NILPROC,&itemHit);	/* for each event, no filter */
  if (itemHit == OK)			/* hit the OK to boot button? */
   Reboot();				/* yes, then do it... */
 } while (itemHit != Cancel);		/* else, exit if Cancel hit */
 ExitToShell();				/* ... back to the Finder */
}

