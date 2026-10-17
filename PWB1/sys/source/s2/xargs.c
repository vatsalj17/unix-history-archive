#include "macros.h"
#define FALSE 0
#define TRUE 1
#define MAXSBUF 255
#define MAXIBUF 512
#define MAXINSERTS 5
#define BUFSIZE 570
#define MAXARGS 255
char *arglist[MAXARGS+1];
char argbuf[BUFSIZE+1];
char *next { argbuf };
char *lastarg { "" };
char **ARGV { arglist };
char *LEOF { "_" }; 
char *INSPAT { "{}" };
struct inserts {
	char **p_ARGV;		/* where to put newarg ptr in arg list */
	char *p_skel;		/* ptr to arg template */
	} saveargv[MAXINSERTS];
char ins_buf[MAXIBUF];
char *p_ibuf;
int PROMPT -1;
int BUFLIM 470;
int N_ARGS 0;
int N_args 0;
int MORE TRUE;
int PER_LINE FALSE;
int ERR FALSE;
int OK TRUE;
int LEGAL FALSE;
int TRACE FALSE;
int INSERT FALSE;
int linesize 0;
int ibufsize;

main(argc,argv)
int argc;
char **argv; {

char *cmdname, *initbuf, **initlist;
int  initsize;
register int j, n_inserts;
register struct inserts *psave;

SCCSID(@(#)xargs.c	1.10);

				/* initialization */

argc--; argv++;
n_inserts = 0;
psave = saveargv;

				/* look for flag arguments */

while  ( (*argv)[0] == '-'  ) {
	switch ( (*argv)[1] ) {
	case 'x': LEGAL = TRUE;
		  break;
	case 'l': PER_LINE = LEGAL = TRUE;
		  N_ARGS = 0;
		  INSERT = FALSE;
		  break;
	case 'i': INSERT = PER_LINE = LEGAL = TRUE;
		  N_ARGS = 0;
		  if ( (*argv)[2] ) {
			INSPAT = &(*argv)[2];
			}
		  break;
	case 't': TRACE = TRUE;
		  break;
	case 'e': LEOF = &(*argv)[2];
		  break;
	case 's': BUFLIM = atoi( &(*argv)[2] );
		  if( BUFLIM>470  ||  BUFLIM<=0 ) {
			ermsg("0 < max-line-size <= 470: ",*argv,"\n",0);
			OK = FALSE;
			}
		  break;
	case 'n': if( (N_ARGS = atoi(&(*argv)[2])) <= 0 ) {
			ermsg("#args must be positive int: ",*argv,"\n",0);
			OK = FALSE;
			}
		  else {
			if( PER_LINE )
				LEGAL = FALSE;
			INSERT = PER_LINE = FALSE;
			}
		  break;
	case 'p': if( (PROMPT = open("/dev/tty",0)) == -1) {
			ermsg("can't read from tty for -p\n",0);
			OK = FALSE;
			}
		  else
			TRACE = TRUE;
		  break;
	default:  ermsg("unknown option: ",*argv,"\n",0);
		  OK = FALSE;
		  break;
	}
	argv++;
	if ( --argc < 1 ) break;
	}
if( ! OK )
	ERR = TRUE;

				/* pick up command name */

if ( argc == 0 ) {
	cmdname = "/bin/echo";
	*ARGV++ = addarg(cmdname);
	}
else
	cmdname = *argv;

				/* pick up args on command line */

while ( OK && argc-- ) {
	if ( INSERT && ! ERR ) {
		if ( index(*argv, INSPAT) != -1 ) {
			if ( ++n_inserts > MAXINSERTS ) {
				ermsg("too many args with ", INSPAT, "\n" ,0);
				ERR = TRUE;
				OK = FALSE;
				}
			psave->p_ARGV = ARGV;
			(psave++)->p_skel = *argv;
			}
		}
	*ARGV++ = addarg( *argv++ );
	}

				/* pick up args from standard input */

initbuf = next;
initlist = ARGV;
initsize = linesize;

while ( OK && MORE ) {
	next = initbuf;
	ARGV = initlist;
	linesize = initsize;
	if ( *lastarg )
		*ARGV++ = addarg( lastarg );

	while ( (*ARGV++ = getarg()) && OK );

				/* insert arg if requested */

	if ( INSERT ) {
		p_ibuf = ins_buf;
		ARGV--;
		j = ibufsize = 0;
		for ( psave=saveargv;  ++j<=n_inserts;  ++psave ) {
			addibuf(psave);
			if ( ERR ) break;
			}
		}
	*ARGV = 0;

				/* exec command */

	if ( ! ERR ) {
		if ( ! MORE && (PER_LINE | (N_ARGS && (N_args==0))) ) exit (0);
		OK = TRUE;
		j = TRACE ? echoargs() : TRUE;
		if( j ) {
			if ( lcall(cmdname, arglist) != -1 ) continue;
			ermsg(cmdname," not executed or returned -1\n", 0);
			OK = FALSE;
			}
		}
	}
if ( OK ) exit (0); else exit (1);
}

checklen(arg)
char *arg;
{
register int oklen;

oklen = TRUE;
if ( (linesize =+ length(arg)+1) > BUFLIM ) {
	lastarg = arg;
	if( N_ARGS ) N_args = 1;
	oklen = OK = FALSE;
	if ( LEGAL ) {
		ERR = TRUE;
		ermsg( "arg list too long\n", 0 );
		}
	}
return ( oklen  ? arg : 0 );
}

addarg(arg)
char *arg;
{
	copy( arg, next);
	arg = next;
	next =+ length(arg)+1;
	return ( checklen(arg) );
}

getarg()
{
register char c, c1, *arg;

while ( (c=getchr()) == ' '
	|| c == '\n'
	|| c == '\t' );
if ( c == '\0' ) {
	MORE = FALSE;
	return 0;
	}

arg = next;
for ( ; ; c = getchr() )
	switch ( c ) {

	case '\t':
	case ' ' :
		if ( INSERT ) { *next++ = c;
				break;
				}
	case '\n':
	case '\0':
		*next++ = '\0';
		if( equal(arg,LEOF) ) {
			MORE = FALSE;
			if( c==' ' || c=='\t' )
				while( c=getchr() )
					if( c=='\n' ) break;
			return 0;
			}
		else {
			if ( (PER_LINE && (c == '\n'))
			||   (N_ARGS && (++N_args >= N_ARGS)) ) {
				N_args = 0;
				lastarg = "";
				OK = FALSE;
				}
			return (checklen(arg));
			}

	case '\\':
		*next++ = getchr();
		break;

	case '"':
	case '\'':
		while( (c1=getchr()) != c) {
			if( c1 == '\0' || c1 == '\n' ) {
				*next++ = '\0';
				ermsg("missing quote?: ",arg,"\n",0);
				OK = FALSE;
				ERR = TRUE;
				return (0);
				}
			*next++ = c1;
			}
		break;

	default:
		*next++ = c;
		break;
	}
}
ermsg(messages)
char *messages;
{
register char **pmsg;

pmsg = (&messages)-1;
while ( *++pmsg )
	write(2,*pmsg,length(*pmsg));
}

echoargs()
{
register char **anarg;
char yesorno[1], junk[1];
register int j;

anarg = arglist-1;
while ( *++anarg ) {
	write(2, *anarg, length(*anarg) );
	write(2," ",1);
	}
if( PROMPT == -1 ) {
	write(2,"\n",1);
	return TRUE;
	}
write(2,"?...",4);
if( read(PROMPT,yesorno,1) == 0 )
	exit(0);
if( yesorno[0] == '\n' )
	return FALSE;
while( ((j=read(PROMPT,junk,1))==1) && (junk[0]!='\n') );
if( j==0 )
	exit (0);
return ( yesorno[0]=='y' );
}

insert(pattern, subst)
char *pattern, *subst;
{
char buffer[MAXSBUF+1];
int len, ipatlen;
register char *pat;
register char *bufend;
register char *pbuf;

len = length(subst);
ipatlen = length(INSPAT)-1;
pat = pattern-1;
pbuf = buffer;
bufend = &buffer[MAXSBUF];

while ( *++pat ) {
	if( index(pat,INSPAT) == 0 ) {
		if ( pbuf+len >= bufend ) break;
		else {
			copy( subst, pbuf);
			pat =+ ipatlen;
			pbuf =+ len;
			}
		}
	else {
		*pbuf++ = *pat;
		if (pbuf >= bufend ) break;
		}
	}

if ( ! *pat ) {
	*pbuf = '\0';
	return (&buffer);
	}
else {
	ermsg( "max arg size with insertion via ", INSPAT, "'s exceeded\n", 0 );
	ERR = TRUE;
	OK = FALSE;
	return 0;
	}
}

addibuf(p)
struct inserts *p;
{
register char *newarg, *skel, *sub;
int l;

skel = p->p_skel;
sub = *ARGV;
linesize =- length(skel)+1;
newarg = insert(skel,sub);
if ( checklen(newarg) ) {
	if( (ibufsize =+ (l=length(newarg)+1)) > MAXIBUF) {
		ermsg("insert-buffer overflow\n", 0);
		ERR = TRUE;
		OK = FALSE;
		}
	copy(newarg, p_ibuf);
	*(p->p_ARGV) = p_ibuf;
	p_ibuf =+ l;
	}
}
getchr() {
char c;
if ( read(0,&c,1) == 1 ) return (c);
return (0);
}
int lcall(sub,subargs)
char *sub, **subargs;
{
	int retcode;

	switch( fork() ) {
	default:
		if( wait(&retcode) == -1  ||  retcode<<8 != 0 )
			return -1;
		return( retcode>>8 );
	case 0:
		pexec(sub,subargs);
		exit (-1);
	case -1:
		return (-1);
		}
}
static char Isccsid[] "@(#)index	1.1";
/*
	If `s2' is a substring of `s1' return the offset of the first
	occurrence of `s2' in `s1',
	else return -1.
*/

index(as1,as2)
char *as1,*as2;
{
	register char *s1,*s2,c;
	int offset;

	s1 = as1;
	s2 = as2;
	c = *s2;

	while (*s1)
		if (*s1++ == c) {
			offset = s1 - as1 - 1;
			s2++;
			while ((c = *s2++) == *s1++ && c) ;
			if (c == 0)
				return(offset);
			s1 = offset + as1 + 1;
			s2 = as2;
			c = *s2;
		}
	 return(-1);
}
static char Csccsid[] "@(#)cat	1.1";
/*
	Concatenate strings.
 
	cat(destination,source1,source2,...,sourcen,0);
 
	returns destination.
*/

cat(dest,source)
char *dest, *source;
{
	register char *d, *s, **sp;

	d = dest;
	for (sp = &source; s = *sp; sp++) {
		while (*d++ = *s++) ;
		d--;
	}
	return(dest);
}
