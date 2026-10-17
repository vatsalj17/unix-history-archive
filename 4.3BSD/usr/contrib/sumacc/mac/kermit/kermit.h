/*
* File kermit.h
*
* Symbol definitions and global variable for kermit.
*/

#include <stdio.h>     		/* Standard io definitions */
#include "mac/quickdraw.h"	/* Macintosh C interface */
#include "mac/osintf.h"
#include "mac/toolintf.h"
#include "mac/packintf.h"
#include "mac/libmac.h"
#include "controls.h"		/* Constants for terminal settings */

     
     
     
     
     
/* Symbol Definitions */
     
#define MAXPACKSIZ  94      /* Maximum packet size */
#define SOH         1       /* Start of header */
#define CR          13      /* ASCII Carriage Return */
#define SP          32      /* ASCII space */
#define DEL         127     /* Delete (rubout) */
     
#define MAXTRY      20      /* Times to retry a packet */
#define MYQUOTE     '#'     /* Standard quoting character */
#define MYPAD       0       /* Number of padding characters I will need */
#define MYPCHAR     0       /* Padding character I need (NULL) */
#define MYEOL       '\n'    /* End-Of-Line character I need */
#define MYTIME      10      /* Seconds after which I should be timed out */
#define MAXTIM      60      /* Maximum timeout interval */
#define MINTIM      2       /* Minumum timeout interval */
     
#define TRUE        -1      /* Boolean constants */
#define FALSE       0

#define	numMenu	    3	    /* Number of menus in program */
#define filemenu    256     /* Reference numbers for menus */
#define modemenu    257
#define optionmenu  255
#define VIS         1       /* Sets window visisble */
#define CONNECT     1       /* Menu item constants */
#define TRANSMIT    2
#define RECEIVE     3
#define CONTROLS    4
#define IMAGE       1
#define MACWRITE    2
#define EXECUTABLE  3
#define FILENAMECNV 4
#define DEBUG	5
#define MYBUFSIZE   2048
#define BUFSIZE	    1024
/* Macintosh error codes */
#define eoFErr      (-39)
#define TOPMARGIN 10	    /* Terminal display constants */
#define BOTTOMMARGIN 286
#define LEFTMARGIN 12
#define RIGHTMARGIN 480
#define CHARWIDTH 6
     
     
/* Macro Definitions */
     
/*
 * tochar: converts a control character to a printable one by adding a space.
 *
 * unchar: undoes tochar.
 *
 * ctl:    converts between control characters and printable characters by
 *         toggling the control bit (ie. ^A becomes A and A becomes ^A).
 */
#define tochar(ch)  ((ch) + ' ')
#define unchar(ch)  ((ch) - ' ')
#define ctl(ch)     ((ch) ^ 64 )
     
     
/* Global Variables */
     
int     size,               /* Size of present data */
        spsiz,              /* Maximum send packet size */
        pad,                /* How much padding to send */
        timint,             /* Timeout for foreign host on sends */
        n,                  /* Packet number */
        numtry,             /* Times this packet retried */
        oldtry,             /* Times previous packet retried */
        filnamcnv = TRUE,   /* -1 means do file name case conversions */
	fp,		    /* Data file reference number (file pointer) */
	innum,		    /* Input driver refnum */
	outnum,		    /* Output driver refnum */

		            /* Mode of data transmission */
        image=FALSE ,       /* 8-bit transmission */
	macwrite=FALSE,     /* Macwrite compatible */
	exec=FALSE,         /* Executable resource file */
	volnum,
    	debug=FALSE,
	flowctl=TRUE;
     
char    state,              /* Present state of the automaton */
        padchar,            /* Padding character to send */
        eol,                /* End-Of-Line character to send */
        escchr,             /* Connect command escape character */
        quote,              /* Quote character in incoming data */
	volname[256],       /* Current volume name */
        filnam[80],            /* Current file name */
        recpkt[MAXPACKSIZ], /* Receive packet buffer */
        packet[522],        /* Packet buffer */
	inbuff[BUFSIZE],    /* buffer for characters not yet printed */
	*endbuff=sizeof(char)*BUFSIZE+inbuff, /* End of buffer */
	mybuff[MYBUFSIZE];  /* Serial drivers new buffer */

MenuHandle myMenus[numMenu];
EventRecord myEvent;
WindowPtr theWindow, whichWindow, ControlWindow,DebugWindow,TabWindow;
WindowRecord wRecord, CWRecord,DWRecord,TWRecord;
			     /* Rectangles */
short wirect[]={40,5,335,505}, /* Kermit window in global */
	inrect[]  = { 0, 0, 390, 500}, /* and local coords */
	myrect[]= {190,80,210,150},    /* To erase packet number */
	ControlRect[]={70,40,280,480}, /* Controls window */
	DebugRect[]={40,15,180,500},
	TabRect[]={310,5,350,505};
CSParam controlparam;      /* To change serial driver paramaters */
ControlHandle baud,bits,parity,stopb,xon,online,theControl;
short config;  		       /* Current Serial driver configuration */

/* Tab settings */
#define NUMTABS 10

char tabs[NUMTABS];
