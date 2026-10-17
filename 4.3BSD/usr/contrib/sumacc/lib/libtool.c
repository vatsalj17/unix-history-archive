/*	libtool.c	1.0	84/05/01	*/

/*
 * Toolbox library.
 *
 * Copyright (C) 1984, Stanford Univ. SUMEX project.
 * May be used but not sold without permission.
 */

/*
 * history
 * 05/01/84	Croft	Created.
 */

/* LINTLIBRARY */

#include "quickdraw.h"
#undef v
#undef h
#include "osintf.h"
#include "toolintf.h"
#include "libmac.h"


/*
 * TOOL INTERFACE
 */

/* General Utilities */

int	BitAnd(l1,l2) { _mact2(RI,0xa858,L,L,L) }
int	BitOr(l1,l2) { _mact2(RI,0xa85b,L,L,L) }
int	BitXor(l1,l2) { _mact2(RI,0xa859,L,L,L) }
int	BitNot(l) { _mact1(RI,0xa85a,L,L) }
int	BitShift(l,c) { _mact2(RI,0xa85c,L,L,S) }
int	BitTst(cp,n) char *cp; { _mact2(RI,0xa85d,B,L,L) }
void	BitSet(cp,n) char *cp; { _mact2(RN,0xa85e,N,L,L) }
void	BitClr(cp,n) char *cp; { _mact2(RN,0xa85f,N,L,L) }
void	LongMul(a,b,dst) Int64Bit *dst; { _mact3(RN,0xa867,N,L,L,L) }
int	FixMul(a,b) { _mact2(RI,0xa868,L,L,L) }
int	FixRatio(numer,denom) { _mact2(RI,0xa869,L,S,S) }
int	HiWord(x) { _mact1(RI,0xa86a,S,L) }
int	LoWord(x) { _mact1(RI,0xa86b,S,L) }
int	FixRound(x) { _mact1(RI,0xa86c,S,L) }
void	PackBits(s,d,sc) Ptr s,d; { _mact3(RN,0xa8cf,N,L,L,S) }
void	UnPackBits(s,d,dc) Ptr *s,*d; { _mact3(RN,0xa8d0,N,L,L,S) }
int	SlopeFromAngle(a) { _mact1(RI,0xa8bc,L,S) }
int	AngleFromSlope(s) { _mact1(RI,0xa8c4,S,L) }
int	DeltaPoint(a,b) Point *a,*b; { _mact2(RI,0xa84f,L,SS,SS) }

int	HandToHand(h) Handle *h; { _macc1(RI,_handtoha,S,L) }
int	PtrToXHand(sp,dh,s) Ptr sp; Handle dh; { _macc3(RI,_ptrtoxha,S,L,L,L) }
int	PtrToHand(sp,dh,s) Ptr sp; Handle *dh; { _macc3(RI,_ptrtohan,S,L,L,L) }
int	HandAndHand(h1,h2) Handle h1,h2; { _macc2(RI,_handandh,S,L,L) }
int	PtrAndHand(p,h,s) Ptr p; Handle h; { _macc3(RI,_ptrandha,S,L,L,L) }

Handle	NewString(s) char *s; { _mact1(RI,0xa906,L,PS) }
void	SetString(sh,s) char *s,**sh; { _mact2(RN,0xa907,N,L,PS) }
Handle	GetString(id) { _mact1(RI,0xa9ba,L,S) }
void	GetIndString(s,id,i) char *s; { _macc3(RN,_getindst,N,VPS,S,S) }

int	Munger(h,off,p1,l1,p2,l2) Handle h; Ptr p1,p2; { _mact6(RI,0xa9e0,L,L,L,L,L,L,L) }

Handle	GetIcon(id) { _mact1(RI,0xa9bb,L,S) }
void	PlotIcon(r,h) Rect *r; Handle h; { _mact2(RN,0xa94b,N,L,L) }
CursHandle GetCursor(id) { _mact1(RI,0xa9b9,L,S) }
PatHandle GetPattern(id) { _mact1(RI,0xa9b8,L,S) }
PicHandle GetPicture(id) { _mact1(RI,0xa9bc,L,S) }
void	GetIndPattern(p,id,i) Pattern *p; { _macc3(RN,_getindpa,N,L,S,S) }

void	ShieldCursor(r,o) Rect *r; Point *o; { _mact2(RN,0xa855,N,L,SS) }


/* Font Manager */

void	InitFonts() { _mact1(RN,0xa8fe,N,N) }
void	GetFontName(id,name) char *name; { _mact2(RN,0xa8ff,N,S,VPS) }
void	GetFNum(name,id) char *name; int *id; { _mact2(RN,0xa900,N,PS,VS) }
void	SetFontLock(flag) { _mact1(RN,0xa903,N,B) }
FMOutPtr FMSwapFont(inrec) FMInput *inrec; { _mact1(RI,0xa901,L,L) }
int	RealFont(id,size) { _mact2(RI,0xa902,B,S,S) }


/* Event Manager */

int	EventAvail(m,r) EventRecord *r; { _mact2(RI,0xa971,B,S,L) }
int	GetNextEvent(m,r) EventRecord *r; { _mact2(RI,0xa970,B,S,L) }
int	StillDown() { _mact1(RI,0xa973,B,N) }
int	WaitMouseUp() { _mact1(RI,0xa977,B,N) }
void	GetMouse(p) Point *p; { _mact1(RN,0xa972,N,L) }
int	TickCount() { _mact1(RI,0xa975,L,N) }
int	Button() { _mact1(RI,0xa974,B,N) }
void	GetKeys(k) KeyMap *k; { _mact1(RN,0xa976,N,L) }

int	PostEvent(n,msg) { _macc2(RI,_posteven,S,S,L) }
void	FlushEvents(which,stop) { _macc2(RN,_flusheve,N,S,S) }
void	SetEventMask(mask) { _macc1(RN,_setevent,N,S) }
int	OSEventAvail(m,e) EventRecord *e; { _macc2(RI,_oseventa,B,S,L) }
int	GetOSEvent(m,e) EventRecord *e; { _macc2(RI,_getoseve,B,S,L) }

/* Window Manager */

void	ClipAbove(w) WindowPeek w; { _mact1(RN,0xa90b,N,L) }
void	PaintOne(w,r) WindowPeek w; RgnHandle r; { _mact2(RN,0xa90c,N,L,L) }
void	PaintBehind(w,r) WindowPeek w; RgnHandle r; { _mact2(RN,0xa90d,N,L,L) }
void	SaveOld(w) WindowPeek w; { _mact1(RN,0xa90e,N,L) }
void	DrawNew(w,f) WindowPeek w; { _mact2(RN,0xa90f,N,L,B) }
void	CalcVis(w) WindowPeek w; { _mact1(RN,0xa909,N,L) }
void	CalcVisBehind(w,r) WindowPeek w;RgnHandle r; { _mact2(RN,0xa90a,N,L,L) }
void	ShowHide(w,f) WindowPtr w; { _mact2(RN,0xa908,N,L,B) }

int	CheckUpdate(e) EventRecord *e; { _mact1(RI,0xa911,B,L) }
void	GetWMgrPort(p) GrafPtr *p; { _mact1(RN,0xa910,N,L) }

void	InitWindows() { _mact1(RN,0xa912,N,N) }
WindowPtr NewWindow(w,r,t,v,id,beh,ga,ref) WindowRecord *w; Rect *r; char *t; WindowPtr beh; { _mact8(RI,0xa913,L,L,L,PS,B,S,L,B,L) }

void	DisposeWindow(w) WindowPtr w; { _mact1(RN,0xa914,N,L) }
void	CloseWindow(w) WindowPtr w; { _mact1(RN,0xa92d,N,L) }
void	MoveWindow(w,h,v,f) WindowPtr w; { _mact4(RN,0xa91b,N,L,S,S,B) }
void	SizeWindow(w,wt,ht,f) WindowPtr w; { _mact4(RN,0xa91d,N,L,S,S,B) }
int	GrowWindow(w,p,b) WindowPtr w; Point *p; Rect *b; { _mact3(RI,0xa92b,L,L,SS,L) }
void	DragWindow(w,p,b) WindowPtr w; Point *p; Rect *b; { _mact3(RN,0xa925,N,L,SS,L) }
void	ShowWindow(w) WindowPtr w; { _mact1(RN,0xa915,N,L) }
void	HideWindow(w) WindowPtr w; { _mact1(RN,0xa916,N,L) }
void	SetWTitle(w,t) WindowPtr w; char *t; { _mact2(RN,0xa91a,N,L,PS) }
void	GetWTitle(w,t) WindowPtr w; char *t; { _mact2(RN,0xa919,N,L,VPS) }
void	HiliteWindow(w,f) WindowPtr w; { _mact2(RN,0xa91c,N,L,B) }
void	BeginUpdate(w) WindowPtr w; { _mact1(RN,0xa922,N,L) }
void	EndUpdate(w) WindowPtr w; { _mact1(RN,0xa923,N,L) }
void	SetWRefCon(w,d) WindowPtr w; { _mact2(RN,0xa918,N,L,L) }
int	GetWRefCon(w) WindowPtr w; { _mact1(RI,0xa917,L,L) }
void	SetWindowPic(w,p) WindowPtr w; PicHandle p; { _mact2(RN,0xa92e,N,L,L) }
PicHandle GetWindowPic(w) WindowPtr w; { _mact1(RI,0xa92f,L,L) }
void	BringToFront(w) WindowPtr w; { _mact1(RN,0xa920,N,L) }
void	SendBehind(w,b) WindowPtr w,b; { _mact2(RN,0xa921,N,L,L) }
WindowPtr FrontWindow() { _mact1(RI,0xa924,L,N) }
void	SelectWindow(w) WindowPtr w; { _mact1(RN,0xa91f,N,L) }
int	TrackGoAway(w,p) WindowPtr w; Point *p; { _mact2(RI,0xa91e,B,L,SS) }
void	DrawGrowIcon(w) WindowPtr w; { _mact1(RN,0xa904,N,L) }

void	ValidRect(r) Rect *r; { _mact1(RN,0xa92a,N,L) }
void	ValidRgn(r) RgnHandle r; { _mact1(RN,0xa929,N,L) }
void	InvalRect(r) Rect *r; { _mact1(RN,0xa928,N,L) }
void	InvalRgn(r) RgnHandle r; { _mact1(RN,0xa927,N,L) }
int	FindWindow(p,w) Point *p; WindowPtr *w; { _mact2(RI,0xa92c,S,SS,L) }
WindowPtr GetNewWindow(id,w,beh) WindowRecord *w; WindowPtr beh; { _mact3(RI,0xa9bd,L,S,L,L) }
int	PinRect(r,p) Rect *r; Point *p; { _mact2(RI,0xa94e,L,L,SS) }
int	DragGrayRgn(rh,p,br,sr,ax,proc) RgnHandle rh; Point *p; Rect *br,*sr; ProcPtr proc; { _mact6(RI,0xa905,L,L,SS,L,L,S,L) }


/* TextEdit */

void	TEActivate(h) TEHandle h; { _mact1(RN,0xa9d8,N,L) }
void	TECalText(h) TEHandle h; { _mact1(RN,0xa9d0,N,L) }
void	TEClick(p,e,h) Point *p; TEHandle h; { _mact3(RN,0xa9d4,N,SS,B,L) }
void	TECopy(h) TEHandle h; { _mact1(RN,0xa9d5,N,L) }
void	TECut(h) TEHandle h; { _mact1(RN,0xa9d6,N,L) }
void	TEDeactivate(h) TEHandle h; { _mact1(RN,0xa9d9,N,L) }
void	TEDelete(h) TEHandle h; { _mact1(RN,0xa9d7,N,L) }
void	TEDispose(h) TEHandle h; { _mact1(RN,0xa9cd,N,L) }
void	TEIdle(h) TEHandle h; { _mact1(RN,0xa9da,N,L) }
void	TEInit() { _mact1(RN,0xa9cc,N,N) }
void	TEKey(k,h) TEHandle h; { _mact2(RN,0xa9dc,N,S,L) }
TEHandle TENew(d,v) Rect *d,*v; { _mact2(RI,0xa9d2,L,L,L) }
void	TEPaste(h) TEHandle h; { _mact1(RN,0xa9db,N,L) }
void	TEScroll(dh,dv,h) TEHandle h; { _mact3(RN,0xa9dd,N,S,S,L) }
void	TESetSelect(ss,se,h) TEHandle h; { _mact3(RN,0xa9d1,N,L,L,L) }
void	TESetText(t,l,h) Ptr t; TEHandle h; { _mact3(RN,0xa9cf,N,L,L,L) }
void	TEInsert(t,l,h) Ptr t; TEHandle h; { _mact3(RN,0xa9de,N,L,L,L) }
void	TEUpdate(r,h) Rect *r; TEHandle h; { _mact2(RN,0xa9d3,N,L,L) }
void	TESetJust(j,h) TEHandle h; { _mact2(RN,0xa9df,N,S,L) }
Handle	TEGetText(h) TEHandle h; { _mact1(RI,0xa9cb,L,L) }
/* Box drawing utility */
void	TextBox(t,l,r,s) Ptr t; Rect *r; { _mact4(RN,0xa9ce,N,L,L,L,S) }


/* Resource Manager */

int	InitResources() { _mact1(RI,0xa995,S,N) }
void	RsrcZoneInit() { _mact1(RN,0xa996,N,N) }
void	CreateResFile(f) char *f; { _mact1(RN,0xa9b1,N,PS) }
int	OpenResFile(f) char *f; { _mact1(RI,0xa997,S,PS) }
void	UseResFile(n) { _mact1(RN,0xa998,N,S) }
int	GetResFileAttrs(n) { _mact1(RI,0xa9f6,S,S) }
void	SetResFileAttrs(n,a) { _mact2(RN,0xa9f7,N,S,S) }
void	UpdateResFile(n) { _mact1(RN,0xa999,N,S) }
void	CloseResFile(n) { _mact1(RN,0xa99a,N,S) }
void	SetResPurge(i) { _mact1(RN,0xa993,N,B) }
void	SetResLoad(a) { _mact1(RN,0xa99b,N,B) }
int	CountResources(t) char *t; { _mact1(RI,0xa99c,S,TS) }
Handle	GetIndResource(t,i) char *t; { _mact2(RI,0xa99d,L,TS,S) }
int	CountTypes() { _mact1(RI,0xa99e,S,N) }
void	GetIndType(t,i) char *t; { _mact2(RN,0xa99f,N,L,S) }
int	UniqueID(t) char *t; { _mact1(RI,0xa9c1,S,TS) }
Handle	GetResource(t,i) char *t; { _mact2(RI,0xa9a0,L,TS,S) }
Handle	GetNamedResource(t,s) char *t,*s; { _mact2(RI,0xa9a1,L,TS,PS) }
void	LoadResource(r) Handle r; { _mact1(RN,0xa9a2,N,L) }
void	ReleaseResource(r) Handle r; { _mact1(RN,0xa9a3,N,L) }
void	DetachResource(r) Handle r; { _mact1(RN,0xa992,N,L) }
void	ChangedResource(r) Handle r; { _mact1(RN,0xa9aa,N,L) }
void	WriteResource(r) Handle r; { _mact1(RN,0xa9b0,N,L) }
int	HomeResFile(r) Handle r; { _mact1(RI,0xa9a4,S,L) }
int	CurResFile() { _mact1(RI,0xa994,S,N) }
int	GetResAttrs(r) Handle r; { _mact1(RI,0xa9a6,S,L) }
void	SetResAttrs(r,a) Handle r; { _mact2(RN,0xa9a7,N,L,S) }
void	GetResInfo(r,id,t,n) Handle r; int *id; char *t,*n; { _mact4(RN,0xa9a8,N,L,VS,L,VPS) }
void	SetResInfo(r,id,n) Handle r; char *n; { _mact3(RN,0xa9a9,N,L,S,PS) }
void	AddResource(r,t,id,n) Handle r; char *n,*t; { _mact4(RN,0xa9ab,N,L,TS,S,PS) }
void	AddReference(r,id,n) Handle r; char *n; { _mact3(RN,0xa9ac,N,L,S,PS) }
void	RmveResource(r) Handle r; { _mact1(RN,0xa9ad,N,L) }
void	RmveReference(r) Handle r; { _mact1(RN,0xa9ae,N,L) }
int	SizeResource(r) Handle r; { _mact1(RI,0xa9a5,L,L) }
int	ResError() { _mact1(RI,0xa9af,S,N) }


/* Control Manager */

ControlHandle NewControl(w,r,t,vis,val,min,max,p,c) WindowPtr w; Rect *r; char *t; { _mact9(RI,0xa954,L,L,L,PS,B,S,S,S,S,L) }
void	DisposeControl(c) ControlHandle c; { _mact1(RN,0xa955,N,L) }
void	KillControls(w) WindowPtr w; { _mact1(RN,0xa956,N,L) }

void	MoveControl(c,h,v) ControlHandle c; { _mact3(RN,0xa959,N,L,S,S) }
void	SizeControl(c,w,h) ControlHandle c; { _mact3(RN,0xa95c,N,L,S,S) }
void	DragControl(c,p,b,s,a) ControlHandle c; Point *p; Rect *b,*s; { _mact5(RN,0xa967,N,L,SS,L,L,S) }
void	ShowControl(c) ControlHandle c; { _mact1(RN,0xa957,N,L) }
void	HideControl(c) ControlHandle c; { _mact1(RN,0xa958,N,L) }
void	SetCTitle(c,t) ControlHandle c; char *t; { _mact2(RN,0xa95f,N,L,PS) }
void	GetCTitle(c,t) ControlHandle c; char *t; { _mact2(RN,0xa95e,N,L,VPS) }
void	HiliteControl(c,s) ControlHandle c; { _mact2(RN,0xa95d,N,L,S) }
void	SetCRefCon(c,r) ControlHandle c; { _mact2(RN,0xa95b,N,L,L) }
int	GetCRefCon(c) ControlHandle c; { _mact1(RI,0xa95a,L,L) }

void	SetCtlValue(c,v) ControlHandle c; { _mact2(RN,0xa963,N,L,S) }
int	GetCtlValue(c) ControlHandle c; { _mact1(RI,0xa960,S,L) }

int	GetCtlMin(c) ControlHandle c; { _mact1(RI,0xa961,S,L) }
int	GetCtlMax(c) ControlHandle c; { _mact1(RI,0xa962,S,L) }
void	SetCtlMin(c,v) ControlHandle c; { _mact2(RN,0xa964,N,L,S) }
void	SetCtlMax(c,v) ControlHandle c; { _mact2(RN,0xa965,N,L,S) }

ProcPtr	GetCtlAction(c) ControlHandle c; { _mact1(RI,0xa96a,L,L) }
void	SetCtlAction(c,p) ControlHandle c; ProcPtr p; { _mact2(RN,0xa96b,N,L,L) }

int	TestControl(c,p) ControlHandle c; Point *p; { _mact2(RI,0xa966,S,L,SS) }
int	TrackControl(c,p,pp) ControlHandle c; Point *p; ProcPtr pp; { _mact3(RI,0xa968,S,L,SS,L) }

int	FindControl(p,w,c) ControlHandle *c; Point *p; WindowPtr w; { _mact3(RI,0xa96c,S,SS,L,L) }
void	DrawControls(w) WindowPtr w; { _mact1(RN,0xa969,N,L) }
ControlHandle	GetNewControl(id,w) WindowPtr w; { _mact2(RI,0xa9be,L,S,L) }


/* Dialog Manager */

void	InitDialogs(p) ProcPtr p; { _mact1(RN,0xa97b,N,L) }
DialogPtr GetNewDialog(id,w,b) Ptr w; WindowPtr b; { _mact3(RI,0xa97c,L,S,L,L) }
DialogPtr NewDialog(w,r,t,vis,p,b,ga,c,i) Ptr w; Rect *r; char *t; WindowPtr b; Handle i; { _mact9(RI,0xa97d,L,L,L,PS,B,S,L,B,L,L) }
int	IsDialogEvent(e) EventRecord *e; { _mact1(RI,0xa97f,B,L) }
int	DialogSelect(e,d,i) EventRecord *e; DialogPtr *d; int *i; { _mact3(RI,0xa980,B,L,L,VS) }
void	ModalDialog(p,i) ProcPtr p; int *i; { _mact2(RN,0xa991,N,L,VS) }
void	DrawDialog(d) DialogPtr d; { _mact1(RN,0xa981,N,L) }
void	CloseDialog(d) DialogPtr d; { _mact1(RN,0xa982,N,L) }
void	DisposDialog(d) DialogPtr d; { _mact1(RN,0xa983,N,L) }
int	Alert(id,p) ProcPtr p; { _mact2(RI,0xa985,S,S,L) }
int	StopAlert(id,p) ProcPtr p; { _mact2(RI,0xa986,S,S,L) }
int	NoteAlert(id,p) ProcPtr p; { _mact2(RI,0xa987,S,S,L) }
int	CautionAlert(id,p) ProcPtr p; { _mact2(RI,0xa988,S,S,L) }
void	CouldAlert(id) { _mact1(RN,0xa989,N,S) }
void	FreeAlert(id) { _mact1(RN,0xa98a,N,S) }
void	CouldDialog(id) { _mact1(RN,0xa979,N,S) }
void	FreeDialog(id) { _mact1(RN,0xa97a,N,S) }
void	ParamText(c0,c1,c2,c3) char *c0,*c1,*c2,*c3; { _mact4(RN,0xa98b,N,PS,PS,PS,PS) }
void	ErrorSound(p) ProcPtr p; { _mact1(RN,0xa98c,N,L) }
void	GetDItem(d,i,k,h,b) DialogPtr d; int *k; Handle *h; Rect *b; { _mact5(RN,0xa98d,N,L,S,VS,L,L) }
void	SetDItem(d,i,k,h,b) DialogPtr d; Handle h; Rect *b; { _mact5(RN,0xa98e,N,L,S,S,L,L) }
void	SetIText(h,t) Handle h; char *t; { _mact2(RN,0xa98f,N,L,PS) }
void	GetIText(h,t) Handle h; char *t; { _mact2(RN,0xa990,N,L,VPS) }
void	SelIText(d,i,s,e) DialogPtr d; { _mact4(RN,0xa97e,N,L,S,S,S) }

void	DlgCut(d) DialogPtr d; { _macc1(RN,_dlgcut,N,L) }
void	DlgPaste(d) DialogPtr d; { _macc1(RN,_dlgpaste,N,L) }
void	DlgCopy(d) DialogPtr d; { _macc1(RN,_dlgcopy,N,L) }
void	DlgDelete(d) DialogPtr d; { _macc1(RN,_dlgdelet,N,L) }


/* Desk Manager */

int	SystemEvent(e) EventRecord *e; { _mact1(RI,0xa9b2,B,L) }
void	SystemClick(e,w) EventRecord *e; WindowPtr w; { _mact2(RN,0xa9b3,N,L,L) }
void	SystemTask() { _mact1(RN,0xa9b4,N,N) }
void	SystemMenu(m) { _mact1(RN,0xa9b5,N,L) }
int	SystemEdit(e) { _mact1(RI,0xa9c2,B,S) }
int	OpenDeskAcc(a) char *a; { _mact1(RI,0xa9b6,S,PS) }
void	CloseDeskAcc(n) { _mact1(RN,0xa9b7,N,S) }


/* Menu Manager */

void	InitMenus() { _mact1(RN,0xa930,N,N) }
MenuHandle NewMenu(id,t) char *t; { _mact2(RI,0xa931,L,S,PS) }
MenuHandle GetMenu(id) { _mact1(RI,0xa9bf,L,S) }
void	DisposeMenu(m) MenuHandle m; { _mact1(RN,0xa932,N,L) }
void	AppendMenu(m,s) MenuHandle m; char *s; { _mact2(RN,0xa933,N,L,PS) }
void	InsertMenu(m,i) MenuHandle m; { _mact2(RN,0xa935,N,L,S) }
void	DeleteMenu(id) { _mact1(RN,0xa936,N,S) }
void	DrawMenuBar() { _mact1(RN,0xa937,N,N) }
void	ClearMenuBar() { _mact1(RN,0xa934,N,N) }

Handle	GetMenuBar() { _mact1(RI,0xa93b,L,N) }
Handle	GetNewMBar(id) { _mact1(RI,0xa9c0,L,S) }
void	SetMenuBar(m) Handle m; { _mact1(RN,0xa93c,N,L) }

int	MenuSelect(p) Point *p; { _mact1(RI,0xa93d,L,SS) }
int	MenuKey(k) { _mact1(RI,0xa93e,L,S) }
void	HiliteMenu(id) { _mact1(RN,0xa938,N,S) }

void	SetItem(m,i,s) MenuHandle m; char *s; { _mact3(RN,0xa947,N,L,S,PS) }
void	GetItem(m,i,s) MenuHandle m; char *s; { _mact3(RN,0xa946,N,L,S,VPS) }
void	EnableItem(m,i) MenuHandle m; { _mact2(RN,0xa939,N,L,S) }
void	DisableItem(m,i) MenuHandle m; { _mact2(RN,0xa93a,N,L,S) }
void	CheckItem(m,i,c) MenuHandle m; { _mact3(RN,0xa945,N,L,S,B) }

void	SetItemIcon(m,i,n) MenuHandle m; { _mact3(RN,0xa940,N,L,S,S) }
void	GetItemIcon(m,i,n) MenuHandle m; int *n; { _mact3(RN,0xa93f,N,L,S,VS) }
void	SetItemStyle(m,i,n) MenuHandle m; { _mact3(RN,0xa942,N,L,S,S) }
void	GetItemStyle(m,i,n) MenuHandle m; int *n; { _mact3(RN,0xa941,N,L,S,VS) }
void	SetItemMark(m,i,n) MenuHandle m; { _mact3(RN,0xa944,N,L,S,S) }
void	GetItemMark(m,i,n) MenuHandle m; int *n; { _mact3(RN,0xa943,N,L,S,VS) }
void	SetMenuFlash(c) { _mact1(RN,0xa94a,N,S) }
void	FlashMenuBar(id) { _mact1(RN,0xa94c,N,S) }

MenuHandle GetMHandle(id) { _mact1(RI,0xa949,L,S) }
int	CountMItems(m) MenuHandle m; { _mact1(RI,0xa950,S,L) }
void	AddResMenu(m,t) MenuHandle m; char *t; { _mact2(RN,0xa94d,N,L,TS) }
void	InsertResMenu(m,t,a) MenuHandle m; char *t; { _mact3(RN,0xa951,N,L,TS,S) }
void	CalcMenuSize(m) MenuHandle m; { _mact1(RN,0xa948,N,L) }


/* Scrap Manager */

int	GetScrap(h,t,o) Handle h; char *t; int *o; { _mact3(RI,0xa9fd,L,L,TS,L) }
ScrapStuff *InfoScrap() { _mact1(RI,0xa9f9,L,N) }
int	LoadScrap() { _mact1(RI,0xa9fa,L,N) }
int	PutScrap(l,t,s) char *t; Ptr s; { _mact3(RI,0xa9fe,L,L,TS,L) }
int	UnloadScrap() { _mact1(RI,0xa9fa,L,N) }
int	ZeroScrap() { _mact1(RI,0xa9fc,L,N) }

/* package manager */

void	InitAllPacks() { _mact1(RN,0xa9e6,N,N) }
void	InitPack(id) { _mact1(RN,0xa9e5,N,S) }

