/*\
 *
 * ulf -- Universal Lineprinter Filter
 *	  Written by Cliff Matthews, on April 7th, 1983 at the
 *	  University of New Mexico
 *
 * Supports:  diablo and dumb (LA180, LA120) "lineprinters"
 *
 * This program uses a "line_structure" that is a linked list of arrays to
 * hold enough characters for one physical line on the lineprinter.  The
 * first array contains the characters of the line, and all subsequent arrays
 * hold overprinting characters.  Blanks are left as zero's, so that they
 * don't have to be printed, just move the printhead whatever way is easiest.
 *
\*/

#include	<stdio.h>
#include	<ctype.h>
#include	<errno.h>
#include	<sgtty.h>
#include	"lp.local.h"

#define	CONTROL		20	/* How many tolerable control characters */
#define	RPRINT		"\0336"	/* Make diablo print backword esc 6 */
#define	FPRINT		"\0335"	/* Make diablo print forward  esc 5 */
#define	FORWARD		1	/* directions for diablo */
#define	BACKWARD	0	/* directions for diablo */

struct	line_s {
	char	*line;
	struct	line_s *next;
};

char	*malloc();
char	*rindex();
char	*pgetstr();
struct	line_s *makeline();

short	BR;		/* baud rate if lp is a tty */
int	FC;		/* flags to clear if lp is a tty */
int	FS;		/* flags to set if lp is a tty */
int	XC;		/* flags to clear for local mode */
int	XS;		/* flags to set for local mode */

char	*lpac;		/* where the lineprinter accounting is kept */
char	*invoke;	/* name this filter was invoked as */
char	*user;		/* name the user that this should be billed to */
char	*ff;		/* what to use for a form feed */
int	linelen;	/* length of a line for current line_printer */
int	pagesize;	/* number of lines on a page */
int	manual;		/* paper must be fed by hand */
int	printhead;	/* location of the printhead */
int	diablo;		/* are we a diablo or not */
int	linenum;	/* number of lines printed so far on this page */
unsigned pages;		/* number of pages printed */
int	direction;	/* 1 means we are printing forward */
static	int eatme = 0;	/* set when disposing of bad output */

main(argc,argv)
int argc;
char *argv[];
{

	char c;				/* last character read */
	struct line_s *linepointer;	/* chars to print later */
	int loc;			/* current printing location */

	init(argv);
	linepointer = makeline();

	while ( (c = getchar()) != EOF ) {
	if (!eatme)
		switch (c) {
		case ' ':			/* Space */
			loc++;
			break;

		case '\b':			/* Backspace */
			loc--;
			break;

		case '\t':			/* Tab */
			loc += 8 - (loc % 8);
			break;

		case '\n':			/* New Line */
			dumplines( linepointer );
			loc = 0;
			break;

		case '\r':			/* Carriage Return */
			loc = 0;
			break;

		case '\f':			/* Form Feed */
			dumplines( linepointer );
			loc = 0;
			throwpage();
			break;

		default:
			c &= 0177;			/* Turn off parity */
			if ( !isprint( c ) )
			tossout();		/* Not Printable */
			else
				if (( loc >= 0) && ( loc <= linelen - 1))
					place( c, loc++, linepointer );
				else
					loc++;			/* off page */
		}
	}
	bill(user);
}

static struct bauds {
	int baud;
	int speed;
} bauds[] = {
	50,	B50,
	75,	B75,
	110,	B110,
	134,	B134,
	150,	B150,
	200,	B200,
	300,	B300,
	600,	B600,
	1200,	B1200,
	1800,	B1800,
	2400,	B2400,
	4800,	B4800,
	9600,	B9600,
	19200,	EXTA,
	38400,	EXTB,
	0,		0
};

init(argv)			/* initialize global variables */
char *argv[];
{
	static char buf[BUFSIZ/2];	/* Stolen from lpr.c */
	char b[BUFSIZ];			/* Stolen from lpr.c */
	int stat;			/* Stolen from lpr.c */
	char *bp = buf;			/* Stolen from lpr.c */
	char *cp;			/* scratch character pointer */
	int i;				/* scratch index */
	register struct bauds *baudp;
	static struct sgttyb sbuf;

	printhead = 0;
	linenum = 0;
	pages = 1;
	invoke = ((cp = rindex(*argv,'/')) ? cp + 1 : *argv);
	user = argv[1];

	if (strncmp("diablo",invoke,sizeof("diablo")-sizeof(char)) == 0)
		diablo = 1;
	else
		diablo = 0;

	if ((stat = pgetent(b, invoke)) <= 0) {
		printf("%s: can't find printer description\n",invoke);
		exit(3);
	} else {
		if ((linelen = pgetnum("pw", &bp)) == NULL)
			linelen = DEFLINELEN;
		if ((pagesize = pgetnum("pl", &bp)) == NULL)
			pagesize = DEFPAGESIZE;
		BR = pgetnum("br", &bp);
		if ((lpac = pgetstr("af", &bp)) == NULL)
			lpac = DEFACCOUNTFILE;
		if ((ff = pgetstr("ff", &bp)) == NULL)
			ff = "\f";
		if ((FC = pgetnum("fc")) < 0)
			FC = 0;
		if ((FS = pgetnum("fs")) < 0)
			FS = 0;
		if ((XC = pgetnum("xc")) < 0)
			XC = 0;
		if ((XS = pgetnum("xs")) < 0)
			XS = 0;
		if ((LF = pgetstr("lf", &bp)) == NULL)
			LF = DEFLOGF;
	}

	if (ioctl(fileno(stdout), TIOCEXCL, (char *)0) < 0) {
		log("cannot set exclusive-use");
		exit(1);
	}
	if (ioctl(fileno(stdout), TIOCGETP, (char *) &sbuf) < 0) {
		log("cannot get tty parameters");
		exit(1);
	}

	if (BR) {
		for (baudp = bauds; baudp->baud; baudp++)
			if (BR == baudp->baud)
				break;
			if (!baudp->baud) {
				log("illegal baud rate %d", BR);
				exit(1);
			}
			sbuf.sg_ispeed = sbuf.sg_ospeed = baudp->speed;
	}
	sbuf.sg_flags &= ~FC;
	sbuf.sg_flags |= FS;
	if (ioctl(fileno(stdout), TIOCSETP, (char *)&sbuf) < 0) {
		log("cannot set tty parameters");
		exit(1);
	}
	if (XC) {
		if (ioctl(fileno(stdout), TIOCLBIC, (char *) &XC) < 0) {
			log("cannot set local tty parameters");
			exit(1);
		}
	}
	if (XS) {
		if (ioctl(fileno(stdout), TIOCLBIS, (char *) &XS) < 0) {
			log("cannot set local tty parameters");
			exit(1);
		}
	}

	putchar ('\r');
	direction = FORWARD;
}

struct line_s *
makeline()	/* malloc up some memory for a line_structure */
{
	struct line_s *s;	/* memory allocated for the new line */
	char *cp;		/* scratch char pointer */
	char *end;		/* last legal array position for while loop */

	s = (struct line_s *) malloc(sizeof(struct line_s));
	if ( s == 0 )
		outofmemory();
	else {
		s->next = NULL;
		cp = s->line = malloc(linelen + 1);
		if ( s->line == 0)
			outofmemory();
		else {
			end = s->line + linelen;
			while ( cp < end )
				*cp++ = ' ';
			*cp = 0;
		}
	}
	return (s);
}

tossout()	/* toss out invalid characters, and keep count, maybe puke */
{

	static int tossed = 0;	/* how many characters have been discarded */

	if (++tossed > CONTROL) {
		if (diablo)
			spaceto(0,FORWARD);
		printf("\r\n\n\n\n\n\n");
		printf("%s: *************************************\r\n",invoke);
		printf("%s: *************************************\r\n",invoke);
		printf("%s: *************************************\r\n",invoke);
		printf("%s: Too many control characters in output\r\n",invoke);
		printf("%s: *************************************\r\n",invoke);
		printf("%s: *************************************\r\n",invoke);
		printf("%s: *************************************\r\n",invoke);
		eatme = 1;
	}
}

place(c, location, p)	/* add a new character to a line_structure */
char c;			/* character to place */
struct line_s *p;	/* structure to put character in */
int location;		/* where in the line the character should go */
{
	char *cp;

	if (*(cp = (p->line + location)) != ' ') {
		if (p->next == NULL)
			p->next = makeline();
		place(c, location, p->next);
	} else
		*cp = c;
}

dumplines(p)		/* dump the top-level line_structure */
struct line_s *p;
{
	char *cp, *end;	/* scratch, and last legit array element */

	if (p->next != NULL)
		rdumpl(p->next);
	qprint(p->line);
	printf("\n");

	linenum++;
	if (linenum == pagesize) {
		pages++;
		linenum = 0;
	}

	cp = p->line;
	end = cp + linelen;
	while ( cp < end )
		*cp++ = ' ';

	p->next = NULL;
}


rdumpl(p)		/* recursively dump the overprints */
struct line_s *p;
{
	if (p->next != NULL)
		rdumpl(p->next);
	qprint(p->line);
	free(p->line);
	free(p);
}

outofmemory()
{
	printf("%s: Out of Memory\n",invoke);
	exit(ENOMEM);
}

qprint(line)		/* print a line efficiently */
char *line;
{
	char *cp;		/* scratch character pointer */
	char *end;		/* last legitimate array position */
	int r,l;		/* right most character, leftmost */
	int i,j;		/* temporary values */

	cp = line + linelen - 1;
	while ( cp >= line && *cp == ' ')
		--cp;

	if ( cp < line )
		return;

	if (diablo) {
		r = cp - line;
		end = line + linelen;
		cp = line;
		while ( *cp++ == ' ' );
		l = cp - line - 1;
		i = abs(printhead - l) + (2 * ((direction == FORWARD) ? 1 : -1));
		j = abs(printhead - r);
		if ( (printhead < l) || (i < j) ) {
			spaceto(l,FORWARD);
			pr(line,l,r);
		} else {
			spaceto(r,BACKWARD);
			pr(line,r,l);
		}

	} else {
		*++cp = '\0';
		printf("%s\r", line);
	}
}

spaceto( where, dir)
int where;			/* which column */
int dir;			/* which way to go afterward */
{
	if (where == 0) {
		putchar('\r');
		direction = FORWARD;	/* where else should we go */
	} else {
		if ( direction != dir ) {
			printf("%s", ( dir == FORWARD ) ? FPRINT : RPRINT );
			direction = dir;
		}
		if (printhead != where) {
			putchar('\033');	/* <esc><tab># gets you to # */
			putchar('\t');
			putchar((char)where+1);
		}
	}
	printhead = where;
}

pr(line,start,stop)	/* prints the characters from start to stop */
int start;		/* array index of first character */
int stop;		/* array index of last character */
char *line;		/* line to play with */
{
	char *end;		/* last character location to access */
	char *cp;		/* scratch character position */

	end = line + stop;
	cp = line + start;
	if (start < stop) {
		while ( cp <= end )
			putchar( *cp++ );
		printhead = stop+1;
	} else {
		while ( cp >= end )
			putchar( *cp-- );
		if ( printhead < 0 )
			printhead = 0;
		printhead = stop-1;
	}
}

throwpage()		/* toss a page */
{
	register int x;

	printf("%s",ff);
	pages++;

	linenum = 0;
}

bill(account)
char *account;
{
	FILE *fopen(), *ac;

	if ((ac = fopen(lpac,"a")) != NULL) {
		fprintf(ac,"%s\t%u\n",account,pages);
		fclose(ac);
	}
}

/*VARARGS1*/
log(message, a1, a2, a3)
char *message;
{
	short console;
	FILE *lfd;
	lfd = fopen(lf,"a");
	console = isatty(fileno(lfd));

	fprintf(lfd, console ? "\r\n%s: " : "%s: ", invoke);
	fprintf(lfd, message, a1, a2, a3);
	if (console)
		putc('\r', lfd);
	putc('\n', lfd);
	fclose(lfd);
}
