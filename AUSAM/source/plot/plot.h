#include	<cplot.h>



/*
**
**	THE FOLLOW ING HASH DEFINES MAY BE CHANGED IF YOU WISH
**
**	THERE IS SEVERAL CHANGES SO THAT PLOT WILL WORK
**	WHEN THE HARD COPY PLOTTER IS IN PARALLEL WITH THE
**	TERMINAL IN USE. A CODE MUST BE TRANSMITTED TO ACTIVATE
**	THE HARD COPY, AND ANOTHER TO TURN IT OFF, RATHER THAN
**	HAVING A SEPARATE DEVICE TO OPEN AND CLOSE.
**	ALSO, WHEN A HARD COPY IS BEING MADE THE MUST BE NO
**	ECHOING OF ANY INFORMATION ON THE H-COPY, SO AN ADDITIONAL
**	GTTY AND STTY IS DONE TO TURN ECHO OFF.
**	NOTE THAT THE HARD COPY HAS A DEVICE CODE, AND THIS PROGRAM
**	ASSUME THIS TO BE "A"	..... CONSULTATION OF THE 4662 MANUAL
**	WILL EXPLAIN WHAT I MEAN.
**
**	NOTE THAT BOTH <GTTY.H> AND LEX DEFINE "ECHO" DIFFERENTLY,
**	SO THE MESSAGE IS BEWARE. (This is not a wine for drinking, it is a 
**	wine for laying down and avoiding).
*/

/*#define	ELEC		/**/
/*
**	UNSW ELECTRICAL ENGINEERING RUN THEIR HARD COPY
**	IN PARALLEL WITH A STANDARD TERMINAL, AND THE 
**	HARD COPY IS ACTIVATED BY CERTAIN CODE CHARACTERS.
**	IF YOU HAVE THIS SETUP, DEFINE ELEC, OTHERWISE
**	IF THE HARD-COPY IS A SEPARATE DEVICE THEN DON'T
**	DEFINE ELEC.
*/
#ifdef	ELEC

#define	ECHOMODE	010

#endif
#define	INCLUDE_DEPTH	6	/* THE LIMIT OF NESTING OF FILES HE IS ALLOWED */



/*
**	A POINT IS AN X AND A Y VALUE (2 FLOATS)
*/
#define	NUM_OF_POINTS	1500
#define	STRING_SPACE	500



/*
**	THE NUMBER OF TICKS IN BETWEEN EACH NUMBER
*/
#define	TICKS_PER_NUM	2



/*
**	THIS ALTERS THE SCALING SENSITIVITY OF
**	PLOT'S AUTO SCALER. THE SCALING ALGORITM
**	IS SO VOLATILE AND TOUCHY I SUGGEST YOU
**	DON'T CHANGE IT AT ALL
*/
#define	SENSITIVITY	2.0
#define	TICK_SIZE	5
#define	WARN_LIMIT	8	/* IF THERE ARE TOO MANY WARNINGS THEN EXIT */



/*
**	THIS IS THE NUMBER OF ALLOWED MARKS (LINES)
**	ON EACH AXIS [mark x=10.5 y= 67.554]
*/
#define	NUM_MARKS	50



/*#define VERBOSE_ERRORS	/**/



/*	comment the above out if 'c' like terseness in error messages is ok */
#define AUTHOR	"Richard Grevis"
#define	FAST		/* You don't want high quality on the terminal with high  */
			/* baud rate (yuk) */
#define	MAX_FLOAT	1.0e37
#define MAX_INT		32767
#define	XCHRSZ		14
#define	YCHRSZ		22	/* THE SIZE OF A CHARACTER IN PLOTTER UNITS */
#define	PLOTTER		"/dev/plotter"
/*
**	THESE DEFINE VARIOUS MODES THAT THE 2662
**	CAN ENTER
*/
#define	CALL_LIGHT_ON	"\033AK"
#define	CALL_LIGHT_OFF	"\033AL"
#define	PLOTTER_ON	"\033AE"
#define	PLOTTER_OFF	"\n\033AF"	/* The leading newline is so that the af is out of the way on the terminal */



/*define RUN_BUG		/* A DEBUG MODE THAT WILL NOT SLOW PLOT DOWN AS MUCH AS THE DEBUG BELOW */



/*#define DEBUG		/**/



/*
**	THE DEFINES THAT ARE USED FOR THE GRAPHICS
**	PART OF THE PROGRAM
*/

#define	HOR_BOX_SIZE	14	/* THE TENTATIVE LIMIT ON THE LENGTH OF A LINE LABEL */
#define	VER_BOX_SIZE	3	/* THE NUMBER OF LABELS THAT CAN FIT IN THE BOX */
				/* FURTHER BELOW VARIABLES OF THE SAME NAME WILL
				** BE FOUND FOR WHEN THE KEY BOX GROWS DYNAMICALLY
				** TO CONTAIN THE STRING ENTRIES
				*/
				/* THAT IS, THE BOX SIZE IN CHARATERS		*/
#define	CROSS_SIZE	5
#define	STAR_SIZE	2
#define	CIRCLE_SIZE	5



/*
**	THIS FELLA IS THE LENGTH
**	OF THE EXAMPLE LINE WHEN
**	A KEY BOX IS WANTED.
**	TRY IT AND SEE
*/
#define	REG_LINE_LEN	50



/*
**	THE INITIAL FORMAT OF THE NUMBERS PRINTED ON THE BORDER
*/
#define	XFORM_INIT		"%7.4g"
#define	YFORM_INIT		"%7.4g"




/*
**	This defines the size of the screen window.
**	During graphics, everything outside this window is
**	RUTHLESSLY truncated.
*/
#define	WXL	156
#define	WXH	945
#define	WYL	64
#define WYH	716





/*
**	THIS IS THE END OF THE THINGS I DONT MIND YOU CHANGING
*/



#ifdef	RUN_BUG
#define	DEBUG
#endif			/* DEBUG MUST BE ON WHEN RUN_BUG IS ON */
#ifdef	RUN_BUG
#define	BUGFILE		"/tmp/plot.bug"
#else
#define	BUGFILE		"plot.bug"
#endif
#define	YES		1
#define	NO		0





/*	ABORT IS USED IN THE SYNTAX ANALISIS TO SAVE WRITING REPETITIVE CODE */


#define	ABORT(y)	{ error(y); nextgroup(); break; }
#define	size(x)		(x>0?(exp(floor(log(x)/2.302585)*2.302585)):(-(exp(floor(log(-(x))/2.302585)*2.302585))))


#define	SUB_TOKEN	1	/* ANDING WITH THIS WILL PRODUCE TRUE IF NOT MAIN TOKEN	*/
#define	LINES_HEADING	4
#define	EOF		(-1)
#define	UNRECOGNISED	(-3)
#define	BAD_CHAR	(-5)
#define	ERROR		(-1)	/* JUST A GENERAL, NOT SPECIFIC ERROR */
				/* I WISH IT WAS NOT 1, BUT I HAVE PROBS OTHERWISE */



/*
**	THE MAIN TOKENS ARE POWERS OF TWO
*/
#define	LEGEND		2
#define	XAXIS		4
#define	YAXIS		010
#define	DATE		020
#define	PLOT		040
#define	MARK		0100
#define	DATA		0200
#define	NEXT		0400



/*
**	ALL OTHERS ARE ODD
*/
#define	NUMBER		3
#define	STRING		5
#define	KEY_AT		7
#define	END		9
#define	FROM		11
#define	TO		13
#define	BY		15
#define	AT		17
#define	POINT_EQ	19
#define	LINE_EQ		21
#define	X_EQ		23
#define	Y_EQ		25
#define	TIMES		27	/* IT IS A "*" NOTHING ELSE	*/



/*
**	THESE ARE THE LINE AND POINT TYPES AVAILABLE
*/
#define	CROSS		29
#define	DOT		31
#define	CIRCLE		33
#define	STAR		35
#define	SOLID		37
#define	SHORT		39
#define	SHORT_LONG	41
#define	LONG		43
#define	NOTHING		45



/*
**	AND SOME OTHER TOKENS I SHOULD HAVE PLACED HIGHER UP
*/
#define	REGRESSION	47
#define	FORMAT		49
#define	OTHER		51




/*
**	THE STATUSES FOR THE LINE TYPES AND 
**	SO ON.
**	THESE STATI ARE FOR GSTATUS
*/
#define	XMARK		02
#define	YMARK		04
#define	POINT_EXPLICIT	010
#define	LINE_EXPLICIT	020
#define	DID_LABEL	040
#define	INITIALISED	0100	/* true if a graphics init has been done */
#define	TO_HARD_COPY	0200	/* If the -p option is set */
#define	PAUSE_ON	0400
#define	HARD_COPY_ON	01000



/*
**	THE OTHER DEFINED STATUSES
**
*/
#define	XMAN_SCALE	1
#define	YMAN_SCALE	2
#define	KEY		4
#define	REG		010
#define NO_PLOT		020	/* IF ERRORS ARE FOUND WE SUPPRESS PLOTTING */
#define	NO_SPECS	040	/* IF THERE ARE NO PLOT SPECS */
#define	NO_WARNINGS	0100
#define	XINC		0200
#define	YINC		0400
#define	NO_BORDER	01000
#define	MOVE_XAXIS	02000
#define	MOVE_YAXIS	04000
#define	WARNS_FLUSHED	010000
#define	NO_NUMBERS	020000	/* DRAW IN THE AXES AND HEADINGS BUT NOT THE NUMBERS */
#define	NO_DATA		040000




/*
**
**	DECLARATIONS OF GLOBAL VARIABLES
**
*/

float	buffer[NUM_OF_POINTS*2];
float	*saved;		/* pointer to the last point added to the buffer */
int	status;		/* KEEPS THE STATUS OF MANUAL SCALING ETC	*/
int	gstatus;	/* KEEP MORE STATUSES OF THE POINT AND LINE VARIETY */
int	tokens_found;		/* AS EACH ONCE ONLY FLAG IS FOUND IT IS TALLIED HERE */
int	*got_some_data;	/* THE PARSER READS ONE TOKEN TOO FAR WHEN DISCOVERING
				THAT DATA FOLLOWS. THIS VARIABLE KEEP A POINTER
				TO THE DATA SO THAT GETDATA MAY USE IT	*/
int	warncount;	/* KEEPS TRACK OF THE NUMBER OF WARNINGS	*/
char	warn_buf[WARN_LIMIT+1][80];
int	xerr_pos,yerr_pos;	/* THE POSITION OF THE NEXT ERROR OR WARNING MESSAGE */
#define	XERR_INIT	170
#define	YERR_INIT	700	/* THE ERRORS GO DOWN THE PAGE FROM THIS POS */
char	*head_str[LINES_HEADING];
char	*sbuff_ptr;			/* THE POINTER TO THE STRING BUFFER */
int	head_count;			/* THE STORAGE LIMIT  FOR THE HEADINGS	*/
					/* AND THE INDEX TO THE NEXT FREE SPACE	*/
char	*date_str;
char	*x_str;
char	*y_str;
char	*label_str;
int	quadrant;
int	line_type, point_type;
int	x_column, y_column;
int	key_quadrant;
int	reg_quadrant;
int	mark_type;
float	inc_min, inc_max;
float	xscale, yscale;
float	xkey_pos, ykey_pos;		/* POSITION OF THE KEY BOX	*/
int	hor_box_size, ver_box_size;	/* THE SIZE OF A GROWABLE BOX	*/
float	xreg_pos, yreg_pos;
float	xmark[NUM_MARKS];
float	ymark[NUM_MARKS];
int	xmark_count;
int	ymark_count;
float	xtick_gap, ytick_gap;
float	xmin, xmax, ymin, ymax;
float	xpos, ypos;			/* THE POS OF THE AXES ON THE GRAPH */
char	xformat[30], yformat[30];



/*
**
**	THESE ARE THE VARIABLE FOR THE LINE
**	OF BEST FIT ETC WHEN DOING A REGRESSION
**
*/
float	sumx,sumy;
float	sumx2,sumy2;
float	sumxy;
int	n;			/* COUNTER OF THE NUMBER OF POINTS */




/*
**	GLOBAL VARIABLES USED WITH THE LEXICAL ANALYSIS
*/

char	*p_token;	/*	A POINTER TO THE TOKEN STRING	*/
float	f_token;	/*	THE VALUE OF THE TOKEN		*/
			/*	EG IF THE TOKEN IS A NUMBER,	*/
			/*	f_token CONTAINS ITS VALUE	*/
int	token;		/*	THE TYPE OF TOKEN		*/
struct plotfile
{
	char pname[80];
	char fname[16];
	FILE	*fptr;	/* the STDIO file pointer	*/
	int	linenum;	/* save the line number as well */
}	infiles[INCLUDE_DEPTH];
struct	plotfile	*name;		/* A GLOBAL POINTER TO THE CURRENT FILE STRUCTURE */

struct	point
{
	float	x;
	float	y;
};




/*	A CONDITIONAL DEBUG DECLARATION	*/
#ifdef	DEBUG
FILE	*debug;
#endif

FILE	*plotter;




/*	ROUTINE AND		*/



/*	STDIO DECLARATIONS	*/
FILE	*freopen();
char	*fgets();
char	*calloc();
FILE	*fopen();
FILE	*zopen();
float	atof();
float	fabs();
struct	point *getxy();
char	*add_str();
long	time();
char	*skip();
float	exp();
float	floor();
float	ceil();
float	log();
