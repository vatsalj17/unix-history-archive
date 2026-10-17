/*
 *	M S G T A I L O R . C
 */

#define NOEXTERNS
#include "./msg.h"

#ifdef pdp11
#	define NMSGS	500
#else
#	define NMSGS	2000
#endif pdp11

char    *savmsgfn = "savebox";
char	*resendprog = "|/usr/local/mmdf/resend ";
char	*sndname = "/usr/local/mmdf/send"; /* Overridden by .msgrc sendprog */

char	*dflshell = "sh";	/* Overridden by getenv("SHELL"); */
char	*dfleditor = "pen";    /* Overridden by getenv("EDITOR"); */

int	pagesize = 22;		/* default line count */
int	linelength = 80;	/* default line size */

int	paging = OFF;		/* Internal paging of long messages */
int	verbose = OFF;		/* Command completion (raw mode) */
int	wmsgflag = OFF;		/* Save messages when calling send */
int	keystrip = ON;		/* Strip header lines on display */
int	bdots = OFF;		/* Binary box dots */
int	mdots = OFF;		/* Mailbox dots */
int	bprint = OFF;		/* Reading... messages */
int	quicknflag = OFF;	/* Turn off NEW before message prints */
int	quickexit = ON;		/* Don't stay in msg if mailbox is empty */
int	prettylist = ON;	/* Print message numbers in listings */
int	filoutflag = ON;	/* Filter text (display controls as ^X) */
int	binsavflag = ON;	/* Save binary box on exec's & pauses */
int	doctrlel = OFF;		/* Page on control-L */
#ifndef SYS5
long	timezone = 5*60*60;	/* Seconds West of GMT */
int	daylight = 1;		/* Do you use daylight savings time? */
				/* 0 = NO, 1 = YES */
char	*tzname[] = {"EST", "EDT",};
				/* Timezone strings, primary and daylight
				   savings time if used, in that order */
#endif SYS5

/***** Do not tailor below this line *****/
int	Nmsgs = NMSGS;
struct message *msgp[NMSGS];
struct message *mptr;
