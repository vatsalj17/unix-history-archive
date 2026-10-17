/*
 * Calendar desk accessory
 *
 * (C) Copyright 1984 Michael Schuster
 * All Rights Reserved
 */

#include "quickdraw.h"
#include "osintf.h"
#include "toolintf.h"
#include "device.h"
#include "res.h"

#define NIL 0
#define FALSE 0
#define TRUE 1

/*
 * string constants
 */
char TITLE[] = {"Calendar"};    /* calendar window title */
char FILE[] = {"Calendar File"};/* calendar file title */
char TEXT[] = {"TEXT"};         /* calendar resource type */

char *dayNames[] =              /* names of week days */
   {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", 
   "Friday", "Saturday", "Sunday"};

char *monthNames[] =            /* names of months */
   {"January", "Feburary", "March", "April", "May", "June",
   "July", "August", "September", "October", "November", "December"};
/*
 * integer constants
 */
long blackPat[] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
long grayPat[] = {0xaa55aa55, 0xaa55aa55, 0xaa55aa55, 0xaa55aa55};

int mLength[] =                 /* lengths of months (non-leap year) */
   {31,31,28,31,30,31,30,31,31,30,31,30,31,31};

/*
 * font constants
 */
#define textFont geneva         /* text font */
#define calFont monaco          /* calendar font */
#define textSize 9              /* font size */
#define lines 13                /* number of lines of text */

int ascent;                     /* character ascent */
int descent;                    /* character descent */
int extent;                     /* character extent (width) */
int indent;                     /* horizontal indentation */
int leading;                    /* vertical spacing */
int topping;                    /* vertical indentation */

/*
 * global variables
 */
WindowPtr window;               /* calendar window */
#define text ((TEHandle) GetWRefCon(window))    /* calendar text */
int rsrc;                       /* calendar resource file */
int tickTime;                   /* tick to draw time */
int undoCommand;                /* command to undo */
Handle resources[32];           /* day resources */

#define undoCutCmd 7
#define undoCopyCmd 8
#define undoPasteCmd 9
#define undoClearCmd 10
#define typeCmd 11
#define undoTypeCmd 12

/*
 * date and time variables
 */
DateTimeRec date;                 /* current date */
DateTimeRec time;                 /* current time */

#define dateVal(d,i) ((short *)d)[DIVal[i]]
int DIVal[] = {1, 2, 0};

/*
 * scrap variables
 */
typedef struct
   {
   short length;
   short filler;
   Handle handle;
   } ScrapRec;
typedef ScrapRec *ScrapPtr;
#define TEScrapAddr 2736        /* address of text edit scrap */

ScrapPtr TEScrap;               /* text edit scrap */
Handle UDScrap;                 /* undo scrap */
int UDSel;                      /* undo scrap selection length */
int UDChars;                    /* undo scrap character count */

/*
 * global regions
 */
Point cellP;                    /* size of calendar cell */
Rect windowR;                   /* calendar window bounds */
Rect titleR;                    /* title bounds */
Rect dayR;                      /* day bounds */
Rect calendarR;                 /* calendar bounds */
Rect monthR;                    /* month bounds */
Rect yearR;                     /* year bounds */
Rect textR;                     /* text bounds */
Rect textClickR;                /* text click bounds */

/*
 * string variables
 */
char blanks[] = {"          "};
char zeros[] = {"0000000000"};
#define strLen 5
char *numToString();
char *index();
char *unParseDate();

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
   if (!dce->dCtlWindow)
      {
      initRegion();
      newData(dce);
      initScrap();
      initDate();
      openResFile();
      }
   return(ioRTS);
   }

/* 
 * driver close routine 
 */
drvrClose(pb, dce)
   CntrlParam *pb;
   DCtlEntry *dce;
   {
   closeResFile();
   disposeData(dce);
   return(ioRTS);
   }

/* 
 * driver control routine 
 */
drvrCtl(pb, dce)
   CntrlParam *pb;
   DCtlEntry *dce;
   {
   SetPort(window);
   switch(pb->csCode)
      {
      case accEvent:
         handleEvent((EventRecord *) pb->csParam.csLong);
         break;

      case accRun:
         TEIdle(text);
         if (TickCount() > tickTime)
            drawTime();
         break;

      case accUndo:
         commandEvent(undoCommand);
         break;
      
      case accCut:
         commandEvent(cutCmd);
         break;

      case accCopy:
         commandEvent(copyCmd);
         break;

      case accPaste:
         commandEvent(pasteCmd);
         break;

      case accClear:
         commandEvent(clearCmd);
         break;
      }
   return(ioRTS);
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

/*
 * resource routines
 */

/*
 * (create and) open resource file
 */
openResFile()
   {
   FInfo fInfo;

   if ((rsrc = OpenResFile(FILE)) < 0)
      {
      /* create and set file info */
      CreateResFile(FILE);
      GetFInfo(FILE, 0, &fInfo);
      BlockMove("ZSYS", fInfo.fdType.s, 4);
      BlockMove("MACS", fInfo.fdCreator.s, 4);
      SetFInfo(FILE, 0, &fInfo);

      rsrc = OpenResFile(FILE);
      }

   /* load month and date resources */
   loadMonthRes();
   loadDateRes();
   }

/* 
 * close resource file 
 */
closeResFile()
   {
   unLoadDateRes();
   CloseResFile(rsrc);
   }

/*
 * load month resources
 */
loadMonthRes()
   {
   register Handle handle;
   register int i;
   int j;
   int id;
   ResType type;
   DateTimeRec resDate;
   char name[64];

   /* initialize handles */
   for (i = 1; i <= 31; i++)
      resources[i] = NIL;

   /* find resource handles for current month, don't load resources */
   j = CountResources(TEXT);
   SetResLoad(FALSE);
   for (i = 1; i <= j; i++)
      {
      if ((handle = GetIndResource(TEXT, i)) && 
          (HomeResFile(handle) == rsrc))
         {
         GetResInfo(handle, &id, type.s, name);
         parseDate(name, &resDate);
         if (sameMonth(&date, &resDate))
            resources[resDate.day] = handle;
         }
      }
   SetResLoad(TRUE);
   }

/*
 * load resource for current date into text edit record
 */
loadDateRes()
   {
   register Handle handle;
   register int length;
   char name[64];

   /* add resource if necessary */
   if (!(handle = resources[date.day]))
      {
      handle = resources[date.day] = NewHandle(0);
      AddResource(handle, TEXT, UniqueID(TEXT), unParseDate(name, &date));
      SetResAttrs(handle, GetResAttrs(handle) | ATT_PURGEABLE);
      }

   /* load resource and make unpurgeable */
   LoadResource(handle);
   HNoPurge(handle);

   /* load text edit record */
   TEDeactivate(text);
   (*text)->hText = handle;
   (*text)->teLength = length = GetHandleSize(handle);
   TECalText(text);
   TESetSelect(length, length, text);
   TEActivate(text);
   }

/*
 * unload resource for current date
 */
unLoadDateRes()
   {
   register Handle handle;

   handle = resources[date.day];
   if (!(*text)->teLength)
      {
      /* remove resource if no text */
      RmveResource(handle);
      DisposHandle(handle);
      resources[date.day] = NIL;
      }
   else
      {
      /* write resource if some text and make purgable */
      if (GetResAttrs(handle) & ATT_CHANGED)
         WriteResource(handle);
      HPurge(handle);
      }
   }

/*
 * change resource for current date
 */
changeDateRes()
   {
   ChangedResource(resources[date.day]);
   }

/*
 * data structure routines
 */

/*
 * new data structures
 */
newData(dce)
   DCtlEntry *dce;
   {
   /* create window */
   OffsetRect(&windowR, 40, 50);
   window = NewWindow((WindowPeek) NIL, &windowR, TITLE, TRUE, 
      rDocProc, (WindowPtr) -1, TRUE, 0);
   OffsetRect(&windowR, -40, -50);
   dce->dCtlWindow = (Ptr) window;

   /* define font and create text edit record */
   SetPort(window);
   TextFont(textFont);
   TextSize(textSize);
   ((WindowPeek) window)->windowKind = dce->dCtlRefNum;
   SetWRefCon(window, (int) TENew(&textR, &textR));
   TextFont(calFont);
   }

/* 
 * dispose data structures 
 */
disposeData(dce)
   DCtlEntry *dce;
   {
   /* dispose text edit resord, make sure resource is not disposed */
   (*text)->hText = NewHandle(0);
   TEDispose(text);

   /* dispose window */
   DisposeWindow(window);
   dce->dCtlWindow = NIL;
   }

/*
 * scrap routines
 */

/*
 * initialize text and undo scraps
 */
initScrap()
   {
   TEScrap = (ScrapPtr) TEScrapAddr;
   UDScrap = NewHandle(0);
   undoCommand = undoCmd;
   }

/*
 * transfer desk scrap to text scrap
 */
getScrap()
   {
   int offset;

   SetHandleSize(TEScrap->handle, TEScrap->length = 0);
   if ((TEScrap->length = GetScrap(TEScrap->handle, TEXT, &offset)) < 0)
      SetHandleSize(TEScrap->handle, TEScrap->length = 0);
   }

/*
 * transfer text scrap to desk scrap
 */
putScrap()
   {
   if (!ZeroScrap())
      PutScrap(TEScrap->length, TEXT, *TEScrap->handle);
   }

/*
 * transfer text scrap, selection, or to end of selection to undo scrap
 */
putUndo(scrap)
   {
   if (!scrap)
      {
      /* transfer text scrap */
      SetHandleSize(UDScrap, TEScrap->length);
      if (MemError())
         SetHandleSize(UDScrap, 0);
      else
         BlockMove(*TEScrap->handle, *UDScrap, GetHandleSize(UDScrap));
      }
   else if (scrap > 0)
      {
      /* transfer selection */
      SetHandleSize(UDScrap, (*text)->selEnd - (*text)->selStart);
      if (MemError())
         SetHandleSize(UDScrap, 0);
      else
         BlockMove(*(*text)->hText + (*text)->selStart, *UDScrap, 
            GetHandleSize(UDScrap));
      }
   else 
      {
      /* transfer to end of selection */
      SetHandleSize(UDScrap, (*text)->selEnd);
      if (MemError())
         SetHandleSize(UDScrap, 0);
      else
         BlockMove(*(*text)->hText, *UDScrap, GetHandleSize(UDScrap));
      UDSel = (*text)->selEnd - (*text)->selStart;
      UDChars = GetHandleSize(UDScrap) - UDSel;
      }
   }

/*
 * transfer undo scrap to text scrap
 */
getUndo()
   {
   DisposHandle(TEScrap->handle);
   TEScrap->length = GetHandleSize(UDScrap);
   TEScrap->handle = UDScrap;
   UDScrap= NewHandle(0);
   }

/* 
 * date routines
 */

/*
 * initialize current date
 */
initDate()
   {
   GetTime(&date);
   }

/*
 * return day of week of the first day of the month
 */
dayOfWeek1(date)
   DateTimeRec *date;
   {
   return (date->dayOfWeek - (date->day - 1) % 7 + 6) % 7;
   }

/*
 * return length of month
 */
monthLength(date)
   DateTimeRec *date;
   {
   if (date->month == 2 &&
       ((date->year % 4 == 0 && date->year % 100 != 0) ||
        date->year % 400 == 0))
      return 29;
   else
      return mLength[date->month];
   }

/*
 * return TRUE if same month
 */
sameMonth(a, b)
   DateTimeRec *a;
   DateTimeRec *b;
   {
   return (a->year == b->year && a->month == b->month);
   }

/* 
 * parse date string "month/day/year"
 */
parseDate(name, date)
   char *name;
   DateTimeRec *date;
   {
   register char *q;
   register char *p;
   register int i;
   int val;

   q = name;
   for (i = 0; i < 3; i++)
      {
      p = index(q, '/');
      *p++ = 0;
      StringToNum(q, &val);
      dateVal(date,i) = val;
      q = p;
      }
   }

/* 
 * unparse date string "month/day/year"
 */
char *unParseDate(name, date)
   char *name;
   DateTimeRec *date;
   {
   register char *p;
   register int i;
   int val;

   p = name;
   for (i = 0; i < 3; i++)
      {
      val = dateVal(date,i);
      NumToString(val, p);
      p += strlen(p);
      *p++ = '/';
      }
   *--p = '\0';

   return name;
   }

/*
 * return number of days to the same day in a given month in the current year
 */
monthDelta(m)
   {
   register int delta;
   int curMonth;

   delta = 1 - date.day;
   curMonth = date.month;
   while (m != date.month)
      {
      if (m > date.month)
         {
         delta += monthLength(&date);
         date.month++;
         }
      else
         {
         date.month--;
         delta -= monthLength(&date);
         }
      }
   delta += min(date.day, monthLength(&date)) - 1;
   date.month = curMonth;
   return delta;
   }

/*
 * return number of days to the same day in the same month of a given year
 */
yearDelta(y)
   {
   register int delta;
   register int i;
   int curYear;

   delta = 1 - date.day;
   curYear = date.year;
   while (y != date.year)
      {
      if (y > date.year)
         {
         for (i = 0; i < 12; i++)
            {
            delta += monthLength(&date);
            if (++date.month > 12)
               {
               date.month = 1;
               date.year++;
               }
            }
         }
      else
         {
         for (i = 0; i < 12; i++)
            {
            if (--date.month < 1)
               {
               date.month = 12;
               date.year--;
               }
            delta -= monthLength(&date);
            }
         }
      }
   delta += min(date.day, monthLength(&date)) - 1;
   date.year = curYear;
   return delta;
   }

/*
 * change current date forward or backward delta days
 */
changeDate(delta)
   {
   int mChanged;

   /* unload current date */
   unLoadDateRes();

   /* fix day of week */
   date.dayOfWeek = (date.dayOfWeek + delta + 1091) % 7 + 1;

   /* change date, check if month had to be changed */
   mChanged = 0;
   while (delta)
      {
      if ((date.day + delta <= monthLength(&date)) &&
          (date.day + delta >= 1))
         {
         date.day += delta;
         delta = 0;
         }
      else if (delta > 0)
         {
         delta -= monthLength(&date) - date.day + 1;
         date.day = 1;
         if (++date.month > 12)
            {
            date.month = 1;
            date.year++;
            }
         mChanged = 1;
         }
      else
         {
         delta += date.day;
         if (--date.month < 1)
            {
            date.month = 12;
            date.year--;
            }
         date.day = monthLength(&date);
         mChanged = 1;
         }
      }

   /* load month if necessary */
   if (mChanged)
      loadMonthRes();

   /* load new date */
   loadDateRes();
   }

/*
 * region routines
 */

/*
 * initialize regions
 */
initRegion()
   {
   /* Monaco font information */
   ascent = 7;
   descent = 2;
   extent = 6;
   indent = 4;
   leading = 3;
   topping = 3;

   /* define sizes of regions */
   SetPt(&cellP, extent * 3 + indent * 2, topping * 2 + ascent + 1);
   SetRect(&titleR, 0, 0, 0, topping * 4 + ascent);
   SetRect(&dayR, 0, 0, 7 * cellP.h, cellP.v);
   SetRect(&calendarR, 0, 0, 7 * cellP.h, 6 * cellP.v);
   SetRect(&monthR, 0, 0, 2 * cellP.h, 6 * cellP.v);
   SetRect(&yearR, 0, 0, 2 * cellP.h, cellP.v);

   /* place regions relative to one another */
   OffsetRect(&dayR, extent, titleR.bottom);
   OffsetRect(&calendarR, extent, dayR.bottom);
   OffsetRect(&monthR, calendarR.right + extent + 2, titleR.bottom);
   OffsetRect(&yearR, calendarR.right + extent + 2, monthR.bottom);
   titleR.right = monthR.right + extent + 1;

   /* define text edit regions */
   SetRect(&textR, 0, 0, ((titleR.right / extent) - 2) * extent + 1,
      (ascent + descent + leading) * lines);
   OffsetRect(&textR, (titleR.right - textR.right) / 2, 
      calendarR.bottom + extent);

   /* finish up */
   SetRect(&windowR, 0, 0, titleR.right, textR.bottom + extent);
   SetRect(&textClickR, 0, calendarR.bottom + 1, titleR.right, windowR.bottom);
   }

/*
 * event routines
 */

/*
 * handle event
 */
handleEvent(event)
   EventRecord *event;
   {
   char chr;

   switch(event->what)
      {
      case activateEvt:
         activateEvent(event->modifiers & activeFlag);
         break;
      
      case updateEvt:
         updateEvent();
         break;
      
      case mouseDown:
         GlobalToLocal(&event->where);
         mouseEvent(&event->where, event->modifiers);
         break;
      
      case keyDown:
      case autoKey:
         chr = event->message & 0xff;
         if (event->modifiers & cmdKey)
            {
            if (chr == 'z' || chr == 'Z')
               commandEvent(undoCommand);
            else if (chr == 'x' || chr == 'X')
               commandEvent(cutCmd);
            else if (chr == 'c' || chr == 'C')
               commandEvent(copyCmd);
            else if (chr == 'v' || chr == 'V')
               commandEvent(pasteCmd);
            else if (chr == 'b' || chr == 'B')
               commandEvent(clearCmd);
            }
         else
            keyDownEvent(chr);
         break;
      }
   }

/*
 * handle activate event
 */
activateEvent(active)
   {
   if (active)
      {
      getScrap();
      TEActivate(text);
      }
   else
      {
      TEDeactivate(text);
      putScrap();
      }
   hiliteRect(&calendarR, &cellP, 7, 6, date.day + dayOfWeek1(&date) - 1);
   undoCommand = undoCmd;
   }

/*
 * handle update event
 */
updateEvent()
   {
   BeginUpdate(window);
   if ((*text)->active)
      hiliteRect(&calendarR, &cellP, 7, 6, date.day + dayOfWeek1(&date) - 1);
   drawDate();
   drawTime();
   frameRect(&dayR, &cellP, 7, 1, (Pattern *) grayPat);
   drawTextRect(&dayR, &cellP, 7, 1, dayNames, 3);
   frameRect(&calendarR, &cellP, 7, 6, (Pattern *) grayPat);
   drawNumRect(&calendarR, &cellP, 7, 6, 
      dayOfWeek1(&date), 1, monthLength(&date), 1, resources);
   frameRect(&monthR, &cellP, 2, 6, (Pattern *) grayPat);
   drawTextRect(&monthR, &cellP, 2, 6, monthNames, 3);
   frameRect(&yearR, &cellP, 2, 1, (Pattern *) grayPat);
   drawNumRect(&yearR, &cellP, 2, 1, 0, 
      date.year - 1, date.year + 1, 2, (Handle *) NIL);
   drawText();
   if ((*text)->active)
      hiliteRect(&calendarR, &cellP, 7, 6, date.day + dayOfWeek1(&date) - 1);
   EndUpdate(window);
   }

/*
 * handle date event, change date either delta days or to today
 */
dateEvent(delta, today)
   {
   if (delta || today)
      {
      hiliteRect(&calendarR, &cellP, 7, 6, date.day + dayOfWeek1(&date) - 1);
      if (today)
         {
         unLoadDateRes();
         GetTime(&date);
         loadMonthRes();
         loadDateRes();
         }
      else
         changeDate(delta);
      drawDate();
      drawNumRect(&calendarR, &cellP, 7, 6, 
         dayOfWeek1(&date), 1, monthLength(&date), 1, resources);
      drawNumRect(&yearR, &cellP, 2, 1, 0, 
         date.year - 1, date.year + 1, 2, (Handle *) NIL);
      drawText();
      hiliteRect(&calendarR, &cellP, 7, 6, date.day + dayOfWeek1(&date) - 1);
      }
   }

/*
 * handle mouse event
 */
mouseEvent(point, modifiers)
   Point *point;
   {
   int delta;

   if (PtInRect(point, &textClickR))
      TEClick(point, (modifiers & shiftKey) ? TRUE : FALSE, text);
   else if (PtInRect(point, &titleR))
      {
      /* today */
      delta = mouseRect(&titleR, &titleR.botRight, point, 1, 1, TRUE);
      if (delta >= 0)
         dateEvent(0, TRUE);
      }
   else if (PtInRect(point, &dayR))
      {
      /* same week */
      delta = mouseRect(&dayR, &cellP, point, 7, 1, TRUE);
      if (delta >= 0)
         dateEvent(delta + 1 - date.dayOfWeek, FALSE);
      }
   else if (PtInRect(point, &calendarR))
      {
      /* same month */
      delta = mouseRect(&calendarR, &cellP, point, 7, 6, TRUE);
      if (delta >= 0)
         dateEvent(delta + 1 - dayOfWeek1(&date) - date.day, FALSE);
      }
   else if (PtInRect(point, &monthR))
      {
      /* another month */
      delta = mouseRect(&monthR, &cellP, point, 2, 6, FALSE);
      if (delta >= 0)
         dateEvent(monthDelta(delta + 1), FALSE);
      }
   else if (PtInRect(point, &yearR))
      {
      /* another year */
      delta = mouseRect(&yearR, &cellP, point, 2, 1, TRUE);
      if (delta >= 0)
         dateEvent(yearDelta((delta) ? date.year + 1 : date.year - 1), FALSE);
      }
   undoCommand = undoCmd;
   }

/*
 * handle key down event
 */
keyDownEvent(chr)
   {
   if (undoCommand != undoTypeCmd)
      putUndo(-TRUE);
   else if (chr == 8 && (*text)->selStart > 0)
      UDChars = min(UDChars, (*text)->selStart - 1);
   TEKey(chr, text);
   changeDateRes();
   undoCommand = undoTypeCmd;
   }

/*
 * handle command event
 */
commandEvent(cmd)
   {
   int i;

   switch (cmd)
      {
      case undoCmd:
         undoCommand = undoCmd;
         break;
      
      case cutCmd:
         putUndo(FALSE);
         TECut(text);
         putScrap();
         undoCommand = undoCutCmd;
         break;
      
      case undoCutCmd:
         TEDeactivate(text);
         TEPaste(text);
         TESetSelect((*text)->selStart-TEScrap->length, (*text)->selEnd, text);
         TEActivate(text);
         getUndo();
         putScrap();
         undoCommand = cutCmd;
         break;
      
      case copyCmd:
         putUndo(FALSE);
         TECopy(text);
         putScrap();
         undoCommand = undoCopyCmd;
         break;
      
      case undoCopyCmd:
         getUndo();
         putScrap();
         undoCommand = copyCmd;
         break;
      
      case pasteCmd:
         putUndo(TRUE);
         TEPaste(text);
         undoCommand = undoPasteCmd;
         break;
      
      case undoPasteCmd:
         TEDeactivate(text);
         TESetSelect((*text)->selStart-TEScrap->length, (*text)->selEnd, text);
         TEDelete(text);
         TEInsert(*UDScrap, GetHandleSize(UDScrap), text);
         TESetSelect((*text)->selStart - GetHandleSize(UDScrap), 
            (*text)->selEnd, text);
         SetHandleSize(UDScrap, 0);
         TEActivate(text);
         undoCommand = pasteCmd;
         break;

      case clearCmd:
         putUndo(TRUE);
         TEDelete(text);
         undoCommand = undoClearCmd;
         break;

      case undoClearCmd:
         TEDeactivate(text);
         TEInsert(*UDScrap, GetHandleSize(UDScrap), text);
         TESetSelect((*text)->selStart - GetHandleSize(UDScrap), 
            (*text)->selEnd, text);
         SetHandleSize(UDScrap, 0);
         TEActivate(text);
         undoCommand = clearCmd;
         break;
      
      case typeCmd:
         TEDeactivate(text);
         TESetSelect((*text)->selEnd, (*text)->selEnd, text);
         i = GetHandleSize(UDScrap);
         TEInsert(*UDScrap, i, text);
         TESetSelect(0, (*text)->selEnd - i, text);
         putUndo(TRUE);
         TESetSelect(UDChars, (*text)->selEnd, text);
         TEDelete(text);
         TESetSelect((*text)->selEnd + i, (*text)->selEnd + i, text);
         TEActivate(text);
         undoCommand = undoTypeCmd;
         break;

      case undoTypeCmd:
         TEDeactivate(text);
         TESetSelect(UDChars, (*text)->selEnd, text);
         TEInsert(*UDScrap + UDChars, GetHandleSize(UDScrap) - UDChars, text);
         putUndo(TRUE);
         TEDelete(text);
         TESetSelect((*text)->selEnd - UDSel, (*text)->selEnd, text);
         TEActivate(text);
         undoCommand = typeCmd;
         break;

      }
   changeDateRes();
   }

/*
 * drawing routines
 */

/*
 * frame rectangle, draw horizontal and vertical lines
 *    rect defines the rectangle
 *    point defines the size of each cell within the rectangle
 *    nh defines the number of columns
 *    nv defines the number of rows
 *    pat defines the pattern
 */
frameRect(rect, point, nh, nv, pat)
   Rect *rect;
   Point *point;
   Pattern *pat;
   {
   register int i;
   register d;

   PenPat(pat);
   /* draw horizontal lines */
   d = point->h * nh;
   MoveTo(rect->left, rect->top);
   for (i = 0; i <= nv; i++)
      {
      Line(d, 0);
      Move(-d, point->v);
      }

   /* draw vertical lines */
   d = point->v * nv;
   MoveTo(rect->left, rect->top);
   for (i = 0; i <= nh; i++)
      {
      Line(0, d);
      Move(point->h, -d);
      }
   PenPat((Pattern *) blackPat);
   }

/*
 * draw text in rectangle
 *    rect defines the rectangle
 *    point defines the size of each cell within the rectangle
 *    nh defines the number of columns
 *    nv defines the number of rows
 *    txt defines the text to be draw into each cell
 *    n defines the number of characters of text to draw
 */
drawTextRect(rect, point, nh, nv, txt, n)
   Rect *rect;
   Point *point;
   char **txt;
   {
   register int i;
   register int j;
   register int k;
   register int d;

   TextFont(calFont);
   MoveTo(rect->left + indent + 1 + ((3 - n) * extent) / 2, 
      rect->top + topping + ascent + 1);
   k = 0;
   d = n * extent;
   for (i = 0; i < nh; i++)
      {
      for (j = 0; j < nv; j++)
         {
         DrawText(txt[k++], 0, n);
         Move(-d, point->v);
         }
      Move(point->h, -(nv * point->v));
      }
   }

/*
 * draw number rectangle
 *    rect defines the rectangle
 *    point defines the size of each cell within the rectangle
 *    nh defines the number of columns
 *    nv defines the number of rows
 *    numbers min, min+delta, min+2*delta, ..., max are drawn into cells
 *    disp defines the initial number of cells to leave blank
 *    hilite defines which cell to hilite
 */
drawNumRect(rect, point, nh, nv, disp, min, max, delta, hilite)
   Rect *rect;
   Point *point;
   Handle *hilite;
   {
   Rect bounds;
   register int i;
   register int j;
   register int k;
   int d;
   Handle *hi;

   TextFont(calFont);
   SetRect(&bounds, 1, 1, point->h, point->v);
   OffsetRect(&bounds, rect->left, rect->top);
   MoveTo(rect->left + indent + 1 + extent / 2,
      rect->top + topping + ascent + 1);
   k = min - disp * delta;
   d = 2 * extent;
   hi = (hilite) ? &hilite[min] : NIL;
   for (i = 0; i < nv; i++)
      {
      for (j = 0; j < nh; j++)
         {
         EraseRect(&bounds);
         if (k >= min && k <= max)
            {
            TextFace((hi && *hi) ? outlineStyle : 0);
            if (hi && *hi)
               Move(-1, 0);
            DrawString(numToString(k, 2));
            if (hi && *hi)
               Move(-1, 0);
            Move(point->h - d, 0);
            (hi) ? hi++ : 0;
            }
         else
            Move(point->h, 0);
         k += delta;
         OffsetRect(&bounds, point->h, 0);
         }
      Move(-(nh * point->h), point->v);
      OffsetRect(&bounds, -(nh * point->h), point->v);
      }
   TextFace(0);
   }

/*
 * mouse rectangle, return cell number
 *    rect defines the rectangle
 *    point defines the size of each cell within the rectangle
 *    where defines the initial location of the mouse
 *    nh defines the number of columns
 *    nv defines the number of rows
 *    hori defines if cells are to be counted horizontally or vertically
 */
mouseRect(rect, point, where, nh, nv, hori)
   Rect *rect;
   Point *point;
   Point *where;
   {
   Rect bounds;
   int i;
   int j;
   int invert;

   SetRect(&bounds, 0, 0, point->h + 1, point->v + 1);
   OffsetRect(&bounds, 
      (i = (where->h - rect->left) / point->h) * point->h + rect->left,
      (j = (where->v - rect->top) / point->v) * point->v + rect->top);
   InvertRect(&bounds);
   invert = TRUE;
   while (StillDown())
      {
      GetMouse(where);
      if (invert != PtInRect(where, &bounds))
         {
         invert = TRUE - invert;
         InvertRect(&bounds);
         }
      }
   if (invert)
      {
      InvertRect(&bounds);
      return (hori) ? j * nh + i : i * nv + j;
      }
   else
      return -1;
   }

/*
 * hilite a cell in a rectangle
 *    rect defines the rectangle
 *    point defines the size of each cell within the rectangle
 *    nh defines the number of columns
 *    nv defines the number of rows
 *    n defines the number of the cell to hilite
 */
hiliteRect(rect, point, nh, nv, n)
   Rect *rect;
   Point *point;
   {
   Rect bounds;

   SetRect(&bounds, 1, 1, point->h, point->v);
   OffsetRect(&bounds, rect->left, rect->top);
   OffsetRect(&bounds, point->h * (n % nh), point->v * (n / nh));
   InvertRect(&bounds);
   }

/*
 * draw text
 */
drawText()
   {
   /* wasteful, but necessary if TECalText used on shorter text */
   EraseRect(&textR); 

   /* update */
   TextFont(textFont);
   TEUpdate(&textR, text);
   }

/*
 * draw date
 */
drawDate()
   {
   Rect bounds;

   TextFont(calFont);
   SetRect(&bounds, 0, 0, extent * 28, ascent + descent);
   OffsetRect(&bounds, extent, 2 * topping);
   EraseRect(&bounds);
   MoveTo(extent, 2 * topping + ascent);
   DrawText(dayNames[date.dayOfWeek - 1], 0, 
     strlen(dayNames[date.dayOfWeek - 1]));
   DrawChar(' ');
   DrawText(monthNames[date.month - 1], 0, 
      strlen(monthNames[date.month - 1]));
   DrawChar(' ');
   DrawString(numToString(date.day, 0));
   DrawChar(',');
   DrawString(numToString(date.year, 0));
   }

/*
 * draw time, set up tickTime for next minute update
 */
drawTime()
   {
   Rect bounds;

   TextFont(calFont);
   SetRect(&bounds, 0, 0, extent * 9, ascent + descent);
   OffsetRect(&bounds, titleR.right - 9 * extent + 1, 2 * topping);
   EraseRect(&bounds);
   MoveTo(titleR.right - 9 * extent + 1, 2 * topping + ascent);
   GetTime(&time);
   tickTime = TickCount() - time.second * 60 + 3600;
   DrawString(numToString((time.hour + 11) % 12 + 1, 2));
   DrawChar(':');
   DrawString(numToString(time.minute, -2));
   DrawChar(' ');
   DrawText("AMPM", (time.hour < 12) ? 0 : 2, 2);
   }

/*
 * utility routines
 */

/*
 * return minimum
 */
min(a, b)
   {
   return (a < b) ? a : b;
   }

/*
 * return length of string 
 */
strlen(s)
   char *s;
   {
   register char *p = s;
   register int n = 0;

   while (*p++)
      n++;
   return(n);
   }

/* 
 * return position of character 
 */
char *index(s, c)
   char *s;
   char c;
   {
   register char *p = s;
   register char n = c;

   while (*p && *p != n)
      p++;
   return(p);
   }

/*
 * convert number to string
 *    w == 0 -> no leading blanks or zeros
 *    w > 0  -> add leading blanks to make string length equal to w
 *    w < 0  -> add leading zeros to make string length equal to -w
 */
char *numToString(n, w)
   {
   if (!w)
      {
      NumToString(n, &blanks[strLen]);
      return &blanks[strLen];
      }
   else if (w > 0)
      {
      NumToString(n, &blanks[strLen]);
      return &blanks[strLen - w + strlen(&blanks[strLen])];
      }
   else
      {
      NumToString(n, &zeros[strLen]);
      return &zeros[strLen + w + strlen(&zeros[strLen])];
      }
   }
