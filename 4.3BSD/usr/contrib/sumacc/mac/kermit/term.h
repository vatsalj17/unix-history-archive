
#include <stdio.h>     		/* Standard io definitions */
#include "mac/quickdraw.h"	/* Macintosh C interface */
#include "mac/osintf.h"
#include "mac/toolintf.h"
#include "mac/packintf.h"
#include "mac/libmac.h"
     
#define DEL         127     /* Delete (rubout) */
#define ESCAPE      27      /* Escape character */	
     
#define BUFSIZE 1024
#define TRUE        -1      /* Boolean constants */
#define FALSE       0

#define FULLENOUGH  500     /* How long to wait before sending out xoff */
#define TOPMARGIN 10	    /* Terminal display constants */
#define BOTTOMMARGIN 286
#define LEFTMARGIN 12
#define RIGHTMARGIN 480
#define LINEHEIGHT 12
#define CHARWIDTH 6
#define NUMSINGCMDS 9	    /* Number of implemented control character
				   routines */ 
#define NUMMULTESCS  13     /* Number of escape sequences recognised */
#define NUMSINGESCS  6
#define NUMTABS 10
     
     
extern int     
	innum,		    /* Serial driver reference numbers */
	outnum,
	fp,		    /* file refnum */
	flowctl;	    /* xon/xoff flag */


int 
	invert=FALSE,	/* Flag for inverted terminal mode */
	insert=FALSE,
	topmargin=TOPMARGIN,	/* Edges of adjustable window */
	bottommargin=BOTTOMMARGIN,
	textstyle=0,
	overrun=FALSE,
	numinbuffer;	    /* Number of read in characters waiting to be
			       printed */
extern char
	inbuff[],
	tabs[],		/* Tab settings */
	*endbuff;

char
	outchar,	    /* Character sent to host. */
	*outp,		    /* Pointer to next charcter to be printed */
	*begptr,           /* Beginning of escape sequence */
	*endptr,           /* end of escape sequence */
	*escseq,
	charmap[256],	    /* Arrays to "remap" the keyboard */
	contcharmap[256],
	controls,controlq;  /* xon/xoff characters */
extern ControlHandle online;
extern WindowPtr theWindow;
extern EventRecord myEvent;
extern short inrect[];    /* Terminal screen */
short  currect[4],	    /* Rect around current character */
       scrollrect[]  = { 0, 0, 390, 500}; /* Scrolling region */
  /* For terminal asynch. communication */
IOParam xoffparam, outparam, inparam;
extern CSParam controlparam;
RgnHandle dumptr;	    /* Dummy ptr to satisfy scrollbits */
KeyMap theKeys;		    /* Map of which key has been pressed */
Point p;

/* Structure for the dispatch tables of terminal commands. */

typedef struct cmd_elem{
	char name;
	int(*funct)();
}CMD;

typedef 
int(*PFI)();


/* Terminal function declarations. */

int tab(), back_space(), space(),
	carriage_return(),line_feed(),clear_screen(),
	home_cursor(), bell();

int text_mode(),clear_line(),erase_display(), cursor_position(),cursor_right(),
	set_scroll_region(),reverse_line_feed(), insert_delete_line(),
	dummy(), unknown(),delete_char(),insert_mode(),end_insert_mode(),
	csi();

/* Terminal control character function command table. */

CMD controltable[NUMSINGCMDS]=
{ 
	{'\010',back_space},
	{'\040',space},
	{'\n',line_feed},
	{'\r',carriage_return},
	{'\013',reverse_line_feed},
	{'\014',clear_screen},
	{'\036',home_cursor},
	{'\t',tab},
	{'\007',bell}
};

CMD singescapetable[NUMSINGESCS]=
{
	{'>',dummy},
	{'=',dummy},
	{'8',dummy},
	{'7',dummy},
	{'M',reverse_line_feed},
	{'[',csi}
};

/* Terminal escape sequence function command table */
CMD escapetable[NUMMULTESCS]=
{
	{'\113',clear_line},	/* K */
	{'\110',cursor_position},	/* H */
	{'\112',erase_display},	 /* J */
	{'\103',cursor_right},	/* C */
	{'\101',reverse_line_feed}, /* A */
	{'\155',text_mode},     /* m */
	{'\162',set_scroll_region}, /* r */
	{'M',insert_delete_line},  /* M */
	{'P',delete_char},
	{'h',insert_mode},
	{'l',end_insert_mode},
	{'*',unknown},
	{'L',insert_delete_line}
};

