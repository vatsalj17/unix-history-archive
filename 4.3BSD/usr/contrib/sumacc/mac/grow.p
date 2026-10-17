PROGRAM Grow;

{ Grow -- Scroll bars and a resizable window added to Edit }
{         by Cary Clark, Macintosh Technical Support       }

USES  {$U Obj:QuickDraw   } QuickDraw,
      {$U Obj:OSIntf      } OSIntf,
      {$U Obj:ToolIntf     } ToolIntf;

CONST lastMenu = 3;      { number of menus }
      appleMenu = 1;     { menu ID for desk accessory menu }
      fileMenu = 256;    { menu ID for File menu }
      editMenu = 257;    { menu ID for Edit menu }

VAR myMenus:  ARRAY [1..lastMenu] OF MenuHandle;
    growRect, dragRect, pRect, tRect:  Rect;
    doneFlag, temp:  BOOLEAN;
    myEvent:  EventRecord;
    code, refNum, MyControl, t:  INTEGER;
    wRecord:  WindowRecord;
    theWindow, whichWindow:  WindowPtr;
    theMenu, theItem:  INTEGER;
    theChar:  CHAR;
    ticks:  LongInt;
    hTE:  TEHandle;
    hCurs:  CursHandle;
    iBeam:  Cursor;
    hScroll, vScroll, whichControl :  ControlHandle;
    TheOrigin : point;

PROCEDURE SetUpMenus;
{ Once-only initialization for menus }
VAR i:  INTEGER;
    appleTitle:  STRING[1];
BEGIN
  InitMenus;   { initialize Menu Manager }
  appleTitle := ' '; appleTitle[1] := CHR(20);
  myMenus[1] := NewMenu(appleMenu,appleTitle);
  AddResMenu(myMenus[1],'DRVR');    { desk accessories }
  myMenus[2] := GetMenu(fileMenu);
  myMenus[3] := GetMenu(editMenu);
  FOR i:=1 TO lastMenu DO InsertMenu(myMenus[i],0);
  DrawMenuBar;
END;    { of SetUpMenus }

PROCEDURE CursorAdjust;
{ Makes cursor be I-beam inside the (active) application window's }
{ content region (except for size box and scroll bar areas). }
VAR mousePt:  Point;
BEGIN
  GetMouse(mousePt);
  IF theWindow = FrontWindow
    THEN
      BEGIN
        IF (PtInRect(mousePt,pRect))
          THEN SetCursor(iBeam)
          ELSE SetCursor(arrow);
      END;
END;

PROCEDURE DoCommand (mResult: LongInt);
VAR name:  STR255;
BEGIN
  theMenu := HiWord(mResult); theItem := LoWord(mResult);
  CASE theMenu OF

  appleMenu:
    BEGIN
      GetItem(myMenus[1],theItem,name);
      refNum := OpenDeskAcc(name);
    END;

  fileMenu:
    doneFlag := TRUE;    { Quit }

  editMenu:
    BEGIN
      IF NOT SystemEdit(theItem-1)
        THEN
          BEGIN
            SetPort(theWindow);
            ClipRect(pRect);

            { Delay so menu title will stay lit a little while if Command key }
            { equivalent was typed. }
            ticks := TickCount + 30;
            REPEAT UNTIL ticks <= TickCount;

            CASE theItem OF

            1:  TECut(hTE);

            2:  TECopy(hTE);

            3:  TEPaste(hTE);

            END;    { of item case }
          END;
    END;    { of editMenu }

  END;    { of menu case }
  HiliteMenu(0);

END;  { of DoCommand }

PROCEDURE MoveScrollBars;
BEGIN
  WITH theWindow^.portRect DO
     BEGIN
       HideControl(vScroll);
       MoveControl(vScroll,right-15,top-1);
       SizeControl(vScroll,16,bottom-top-13);
       ShowControl(vScroll);
       HideControl(hScroll);
       MoveControl(hScroll,left-1,bottom-15);
       SizeControl(hScroll,right-left-13,16);
       ShowControl(hScroll)
     END
END;

PROCEDURE ResizePRect;
BEGIN
  pRect := thePort^.portRect;
  pRect.left := pRect.left+4; pRect.right := pRect.right-15;
  pRect.bottom := pRect.bottom-15
END;

PROCEDURE GrowWnd (whichWindow: WindowPtr);
{ Handles growing and sizing the window and manipulating }
{ the update region. }
VAR longResult:  LongInt;
    height, width:  INTEGER;
    trect : rect;
BEGIN
  longResult := GrowWindow(whichWindow,myEvent.where,growRect);
  IF longResult = 0 THEN EXIT(GrowWnd);
  height := HiWord(longResult); width := LoWord(longResult);

  { Add the old "scroll bar area" to the update region so it will }
  { be redrawn (for when the window is enlarged). }
  tRect := whichWindow^.portRect; tRect.left := tRect.right-16;
  InvalRect(tRect);
  tRect := whichWindow^.portRect; tRect.top := tRect.bottom-16;
  InvalRect(tRect);

  { Now draw the newly sized window. }
  SizeWindow(whichWindow,width,height,TRUE);
  MoveScrollBars;
  ResizePRect;

  { Adjust the view rectangle for TextEdit. }
  hTE^^.viewRect := pRect;

  { Add the new "scroll bar area" to the update region so it will }
  { be redrawn (for when the window is made smaller). }
  tRect := whichWindow^.portRect; tRect.left := tRect.right-16;
  InvalRect(tRect);
  tRect := whichWindow^.portRect; tRect.top := tRect.bottom-16;
  InvalRect(tRect);
END;     { of GrowWnd }

PROCEDURE DrawWindow (whichWindow: WindowPtr);
{ Draws the content region of the given window, after erasing whatever }
{ was there before. }
VAR i:  INTEGER;
BEGIN
  ClipRect(whichWindow^.portRect);
  EraseRect(whichWindow^.portRect);
  DrawGrowIcon(whichWindow);
  DrawControls (whichWindow);
  TEUpdate(pRect,hTE)
END;    { of DrawWindow }

PROCEDURE ScrollBits;
VAR oldOrigin: Point;
    dh, dv : INTEGER;
BEGIN
  WITH TheWindow^ DO
    BEGIN
      oldOrigin := theOrigin;
      theOrigin.h := 4*GetCtlValue(hScroll);
      theOrigin.v := 4*GetCtlValue(vScroll);
      dh := oldOrigin.h - theOrigin.h;
      dv := oldOrigin.v - theOrigin.v;
      TEScroll (dh, dv, hTE)
    END
END;

PROCEDURE ScrollUp(whichControl:ControlHandle; theCode: INTEGER);
BEGIN
  IF theCode = inUpButton THEN
     BEGIN
       SetCtlValue(whichControl,GetCtlValue(whichControl)-1);
       ScrollBits
     END
END;

PROCEDURE ScrollDown(whichControl:ControlHandle; theCode: INTEGER);
BEGIN
  IF theCode = inDownButton THEN
     BEGIN
       SetCtlValue(whichControl,GetCtlValue(whichControl)+1);
       ScrollBits
     END
END;

PROCEDURE PageScroll(code, amount: INTEGER);
VAR myPt: Point;
BEGIN
  REPEAT
    GetMouse(myPt);
    IF TestControl(whichControl,myPt) = code THEN
       BEGIN
         SetCtlValue(whichControl,getCtlValue(whichControl)+amount);
         ScrollBits
       END
  UNTIL NOT StillDown;
END;

BEGIN    { main program }
  InitGraf(@thePort);
  InitFonts;
  FlushEvents(everyEvent,0);
  InitWindows;
  SetUpMenus;
  TEInit;
  InitDialogs(NIL);
  SetCursor(arrow);
  SetRect(dragRect,4,24,508,338);
  SetRect(growRect,100,60,512,302);
  doneFlag := FALSE;

  theWindow := GetNewWindow(256,@wRecord,POINTER(-1));
  SetPort(theWindow);
  theWindow^.txFont := 2;

  ResizePrect;
  hTE := TENew(pRect,pRect);
  hCurs := POINTER(ORD(GetCursor(256))); iBeam := hCurs^^;

  vScroll :=   GetNewControl(256,TheWindow);
  hScroll :=   GetNewControl(257,TheWindow);
  theOrigin.h := 0; theOrigin.v := 0;

  REPEAT
    CursorAdjust;
    SystemTask;
    TEIdle(hTE);
    temp := GetNextEvent(everyEvent,myEvent);
    CASE myEvent.what OF

    mouseDown:
      BEGIN
        code := FindWindow(myEvent.where,whichWindow);
        CASE code OF

        inMenuBar:
          DoCommand(MenuSelect(myEvent.where));

        inSysWindow:
          SystemClick(myEvent,whichWindow);

        inDrag:
          DragWindow(whichWindow,myEvent.where,dragRect);

        inGoAway:
          IF TrackGoAway(whichWindow,myEvent.where)
            THEN doneFlag := TRUE;

        inGrow:
          IF whichWindow = FrontWindow
            THEN GrowWnd(whichWindow)
            ELSE SelectWindow(whichWindow);

        inContent:
        BEGIN
          IF whichWindow <> FrontWindow
          THEN SelectWindow(whichWindow)
          ELSE
          BEGIN {front}
            GlobalToLocal(myEvent.where);
            IF PtInRect (myEvent.where, PRect)
            THEN IF BitAnd(myEvent.modifiers,512) <> 0   { Shift key pressed }
              THEN TEClick(myEvent.where,TRUE,hTE)
              ELSE TEClick(myEvent.where,FALSE,hTE)
            ELSE
            BEGIN  {controls}
              MyControl := FindControl(myEvent.where,whichWindow,whichControl);
              Case MyControl of
                inUpButton:   t :=
                            TrackControl(whichControl,myEvent.where,@scrollUp);
                inDownButton: t :=
                           TrackControl(whichControl,myEvent.where,@scrollDown);
                inPageUP:     PageScroll(MyControl,-10);
                inPageDown:   PageScroll(MyControl,10);
                inThumb:      BEGIN
                                t :=
                                TrackControl(whichControl,myEvent.where,NIL);
                                ScrollBits
                              END
              END {Case MyControl}
            END {controls}
          END {front}
        END {in Content}
      END     { of code case }
    END;    { of mouseDown }

    keyDown, autoKey:
      BEGIN
        IF theWindow = FrontWindow
          THEN
            BEGIN
              theChar := CHR(myEvent.message MOD 256);
              IF BitAnd(myEvent.modifiers,256) <> 0    { Command key pressed }
                THEN DoCommand(MenuKey(theChar))
                ELSE TEKey(theChar,hTE)
            END
      END;    { of keyDown }

    activateEvt:
      BEGIN
        DrawGrowIcon(theWindow);
        IF ODD (myEvent.modifiers) THEN   { window is becoming active }
        BEGIN
          TEActivate(hTE);
          ShowControl (vScroll);
          ShowControl (hScroll)
        END ELSE
        BEGIN
          TEDeactivate(hTE);
          HideControl (vScroll);
          HideControl (hScroll)
        END
      END;    { of activateEvt }

    updateEvt:
      BEGIN
        BeginUpdate(theWindow);
        DrawWindow(theWindow);
        EndUpdate(theWindow)
      END    { of updateEvt }

    END    { of event case }

  UNTIL doneFlag
END.

