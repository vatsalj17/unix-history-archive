/*
 * Print desk accessory
 *
 * (C) Copyright 1984 Michael Schuster
 * All Rights Reserved
 *
 */

/*
 * 08/21/84	Schuster	Created
 */

#include "quickdraw.h"
#include "osintf.h"
#include "toolintf.h"
#include "packintf.h"
#include "device.h"

#define NIL 0
#define FALSE 0
#define TRUE 1

#define pageLen 62
#define topOfForm pChar('\014')
#define topSpace pChar('\n'); pChar('\n')

typedef struct
   {
   int volumn;			/* file volumn */
   char name[64];		/* file name */
   } File;

#define queueLen 8
File queue[queueLen];		/* file queue */

int indent;			/* indentation counter for tab handler */
int lines;			/* line counter for top of form handler */
#define bufferLen 64
char buffer[bufferLen];		/* io buffer */

int port;			/* printer port ID */
int file;			/* file ID */
int open;			/* TRUE if file open */

int inSFGetFile;		/* TRUE if SFGetFile dialog active */
   
/*
 * file routines
 */

/*
 * put a file into the queue
 */
putFile(name, length, volumn)
   char *name;
   {
   register int i;

   for (i = 0; i < queueLen; i++)
      {
      if (!*queue[i].name)
         {
	 BlockMove(name, queue[i].name, length);
         queue[i].name[length] = '\0';
	 queue[i].volumn = volumn;
	 return;
	 }
      }
   }

/*
 * get a file from the queue, if any
 */
getFile()
   {
   register int i;

   if (open)
      return TRUE;
   else
      {
      for (i = 0; i < queueLen; i++)
	 {
	 if (*queue[i].name)
	    {
	    open = 
               (FSOpen(queue[i].name, queue[i].volumn, &file)) ? FALSE : TRUE;
	    if (open)
	       {
	       pChar('\033');		/* software reset */
	       pChar('c');
	       topSpace;
	       }
            *queue[i].name = 0;
	    lines = indent = 0;
	    return open;
	    }
	 }
      return FALSE;
      }
   }

/*
 * close the open file and optionally purge all files from the queue
 */
closeFile(purge)
   {
   register int i;

   if (open)
      {
      topOfForm;
      FSClose(file);
      }
   open = FALSE;

   if (purge)
      for (i = 0; i < queueLen; i++)
	 *queue[i].name = 0;
   }

/*
 * driver routines
 */

/*
 * driver open routine
 */
drvrOpen(pb, dce)
   CntrlParam *pb;
   DCtlEntry *dce;
   {
   Point point;
   SFReply sfReply;
   SFTypeList sfTypeList;

   FSOpen(".BOut", 0, &port);
   SetPt(&point, 50, 50);
   inSFGetFile = TRUE;
   BlockMove("TEXT", sfTypeList.ftype[0].s, 4);
   SFGetFile(&point,"",(ProcPtr)NIL,1,(Ptr)&sfTypeList,(ProcPtr)NIL,&sfReply);
   inSFGetFile = FALSE;
   if (sfReply.good)
      putFile(&sfReply.fName[1], sfReply.fName[0], sfReply.vRefNum);
   return(ioRTS);
   }

/*
 * driver close routine
 */
drvrClose(pb, dce)
   CntrlParam *pb;
   DCtlEntry *dce;
   {
   closeFile(FALSE);
   return(ioRTS);
   }

/*
 * driver control routine
 */
drvrCtl(pb, dce)
   CntrlParam *pb;
   DCtlEntry *dce;
   {
   int chars;

   switch (pb->csCode)
      {
      case accRun:
         if (inSFGetFile)
            ;
	 else if (getFile() && !portHeldUp())
            {
            chars = bufferLen;
	    FSRead(file, &chars, buffer);
	    if (chars)
	       pText(chars, buffer);
	    else
	       closeFile(FALSE);
	    }
         else
            ; /* CloseDeskAcc(dce->dCtlRefNum); */
         break;
      }
   return(ioRTS);
   }

/*
 * return TRUE if port held up
 */
portHeldUp()
   {
   CSParam param;

   Status(port, 8, &param);
   return param.serStaRec.ctsHold;
   }

/*
 * write char to port
 */
pChar(c)
   char c;
   {
   int chars;

   chars = 1;
   FSWrite(port, &chars, &c);
   }

/*
 * write text to port
 */
pText(chars, buffer)
   char *buffer;
   {
   register char *b;
   register char *p;
   register char *e;

   b = p = e = buffer;
   e += chars;
   while (p < e)
      {
      if (*p == '\r')
         {
	 *p++ = '\n';
         indent = 0;
	 lines++;
	 if (!(lines % pageLen))
	    {
	    chars = p - b;
	    FSWrite(port, &chars, b);
	    b = p;
	    topOfForm;
	    topSpace;
	    }
         }
      else if (*p == '\t')
         {
         chars = p - b;
	 FSWrite(port, &chars, b);
	 b = ++p;
	 chars = 8 - (indent % 8);
	 FSWrite(port, &chars, "        ");
	 indent += chars;
         }
      else if (*p < '\040' || *p > '\176')
         {
	 *p++ = ' ';
	 indent++;
	 }
      else
	 {
         p++;
	 indent++;
	 }
      }
   chars = p - b;
   FSWrite(port, &chars, b);
   }

/*
 * driver status routine
 */
drvrStatus(pb, dce)
   CntrlParam *pb;
   DCtlEntry *dce;
   {
   return(ioRTS);
   }

/*
 * driver prime routine
 */
drvrPrime(pb, dce)
   CntrlParam *pb;
   DCtlEntry *dce;
   {
   return(ioRTS);
   }
