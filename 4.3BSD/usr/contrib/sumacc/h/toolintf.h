/*	toolintf.h	1.0	05/03/84	*/

/*
 * Toolbox interface.
 *
 * Copyright 1982, 1983, 1984 by Apple Computer, Inc.  All Rights Reserved.
 *
 * C language version Copyright 1984, Stanford Univ. SUMEX project.
 * May be used but not sold without permission.
 *
 * history
 * 11/23/83	Sebok	Merge of mac*mgr, textedit and grafutil.
 * 02/10/84	Martin	Inline traps.
 * 03/20/84	Knaster	Fixed trap numbers.
 * 05/03/84	Croft	C language version.
 * 10/10/84	Croft	Merged in JTC mods; July/Oct supplements.
 */


/* for Font Manager */

#define	systemFont	0
#define	applFont	1
#define	newYork		2
#define	geneva		3
#define	monaco		4
#define	venice		5
#define	london		6
#define	athens		7
#define	sanFran		8
#define	toronto		9

#define	propFont	0x9000
#define	fixedFont	0xB000
#define	fontWid		0xACB0


/*for Window Manager*/

/*window messages*/
#define	wDraw		0
#define	wHit		1
#define	wCalcRgns	2
#define	wNew		3
#define	wDispose	4
#define	wGrow		5
#define	wDrawGIcon	6

/*types of windows*/
#define	dialogKind 	2
#define	userKind   	8

/*window definition procedure IDs*/
#define	documentProc	0
#define	dBoxProc    	1
#define	plainDBox	2
#define	altDBoxProc	3
#define	noGrowDocProc	4
#define	rDocProc    	16

/*FindWindow Result Codes*/
#define	inDesk      	0
#define	inMenuBar   	1
#define	inSysWindow 	2
#define	inContent   	3
#define	inDrag      	4
#define	inGrow      	5
#define	inGoAway    	6

/*defProc hit test codes*/
#define	wNoHit      	0
#define	wInContent  	1
#define	wInDrag     	2
#define	wInGrow     	3
#define	wInGoAway   	4

/* axis constraints for DragGrayRgn */
#define	bothAxes	0
#define	hAxisOnly	1
#define	vAxisOnly	2


/*for TextEdit*/

#define	teJustLeft	0
#define	teJustRight	-1
#define	teJustCenter	1


/*for Resource Manager*/

/*Resource attribute byte*/
#define	resSysRef       128             /* Reference to system resource? */
#define	resSysHeap      64              /* System or application heap?   */
#define	resPurgeable    32              /* Purgeable resource?           */
#define	resLocked       16              /* Load it in locked?            */
#define	resProtected    8               /* Protected?                    */
#define	resPreload      4               /* Load in on OpenResFile?       */
#define	resChanged      2               /* Resource changed?             */

#define	mapReadOnly     128             /* Resource file read-only       */
#define	mapCompact      64              /* Compact resource file         */
#define	mapChanged      32              /* Write map out at update       */

#define	resNotFound     -192            /* Resource not found            */
#define	resFNotFound    -193            /* Resource file not found       */
#define	addResFailed    -194            /* AddResource failed            */
#define	addRefFailed    -195            /* AddReference failed           */
#define	rmvResFailed    -196            /* RmveResource failed           */
#define	rmvRefFailed    -197            /* RmveReference failed          */

/* ID's for resources in sysResDef */
/* cursors */
#define	iBeamCursor	1
#define	crossCursor	2
#define	plusCursor	3
#define	watchCursor	4

/* icons */
#define	stopIcon	0
#define	noteIcon	1
#define	cautionIcon	2

/* patterns */
#define	patternList	0	/* ID of PAT# containing 38 patterns */


/*for Control Manager*/

/*control messages*/
#define	drawCntl	0 
#define	testCntl	1 
#define	calcCRgns	2 
#define	initCntl	3 
#define	dispCntl	4 
#define	posCntl		5 
#define	thumbCntl	6 
#define	dragCntl	7 
#define	autoTrack	8 


/*FindControl Result Codes*/
#define	inButton       10 
#define	inCheckbox     11 
#define	inUpButton     20 
#define	inDownButton   21 
#define	inPageUp       22 
#define	inPageDown     23 
#define	inThumb        129 

/*control definition proc ID's*/
#define	pushButProc     0 
#define	checkBoxProc    1 
#define	radioButProc    2 
#define	scrollBarProc   16 

#define	useWFont        8 

/*for Dialog Manager*/

#define	userItem      0 
#define	ctrlItem      4 
#define	btnCtrl     0               /* Low two bits: what kind of control */
#define	chkCtrl     1 
#define	radCtrl     2 
#define	resCtrl     3 

#define	statText      8               /* Static text    */
#define	editText      16              /* Editable text  */
#define	iconItem      32              /* Icon item      */
#define	picItem       64              /* Picture item   */
#define	itemDisable   128             /* Disable item if set */

#define	OK            1               /* OK button is first by convention */
#define	okButton	OK
#define	Cancel        2               /* Cancel button is 2nd by convention */
#define	cancelButton	Cancel
#define	DlgFont	(*(short *)0xafa)


/*for Desk Manager*/

#define	undoCmd      0 
#define	cutCmd       2 
#define	copyCmd      3 
#define	pasteCmd     4 
#define	clearCmd     5 


/*for Menu Manager*/

#define	noMark		0	/* mark symbols for MarkItem */
#define	commandMark	0x11
#define	checkMark	0x12
#define	diamondMark	0x13
#define	appleMark	0x14
#define	textMenuProc	0

/* menu defProc messages */
#define	mDrawMsg      0 
#define	mChooseMsg    1 
#define	mSizeMsg      2 


/*for Scrap Manager*/

#define	noTypeErr   -102 

/*package manager*/
#define	dskInit   2           /*Disk Initializaton*/
#define	stdFile   3           /*Standard File*/
#define	flPoint   4           /*Floating-Point Arithmetic*/
#define	trFunc    5           /*Transcendental Functions*/
#define	intUtil   6           /*International Utilities*/
#define	bdConv    7           /*Binary/Decimal Conversion*/


/*General Utilities*/

typedef	long	Fixed;
typedef	struct {
	long	hiLong;
	long	loLong;
} Int64Bit;

typedef	Cursor * CursPtr;
typedef	CursPtr * CursHandle;

typedef	Pattern * PatPtr;
typedef	PatPtr * PatHandle;


/*for Font Manager*/

typedef	struct {
	short	family;
	short	size;
	char	face;
	char	needBits;
	short	device;
	Point	numer;
	Point	denom;
} FMInput;

typedef FMOutRec FMOutPut;	/* already defined in quickdraw */

typedef	struct {
	short	fontType;
	short	firstChar;
	short	lastChar;
	short	widMax;
	short	kernMax;
	short	nDescent;
	short	fRectMax;
	short	chHeight;
	short	owTLoc;
	short	ascent;
	short	descent;
	short	leading;
	short	rowWords;
/*	short	bitImage[rowWords,chHeight];
	short	locTable[];
	short	owTable[]; */
} FontRec;


/*for Event Manager*/

typedef	struct {
	long	kmap[4];	/* 4*32 = 128 bits */
} KeyMap;


/*for Window Manager*/

struct WindowRecord {
	GrafPort port;
	short	windowKind;
	char	visible;
	char	hilited;
	char	goAwayFlag;
	char	spareFlag;
	RgnHandle strucRgn;
	RgnHandle contRgn;
	RgnHandle updateRgn;
	Handle	windowDefProc;
	Handle	dataHandle;
	Handle	titleHandle;
	short	titleWidth;
	Handle	controlList;
	struct WindowRecord *nextWindow;
	PicHandle windowPic;
	long	refCon;
};

typedef struct WindowRecord WindowRecord;
typedef GrafPtr WindowPtr;
typedef	WindowRecord *WindowPeek;


/*for TextEdit*/

typedef	struct {
	Rect	destRect;
	Rect	viewRect;
	Rect	selRect;		/* select rectangle */
	short	lineHeight;		/* current font lineheight */
	short	fontAscent;		/* current font ascent */
	Point	selPoint;		/* selection point (mouseLoc) */
	short	selStart;
	short	selEnd;
	short	active;			/* nonzero if active */
	ProcPtr	wordBreak;		/* wordbreak routine */
	ProcPtr	clikLoop;		/* clickloop routine */
	long	clickTime;		/* time of 1st click */
	short	clickLoc;		/* char location of click */
	long	caretTime;		/* time for next caret blink */
	short	caretState;
	short	just;			/* fill style */
	short	teLength;		/* length of text below */
	Handle	hText;			/* handle to actual text */
	short	recalBack;		/* nonzero if recal in backround */
	short	recalLines;		/* line being recaled */
	short	clikStuff;		/* internal clickstuff */
	short	crOnly;			/* -1 if CR line breaks only */
	short	txFont;
	Style	txFace;
	short	txMode;
	short	txSize;
	GrafPtr	inPort;			/* the grafport */
	ProcPtr	highHook;		/* highlighting hook */
	ProcPtr	caretHook;
	short	nLines;
	short	lineStarts[1];		/* actual linestarts */
} TERec;

typedef	TERec * TEPtr;
typedef TEPtr * TEHandle;
#define	TEScrapHandle	(*(Handle *)0xab4)
#define	TEScrapLen	(*(short *)0xab0)


/*for Control Manager*/

typedef	struct {
	Handle	nextControl;
	WindowPtr contrlOwner;
	Rect	contrlRect;
	char	contrlVis;
	char	contrlHilite;
	short	contrlValue;
	short	contrlMin;
	short	contrlMax;
	Handle	contrlDefProc;
	Handle	contrlData;
	int	(*contrlAction)();
	long	contrlRfCon;
	Str255	contrlTitle;
} ControlRecord;

typedef ControlRecord * ControlPtr;
typedef ControlPtr * ControlHandle;


/*for Dialog Manager*/

typedef	struct	{
	WindowRecord window;
	Handle	items;
	TEHandle textH;
	short	editField;
	short	editOpen;
	short	aDefItem;
} DialogRecord;

typedef WindowPtr DialogPtr;

typedef	struct	{
	Rect	boundsRect;
	short	procID;
	char	visible;
	char	filler1;
	char	goAwayFlag;
	char	filler2;
	long	refCon;
	short	itemsID;
	Str255	title;
} DialogTemplate;

typedef DialogTemplate * DialogTPtr;
typedef DialogTPtr * DialogTHndl;

typedef	struct {
	Rect	boundsRect;
	short	itemsID;
	long	stages;
} AlertTemplate;

#define	boldItem	8
#define	boxDrawn	4
#define	stageShift	4

typedef AlertTemplate * AlertTPtr;
typedef AlertTPtr * AlertTHndl;
#define	AlertStage	(*(short *)0xa9a)


/*for Menu Manager*/

typedef struct {
	short	menuId;
	short	menuWidth;
	short	menuHeight;
	Handle	menuProc;
	long	enableFlags;
	Str255	menuData;
} MenuInfo;

typedef MenuInfo * MenuPtr;
typedef MenuPtr * MenuHandle;


/*for Scrap Manager*/

typedef struct {
	long	scrapSize;
	Handle	scrapHandle;
	short	scrapCount;
	short	scrapState;
	char	*scrapName;
} ScrapStuff;


/* Functions returning other then integer */

ControlHandle	GetNewControl(),NewControl();
CursHandle GetCursor();
DialogPtr GetNewDialog(),NewDialog();
FMOutPtr FMSwapFont();
Handle	GetIcon(),GetIndResource(),GetMenuBar(),GetNamedResource();
Handle	GetNewMBar(),GetResource(),GetString(),NewString(),TEGetText();
MenuHandle GetMHandle(),GetMenu(),NewMenu();
PatHandle GetPattern();
PicHandle GetPicture(),GetWindowPic();
ProcPtr	GetCtlAction();
ScrapStuff *InfoScrap();
TEHandle TENew();
WindowPtr FrontWindow(),GetNewWindow(),NewWindow();
