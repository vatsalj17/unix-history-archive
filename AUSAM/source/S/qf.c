#include	<stdio.h>
/*
 *	filter to optimize the output of nroff when it is
 *	directed to a QUME spint micro-3 printer.
 *
 *	GJE and ADF
 *
 */
#define		MAXPAGE		10000	/* max chars on a page */
#define		CR	4	/* a carriage return takes  a CR'th of the time of one character */
#define		BS	1.8	/* backspaces take BS * longer than spaces */
#define CRLF 020
#define ECHO 010
#define		FORWARDS	1
#define		BACKWARDS	0
#define		VT		013
#define		IN_PLOT		1
#define		OUT_PLOT	0
#define		PLOT_IN_CHAR	03
#define		PLOT_OUT_CHAR	02
#define		QUME 0
#define		HUPI		60
#define		VUPI		48	/* vertical units per inch */
#define		PLDEF		12	/* default page length */
#define		QUMEWIDTH	15 * HUPI
#define		DECWRITER 1

struct 
{
	char c;
	int pos;
}
linebuf[500];
int	device	QUME;
FILE	*termin;
int	singlesheet;
int	hpos = 0, vpos = 0;
int	nchars;
int	xpr;	/* the co-ordinate increment for printable characters */
int	xnonpr, ynonpr;	/* the co-ordinate increment for non-printables		*/
int	mode;
int	intended_mode;
int	tab_len;
int	char_width, char_hight, page_length;
int	pitch	12;
int	lpi	6;
int	length	0;
int	ttyold[3], ttynew[3];
char	obuf[BUFSIZ];
long	page[MAXPAGE];

main( ac, av )
int	ac;
char	**av;
{
	int	cmp(), intr();
	FILE	*fopen();
	register	i, j;
	int pagenumber;

	/* set the pitch from name of invocation */
	if( av[0][3] == '0' )
		pitch = 10;

	av++;
	ac--;

	singlesheet = 0;
	while( ac-- )
	{
		if( av[0][0] == '-' )
		{
			switch( av[0][1] )
			{
		case 'p':
			pitch = atoi( &av[0][2] );
			break;
		case 'i':
			lpi = atoi( &av[0][2] );
			break;
		case 's':
			if(device == QUME)
				singlesheet = ((singlesheet = atoi(&av[0][2])) < 1 ? 1: singlesheet);
1;
			break;
		case 'd':
			device = DECWRITER;
			break;

		case 'l':
			length = atoi( &av[0][2] );
			break;
		default:
			errexit( "usage: qf [-pn] [-in] [-ln]\n" );
			}
		}
		else
			errexit( "usage: qf [-pn] [-in] [-ln]\n" );
		av++;
	}

	/* test pitch */
	if( (pitch > HUPI) || (pitch <= 0) )
		errexit( "Unreasonable pitch\n" );
	/* test lines per inch */
	char_width = HUPI/pitch;
	if( (lpi > VUPI) || (lpi <= 0) )
		errexit( "Unreasonable number of lines per inch\n" );
	char_hight = VUPI/lpi;
	page_length = ( length ? VUPI*length : VUPI*PLDEF );
	tab_len = 8*char_width;

	if(singlesheet)
		termin = fopen( "/dev/tty", "r" );
	/* clean exit on signals */
	signal( 2, intr );
	signal( 3, intr );

	/* have to compensate for the fact that nroffs stty will fail */
	if( gtty(1, ttyold) != -1 )
	{
		ttynew[0] = ttyold[0];
		ttynew[1] = ttyold[1];
		ttynew[2] = ttyold[2] & ~(CRLF|ECHO);
		if(stty( 1, ttynew ) < 0)
			errexit("Cant set Qume to suitable mode\n");
	}
	else
		if( device == QUME )
			fprintf(stderr, "Warning: need control of Qume for proper results\n" );

	/* buffered output */
	setbuf( stdout, obuf );

	/*
		Better be sure we know what state
		the qume is in to start with.
	*/
	if(device == QUME)
	{
		mode = OUT_PLOT;
		intended_mode = OUT_PLOT;
		putchar(PLOT_OUT_CHAR);
	}
	putchar('\r');

	/* initialize variables based on known state of Qume */
	xnonpr = xpr = char_width;
	ynonpr = char_hight;


	pagenumber = 0;
	while( !feof(stdin) )
	{
		inpage();
		qsort( page, nchars, sizeof(page[0]), cmp );
		outpage();
		pagenumber++;
		if(singlesheet!=0)if((pagenumber%singlesheet)==0)
		{
			gettobottom();
			fflush(stdout);
			fgets("dummy buffer", sizeof("dummy buffer"), termin);
		}
	}

	/* better get to bottom of last page */
	gettobottom();
	/* get him out of plot mode */
	intended_mode = OUT_PLOT;
	putch('\r');

	fflush(stdout);
	stty( 1, ttyold );
}

intr()
{
	signal( 2, 1 );
	errexit("\002\r\n");
}

errexit(s)
char *s;
{
	stty( 1, ttyold );
	while(*s)
		write(1,s++,1);
	/* exit without flushing */
	_exit(1);
}

/* put out a character and set appropriate variables to describe the position of the Qume */
putch( c )
register char	c;
{
	register	xs, ys;
	register	n;
	static	qume_width = QUMEWIDTH;

	if( ( intended_mode == IN_PLOT ) && !( mode == IN_PLOT ) )
	{
		putchar( PLOT_IN_CHAR );
		mode = IN_PLOT;
	}
	else
		if( ( intended_mode == OUT_PLOT ) && !( mode == OUT_PLOT ) )
		{
			putchar( PLOT_OUT_CHAR );
			mode = OUT_PLOT;
		}

	xs = ( mode == IN_PLOT ? 1 : char_width );
	ys = ( mode == IN_PLOT ? 1 : char_hight );

	switch( c )
	{

		case '\r':
			hpos = 0;
			break;

		case ' ':
			hpos =+ xs;
			if(hpos > qume_width)
				errexit("You have asked the Qume to print past the end of its carriage\n");
			break;

		case '\b':
			hpos =- xs;
			if(hpos < 0)
				errexit("You have asked the Qume to backspace past its left  margin\n");
			break;

		case '\n':
			vpos =+ ys;
			break;

		case VT:
			vpos =- ys;
			break;

		default:
			hpos =+ ( mode == IN_PLOT ? 0 : char_width );
			break;

	}
	putchar( c );
}

/* collect a page of text */
inpage()
{
	register char	c;
	long		l;
	static int	ivpos = 0;
	static	int	ihpos = 0;
	static int vp = 0;
	static int hv = 0;

	static int	lastvpos = 0;
	static int	lasthpos = 0;
	nchars = 0;

	while( ivpos < page_length )
	{
		c = getchar();
		if( feof(stdin) )
			return(-1);
		switch( c )
		{


	case PLOT_IN_CHAR:
		xpr = 0;
		xnonpr = ynonpr = 1;
		break;

	case PLOT_OUT_CHAR:
		xnonpr = xpr = char_width;
		ynonpr = char_hight;
		break;

	case ' ':
		ihpos =+ xnonpr;
		break;

	case '\t':
		break;
	case '\b':
		ihpos =- xnonpr;
		if( ihpos < 0 )
			ihpos = 0;
		break;

	case '\n':
		ivpos =+ ynonpr;
		break;

	case VT:
		ivpos =- ynonpr;
		if( ivpos < 0 )
			errexit( "print not contained on one page\n" );
		break;

	case '\r':
		ihpos = 0;

		lasthpos = 0;
		hv = 0;
		break;

	default:
		/* pack the character and its co-ordinates in a long */
		if(device==QUME)
		{
			vp = ivpos;
			hv = ihpos;
		}
		else
		{
			hv =+ (abs(ihpos-lasthpos) > char_width/2) ?  (((ihpos-lasthpos)/char_width) * char_width): 0;
			vp =+ ((abs(ivpos-lastvpos) > char_hight/4 )?  (( (ivpos-lastvpos)/(char_hight/2)) * char_hight) : 0);
		}
		l = vp;
		l = ( l << 12 ) | hv;
		l = ( l << 8 ) | c;
		page[nchars++] = l;
		if( nchars >= MAXPAGE )
			errexit("too many chars this page ??\n");
		lastvpos = ivpos;
		lasthpos = ihpos;
		ihpos =+ xpr;
		break;

		}
	}
	ivpos =- page_length;
	lastvpos =- page_length;
	vp = 0;
}

/* output a page */
outpage()
{
	register	nch = 0;
	char	c;
	int best;
	int left, right;
	int x, y;
	int method;
	int temp;
	int  hwm;

	while(nch != nchars)
	{
		decode( nch++, &x, &y, &c );
		linebuf[0].c = c;
		linebuf[0].pos = x;
		hwm = 0;
		/* gather the characters on the same line */
		while((decode(nch, &x, &temp, &c) == y) && (nch < nchars))
		{
			nch++;
			linebuf[++hwm].c = c;
			linebuf[hwm].pos = x;
		}
		/*
		   This is the bit that decides whether
			we should print the line forwards or backwards
			and whether we should do a CR first
		*/
		if(device == QUME)
		{
			left = linebuf[0].pos;
			right = linebuf[hwm].pos;
			method = BACKWARDS;
			if( hpos >= right )
				best = (hpos - left) * BS;
			else
				best = (right - left) * BS + (right - hpos);
			if( (temp = ((hpos-left) * BS + (right-left))) < best )
			{
				best = temp;
				method = FORWARDS;
			}
			if( (right + (hpos / CR))< best )
			{
				intended_mode = OUT_PLOT;
				putch('\r');
				method = FORWARDS;
			}
			repeat( OUT_PLOT, '\n', ( y-vpos )/ char_hight );
			if( y != vpos )
				repeat( IN_PLOT, '\n', y-vpos );
		
			if( method == FORWARDS )
				outline( 0, hwm, 1 );
			else
				outline( hwm, 0, -1 );
		}
		else
		{
			/* no point optimising for a decwriter */
			repeat(OUT_PLOT, '\n', (y-vpos)/char_hight );
			putch('\r');
			for(temp=0; temp<=hwm; temp++)
			{
				if( linebuf[temp].pos == hpos )
					putch('\b');
				else
					repeat(OUT_PLOT, ' ', (linebuf[temp].pos - hpos )/char_width - 1);
				putch(linebuf[temp].c);
			}
		}
	}
	if( device == DECWRITER )
	{
		putch('\r');
		vpos = 0;
		fputs( "\n\n---------------------------------------------------------\r\n", stdout );
	}
	else
		vpos =- page_length;
}

/* put out "n" of "c" in the given mode */
repeat( plotype, c, n )
int		plotype;
register char	c;
register	n;
{
	if( n == 0 )
		return;
	intended_mode = plotype;
	/* should never happen */
	if( n < 0 )
	{
		fprintf( stderr, "**** %d %c\n", n, c );
		return;
	}
	while( n-- )
		putch( c );
}

/* long comparison */
cmp( l1, l2 )
long	*l1, *l2;
{
	if( *l1 < *l2 )
		return( -1 );
	if( *l1 == *l2 )
		return( 0 );
	return( 1 );
}

/*
 * get the character and co-odinates out of the long which
 * can be found in page[where]
 */
decode( where, x, y, c )
int where;
int *x, *y;
char *c;
{
	*y = (page[where] >> 20) & 07777;
	*x = (page[where] >> 8) & 07777;
	*c = page[where] & 0377;

	return(*y);
}
/*
 * optimally output a line
 * the line is in the array "linebuf"
 * and is indexed from "start" to "end" in increments of "step"
 */
outline(start, end, step)
int start, end, step;
{
	register int i;
	register char c;
	register int col;
	int count;
	int ti;
	int blflag;

	for( i=start; i!=end+step; )
	{
		col = linebuf[i].pos;
		blflag = 0;
		if( col != hpos )
		{
			if( col > hpos )
			{
				if( col-hpos < char_width )
					repeat( IN_PLOT, ' ', col -hpos );
				else
				{
					/*
						we move to one blank of where we
						should be because the qume
						spaces before it prints a
						character.
					*/
					repeat( OUT_PLOT, ' ', ( col-hpos )/ char_width - 1);
					if( col != (hpos + char_width) )
						repeat( IN_PLOT, ' ', col-hpos - char_width);
					blflag = 1;
				}
			}
			else
			{
				repeat( OUT_PLOT, '\b', (hpos-col)/char_width);
				if( hpos != col )
					repeat( IN_PLOT, '\b', hpos-col );
				intended_mode = IN_PLOT;
			}
		}
		else
			intended_mode = IN_PLOT;

		/* over-strike optimisation */
		/*
			if there is more than one character
			with the same co-ordinate, go into
			plot-mode and stuff them all out.
			First we gather up the characters.
		*/
		count = 1;
		ti = i;
		while( ( col == (linebuf[ti+step].pos) ) && (ti != end ) )
		{
			count++; ti =+ step;
		}
		if( count > 1 )
		{
			if( blflag )
			{
				/*
					expecting a blank to be
					produced but
					we are going to go
					in plot mode so we
					better compensate for the
					fact that a character does
					not generate a blank
					in plot mode.
				*/
				intended_mode = OUT_PLOT;
				putch( ' ' );
			}
			intended_mode = IN_PLOT;
			while( count-- )
			{
				/*
					dump the overstruck characters
					advance i to empty linebuf
				*/
				putch( linebuf[i].c );
				i =+ step;
			}
		}
		else
		{
			if( blflag )
			{
				/*
					a blank is required
					so we better print the character
					in plot mode.
				*/
				intended_mode = OUT_PLOT;
			}
			putch( linebuf[i].c );
			i =+ step;
		}
	}
}
gettobottom()
{
	vpos =% page_length;
	repeat( OUT_PLOT, '\n', (-vpos)/char_hight );
	if(device==QUME)
		if(vpos!=page_length)
			repeat( IN_PLOT, '\n', -vpos );
}

