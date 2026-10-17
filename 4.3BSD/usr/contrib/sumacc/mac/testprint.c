/*
 * test print stuff
 */

#include "quickdraw.h"
#include "osintf.h"
#include "toolintf.h"
#include "macprint.h"

#define NIL 0

main()
   {
   struct QDVar theQD;
   THPrint hPrint;
   TPPrPort pPort;
   TPrStatus status;
   GrafPtr port;

   QD = &theQD;
   InitGraf(&thePort);
   InitFonts();
   InitWindows();
   TEInit();
   InitDialogs((ProcPtr) NIL);
   SetCursor(&theQD.arrow);

   PrOpen();

   hPrint = (THPrint) NewHandle(sizeof(TPrint));

   if (PrStlDialog(hPrint) && PrJobDialog(hPrint))
      {
      GetPort(&port);
      pPort = PrOpenDoc(hPrint, (TPPrPort) NIL, (Ptr) NIL);
      SetPort((GrafPtr) pPort);
      PrOpenPage(pPort, (Rect *) NIL);
      TextBox("some text", 9, &(*hPrint)->PrInfo.rPage, 0);
      PrClosePage(pPort);
      PrCloseDoc(pPort);

      if ((*hPrint)->PrJob.bJDocLoop == bSpoolLoop)
         PrPicFile(hPrint, (TPPrPort) NIL, (Ptr) NIL, (Ptr) NIL, &status);

      SetPort(port);
      }

   PrClose();

   ExitToShell();
   }
