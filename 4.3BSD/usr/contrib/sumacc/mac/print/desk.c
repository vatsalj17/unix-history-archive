/*
 * Desk accessory launcher/installer.
 *
 * (C) Copyright 1984 Michael Schuster
 * All Rights Reserved
 */

/*
 * 08/10/84	Schuster	Created
 * 08/21/84	Schuster	Search for unused resource id
 */

#include "quickdraw.h"
#include "osintf.h"
#include "toolintf.h"

#define NIL 0
#define FALSE 0
#define TRUE 1

#define minID 12
#define maxID 31

#define appleMenu 1
#define fileMenu 2
#define editMenu 3

#define closeItem 1
#define installItem 2
#define quitItem 3
#define undoItem 1
#define cutItem 3
#define copyItem 4
#define pasteItem 5
#define clearItem 6

MenuHandle menus[editMenu + 1];

main()
   {
   struct QDVar QDVar;
   EventRecord event;
   WindowPtr window;
   Rect dragRect;

   QD = &QDVar;
   InitGraf(&thePort);
   InitFonts();
   InitWindows();
   InitMenus();
   TEInit();

   dragRect = QD->screenBits.bounds; 

   menus[appleMenu] = NewMenu(appleMenu, "\24");
   AddResMenu(menus[appleMenu], "DRVR");
   InsertMenu(menus[appleMenu], 0);
   menus[fileMenu] = NewMenu(fileMenu, "File");
   AppendMenu(menus[fileMenu], "Close");
   AppendMenu(menus[fileMenu], "Install");
   AppendMenu(menus[fileMenu], "Quit");
   InsertMenu(menus[fileMenu], 0);
   menus[editMenu] = NewMenu(editMenu, "Edit");
   AppendMenu(menus[editMenu], "Undo/Z");
   AppendMenu(menus[editMenu], "(-");
   AppendMenu(menus[editMenu], "Cut/X");
   AppendMenu(menus[editMenu], "Copy/C");
   AppendMenu(menus[editMenu], "Paste/V");
   AppendMenu(menus[editMenu], "Clear/B");
   InsertMenu(menus[editMenu], 0);
   DrawMenuBar();

   SetCursor(&QD->arrow);
   while (TRUE)
      {
      SystemTask();
      if (FrontWindow())
         DisableItem(menus[fileMenu], installItem);
      else
         EnableItem(menus[fileMenu], installItem);

      if (!GetNextEvent(everyEvent, &event))
         continue;

      switch (event.what)
         {
         case mouseDown:
            switch (FindWindow(&event.where, &window))
               {
               case inMenuBar:
                  SetCursor(&QD->arrow);
                  commandEvent(MenuSelect(&event.where));
                  HiliteMenu(0);
                  break;

               case inSysWindow:
                  SystemClick(&event, window);
                  break;

               case inDrag:
                  DragWindow(window, &event.where, &dragRect);
                  break;

               case inGoAway:
                  if (TrackGoAway(window, &event.where))
                     commandEvent((fileMenu << 16) | closeItem);        
                  break;

               case inContent:
                  if (window != FrontWindow())
                     SelectWindow(window);
                  break;
               }
            break;

         case keyDown:
         case autoKey:
            if (event.modifiers & cmdKey)
               {
               commandEvent(MenuKey(event.message & 0xff));
               HiliteMenu(0);
               }
            break;
         }
      }
   }

commandEvent(menuItem)
   {
   WindowPtr window;
   int menu;
   int item;
   char name[64];
   char *c2pnstr();

   window = FrontWindow();
   SetPort(window);
   menu = HiWord(menuItem);
   item = LoWord(menuItem);

   switch (menu)
      {
      case appleMenu:
         GetItem(menus[appleMenu], item, name);
         OpenDeskAcc(c2pnstr(name));
         break;

      case fileMenu:
         switch (item)
            {
            case closeItem:
               CloseDeskAcc(((WindowPeek) window)->windowKind);
               break;

            case installItem:
               install();
               break;

            case quitItem:
               ExitToShell();
               break;    
            }
         break;

      case editMenu:
         switch (item)
            {
            case undoItem:
               SystemEdit(undoCmd);
               break;

            case cutItem:
               SystemEdit(cutCmd);
               break;

            case copyItem:
               SystemEdit(copyCmd);
               break;

            case pasteItem:
               SystemEdit(pasteCmd);
               break;

            case clearItem:
               SystemEdit(clearCmd);
               break;      
            }
         break;
      }
   }

/* install local drivers as system drivers */
install()
   {
   int i;
   int j;
   int k;
   int rsrc;
   Handle oldHandle;
   Handle handle;
   Handle drivers[16];
   ResType type;
   int id;
   int newId;
   int oldId;
   int attrs;
   char name[256];

   j = CountResources("DRVR");
   k = 0;
   rsrc = CurResFile();
   SetResLoad(FALSE);
   for (i = 1; i <= j; i++)
      {
      handle = GetIndResource("DRVR", i);
      if (handle)
         {
         ReleaseResource(handle);
         handle = GetIndResource("DRVR", i);
         }
      if (handle && HomeResFile(handle) == rsrc)
         drivers[++k] = handle;
      }
   SetResLoad(TRUE);

   for (i = 1; i <= k; i++)
      {
      UseResFile(rsrc);
      LoadResource(drivers[i]);
      handle = NewHandle(GetHandleSize(drivers[i]));
      if (!MemError())
         {
         BlockMove(*drivers[i], *handle, GetHandleSize(handle));
         GetResInfo(drivers[i], &id, type.s, name);
         attrs = GetResAttrs(drivers[i]);
         ReleaseResource(drivers[i]);
         UseResFile(0);

         if (oldHandle = GetNamedResource("DRVR", name))
            {
            GetResInfo(oldHandle, &oldId, type.s, name);
            UseResFile(rsrc);
            if (installAlert(name, "DRVR", oldId) == 2)
               continue;
            UseResFile(0);
            RmveResource(oldHandle);
            DisposHandle(oldHandle);
            id = oldId;
            }
         else
            {
	    SetResLoad(FALSE);
	    newId = minID;
	    while (newId <= maxID && (oldHandle = GetResource("DRVR", newId)))
	       {
	       ReleaseResource(oldHandle);
	       newId++;
	       }
	    SetResLoad(TRUE);
	    id = (newId <= maxID) ? newId : id;

	    if (oldHandle = GetResource("DRVR", id))
	       {
	       UseResFile(rsrc);
	       if (installAlert(name, "DRVR", id) == 2)
		  continue;
	       UseResFile(0);
	       RmveResource(oldHandle);
	       DisposHandle(oldHandle);
	       }
            }

         AddResource(handle, "DRVR", id, name);
         if (!ResError())
            {
            WriteResource(handle);
            SetResAttrs(handle, attrs);
            }
         }
      else
         ReleaseResource(drivers[i]);
      }
   UpdateResFile(0);
   UseResFile(rsrc);
   }

/* install alert */
installAlert(name, type, id)
   char *name;
   char *type;
   int id;
   {
   char idstring[16];

   NumToString(id, idstring);
   ParamText(name, type, idstring, "");
   return CautionAlert(256, (ProcPtr) NIL);
   }

/* convert a C string to a Pascal string with a leading NUL character */
char *c2pnstr(s)
   char *s;
   {
   extern char *isapstr();
   char *t;
   int i;
   int j;

   i = 0;
   while (*s++)
     i++;
   j = i;
   t = s - 2;
   while (j--)
      *s-- = *t--;
   *s-- = 0;
   *s = (char) i + 1;
   return(isapstr(s));
   }

