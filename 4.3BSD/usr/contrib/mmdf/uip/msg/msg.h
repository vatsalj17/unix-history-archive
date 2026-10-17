/*
 *			M S G . H
 *
 * This file contains the declarations for all the global variables
 * and defines for the MSG program.
 *
 *		R E V I S I O N   H I S T O R Y
 *
 *	06/09/82  MJM	Split the enormous MSG program into pieces.
 *
 *	06/10/82  MJM	Added additional flags bits
 *
 *	08/07/82  MJM	Allowed compile-time setting of NMSGS
 *
 *	09/13/82  MJM	Made msgp[] an array of pointers, to permit
 *			undelete to insert messages.
 *
 *	12/04/82  HW	Added xtra options, .msgrc, fixed two signal bugs.
 */

#define	ON		1
#define	OFF		0

#define SETREAD		1		/* regular file read parsing */
#define SETAPND		2		/* parse appended messages  */
#define SETUNDIGEST	3		/* undigestify one message */

/* Selection of who to answer to */
#define ANSFROM		01		/* just to senders */
#define ANSCC		02		/*   from & cc names */
#define ANSTO		04		/*   from & to names */
#define ANSALL		07		/* copies to all recipients */

/* Flags to gethead() to omit a field */
#define NODATE		(char *)0
#define NOFROM		(char *)0
#define NOSNDR		(char *)0
#define NORPLY		(char *)0
#define NOTO		(char *)0
#define NOCC		(char *)0
#define NOSUBJ		(char *)0

#define SIZEDATE	9
#define SIZEFROM	30	   	/* ALSO change hdrfmt  */
#define SIZESUBJ	40		/* */
#define SIZETO		30

#define	M_BSIZE		256

#define SPSIGS		0400007

struct message {
	long	start;			/* Offset into file */
	long	len;			/* Length of message */
	int	flags;			/* FLAGS word */
	long	date;
	char	datestr[SIZEDATE];
	char	from[SIZEFROM];
	char	to[SIZETO];
	char	subject[SIZESUBJ];
};

/* Flags values in msg[] struct */
#define M_DELETED	0000001		/* Msg flagged for deletion */
#define	M_PUT		0000002		/* Msg put to another file */
#define M_NEW		0000004		/* Msg not seen yet */
#define M_KEEP		0000010		/* Msg to be kept in this file */
#define M_ANSWERED	0000020		/* Msg has been answered */
#define M_FORWARDED	0000040		/* Msg has been forwarded */

#define M_RESTMAIL	0040000		/* Special for OverWrite only */
#define M_PROCESS_IT	0100000		/* Msg flagged for processing */

#ifndef NOEXTERNS
/* MMDF GLOBALS */
extern int 	sentprotect;	/* default mailbox protection */
extern char	*mldflfil;	/* default mailbox file */
extern char	*mldfldir;	/* directory containing mailbox file */
extern char	*delim1;	/* string delimiting messages (start) */
extern char	*delim2;	/* string delimiting messages (end) */
/* End of MMDF GLOBALS */

extern struct message *msgp[];
extern struct message *mptr;

extern int  errno;			/* sys-call error number */

extern int Nmsgs;		/* For Global use */

#define IDENTITY	0x6832		/* Magic Number; current value h2 */
/* Status structure, also exists at the front of a binary index file. */
struct status {
	int		ms_ident;	/* "Magic Number" */
	long		ms_time;	/* Last time file was processed */
	unsigned	ms_nmsgs;	/* Number of messages processed */
	unsigned	ms_curmsg;	/* "Current" message */
	long		ms_eofpos;	/* Msg file eof position */
	unsigned	ms_markno;	/* Marked message no */
	int		ms_xxx1;	/* Place holder - not used */
	int		ms_xxx2;	/* Place holder - not used */
	long		ms_xxx3;	/* Place holder - not used */
} status;

#define OLDIDENTITY	0x6831		/* Magic Number; previous value h1 */
struct oldstatus {
	int		ms_ident;	/* "Magic Number" */
	long		ms_time;	/* Last time file was processed */
	unsigned	ms_nmsgs;	/* Number of messages processed */
	unsigned	ms_curmsg;	/* "Current" message */
	long		ms_eofpos;	/* Msg file eof position */
};

/* Stat buffers, used for comparing times */
struct stat statb1;			/* Reference buffer */
struct stat statb2;			/* Temporary buffer */

char	ch_erase;

int     nottty;
unsigned int    msgno;			/* message currently being processed */

/* user search key for from/subject/text msg sequences */
char    key[80];
char	*gc;

char    filename[64];
char	outfile[256];
char	defmbox[60];
char	oldfile[60];
char	*homedir;		/* Full path to home directory */

char    maininbox[60];
char	binarybox[60];		/* Binary map file filename */
char	msgrcname[60];		/* User options filename */
char    defoutfile[60];
char    username[20];
char    ascending;
char    ismainbox;		/* current file is receiving .mail  */
char    nxtchar;
char    lstsep;			/* separate messages by formfeed   */
char    lstmore;		/* second, or more listed message  */
char	autoconfirm;		/* bypass asking user confirmation */
char	anstype;		/* what addresses to send answers to  */
char	binaryvalid;		/* TRUE if incore binary box is valid */
int	(*orig) ();		/* DEL interrupt value on entry */
int	verbose;

int     outfd;

int exclfd;		/* overwrit() exclusive use fd */
jmp_buf savej;

unsigned int ansnum;
unsigned int fwdnum;

FILE * filefp;			/* Input FILEp */
FILE * outfp;			/* Output FILEp */
FILE * binfp;			/* Binary map FILEp */
FILE * fpmsgrc;			/* User options FILEp */

char    ttyobuf[BUFSIZ];
char    sndto[M_BSIZE];
char    sndcc[M_BSIZE];
char    sndsubj[M_BSIZE + 9];

char	lastc;
int	istty;
int	keystrip;
int	mdots, bdots, bprint;
int	paging;
int	pagesize;
int	quicknflag;
int	quickexit;
int	filoutflag;
int	binsavflag;
int	wmsgflag;
int	doctrlel;
int	prettylist;
int	linelength;
int	linecount;
int	outmem;
char	*ushell, *ueditor;
int	readonly;
char	*tempname;
char	*binarypre;
char	*savmsgfn;
char	*sndname;		/* Name of mail sending program */
char	*nullstr;		/* A null string */

char	draft_work[64];		/* Work file for 2-window answer */
char	draft_original[64];	/* Original message(s) for 2-window ans */
char	draftorig[64];		/* Message saving file */

#define	MAXKEYS		41
char	*keywds[MAXKEYS];
#define	SP_BODY		0	/* Reading msg body */
#define	SP_HNOSP	1	/* Line was NOT stripped */
#define SP_HSP		2	/* Line was stripped */

#define DOIT		0	/* Argument to cpyiter() */
#define NOIT		1	/* Argument to cpyiter() */
#define DOLF		0
#define NOLF		1

extern char *compress ();
extern char *strdup ();
extern char *malloc();
extern char *index(), *strend(), *rindex();

extern prmsg(), delmsg(), undelmsg(), prhdr(), movmsg(), onint(), onnopipe();
extern putmsg(), writmsg(), gomsg(), lstmsg(), keepmsg(), lstbdy(), writbdy();
extern onstop(), hdrfile(), dolstmsg();

extern ansmsg(), fwdmsg(), fwdpost(), resendmsg(), ansend();
extern undigestify();
extern xomsgrc();
extern char *getenv();
extern char *xfgets();
extern long smtpdate();
extern qcomp();

#endif NOEXTERNS
