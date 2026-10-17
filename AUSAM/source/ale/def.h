/*
 *
 *	This file contains all of the true defines
 *
 */

/*
 *	all of the default values for the possible destinations
 */

int	a_width[]
{
	100, 132, 136		/* page width for QUME, LP05, LA180	*/
};

int	a_length[]
{
	72, 66, 51		/* page length for QUME, LP05, LA180	*/
};

int	a_pitch[]
{
	12, 10, 10		/* pitch for QUME, LP05, LA180		*/
};

int	a_lines[]
{
	6, 8, 6			/* lines/inch for QUME, LP05, LA180	*/
};

int	a_dlabs[]
{
	2, 10, 10		/* #dummy labels for QUME, LP05, LA180	*/
};

char	*devices[]
{
	"QUME ",
	"LP05 ",
	"LA180"
};

char	outfile[50]	"temp";	/* default output file			*/
int	dest		QUME;	/* default destination			*/
char	illc[]	"!#%*<=>?@[]_|{}",	/* all illegal chars		*/
	illchar[0177];		/* an array subcripted by chars		*/
char	line_buf[MAXLINE];	/* where each line is kept		*/
char	fields[NFIELDS+1][MAXBPF],/* where the fields are kept		*/
	dear_field[MAXBPF];	/* where the dear field is stored	*/
int	size_field[NFIELDS+1];	/* the size of the corrosponding field	*/
char	*names[N_NAMES];	/* used by let() for name recognition	*/
struct	label 	labs[3];		/* where the labels are kept		*/
char	*titles[]		/* all of the possible titles		*/
{
	"Mr",
	"Miss",
	"Mrs",
	"Ms",
	"Sir",
	"Dr",
	"Professor",
	"Associate",
	"Prof",
	"Assoc",
	"Sister",
	"Reverend",
	"The",
	"Honourable",
	"Most",
	0
};
