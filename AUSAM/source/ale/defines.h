#include	<stdio.h>

/*
 *	these defines are the possible values of 'dest'
 */

#define		QUME	0
#define		LP05	1
#define		LA180	2

int	dest;			/* destination for output		*/

/*
 *	the next arrays are indexed by 'dest' and yield
 *	the appropriate number for that destination
 */

#define		WRAP_WIDTH	40	/* max length of a printed line	*/
int	a_width[],		/* width of the output			*/
	a_length[],		/* page length of output		*/
	a_pitch[],		/* pitch of output			*/
	a_lines[],		/* lines/inch of the output		*/
	a_dlabs[];		/* #dummy labels to be output		*/
char	*devices[];		/* the name of the output device	*/

/*
 *	these variables are the values
 *	used for the appropriate info
 */

int	width,			/* output width				*/
	length,			/* page length				*/
	pitch,			/* pitch of output			*/
	lines,			/* lines/inch				*/
	dlabs;			/* #dummy labels			*/

/*
 *	now come all of the flags
 */

int	prflag,			/* print flag				*/
	letflag,		/* produce letters			*/
	labflag,		/* produce lables			*/
	envflag,		/* produce envelopes			*/
	oflag,			/* to specify output file		*/
	sflag,			/* silent flag				*/
	dflag,			/* to specify the destination		*/
	wflag,			/* user specified width			*/
	lflag,			/* user specified page length		*/
	pflag,			/* user specified pitch			*/
	iflag,			/* user specified lines/inch		*/
	cflag;			/* to ignore capital letters in sername */

/*
 *	the next 3 defines are the endings to
 *	be appended to the output filename
 */

#define		LETEND	'A'
#define		LABEND	'L'
#define		ENVEND	'E'

char	outfile[];		/* output file name  default "temp"	*/
int	outp_A,			/* output ptr for letters		*/
	outp_L,			/* output ptr for labels		*/
	outp_E;			/* output ptr for envelopes		*/

/*
 *	these defines are the possible terminating
 *	conditions of getfield()
 *	meanings explained then
 */

#define		END	0
#define		DEAR	1
#define		NL	2
#define		SEP	3

/*
 *	these variables and defines are to do with
 *	error recovery and errors
 */

#define		DEAR_FIELD	1
#define		NORM_FIELD	0
#define		MAXERRORS	10
int	status,			/* status of which field we are in	*/
	recover,		/* used in envsave() & envrest()	*/
	error_flag,		/* whether an error has occured		*/
	compile;		/* just temporary			*/

/*
 *	these buffers are where the fields
 *	are kept before any processing is
 *	performed on them
 */

#define		NFIELDS		10
#define		MAXBPF		80
char	fields[][MAXBPF],	/* -> to where the fields are kept	*/
	dear_field[];		/* where the dear field is kept		*/
int	size,			/* size of last field returned		*/
	size_field[];		/* array where the sizes are kept	*/

/*
 *	these variables are to do
 *	with the input file
 */

#define		MAXLINE		512
int	ioptr,			/* the input file pointer for stdio	*/
	lineno,			/* line number of current file		*/
	stdinput,		/* whether input is from stdin or not	*/
	new_address,		/* whether to remove comments or not	*/
	eof;			/* whether the eof has been reached	*/
char	*infile,		/* the name of the current input file	*/
	line_buf[],		/* where each line is kept when read	*/
	*l_ptr;			/* used as a pointer to line_buf	*/

/*
 *	these are the defines for illegal
 *	character checking
 */

#define		LEGAL		0
#define		ILLEGAL		1
char	illchar[],		/* table of illegal chars		*/
	illc[];			/* all the illegal characters		*/
#define		NULLCHAR	0

/*
 *	these defines are used by lab()
 *	to format the labels
 */

#define		MAXLINES	10
#define		MAXCHARS	50
int	lab_width,		/* width of label (including blanks)	*/
	lab_hight,		/* hight of label (+ blank lines)	*/
	nlabels;		/* #labels formated so far		*/
struct	label
{
	char	lab_lines[MAXLINES][MAXCHARS];
}
 	labs[]; 

/*
 *	these variables and defines are used
 *	by let() to break up the fields into
 *	names and titles etc.
 */

#define		N_NAMES		15
char	*names[],		/* pointers to the names		*/
	**t_ptr;		/* pointer to the titles		*/
int	n_titles,		/* number of titles found		*/
	n_names,		/* number of names found		*/
	still_title;		/* loop variable			*/

/*
 *	this array contains all the known
 *	titles people can have
 */

char	*titles[];		/* titles				*/
