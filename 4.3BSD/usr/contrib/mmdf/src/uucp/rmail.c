/*
 *                              R M A I L . C
 *
 *      Developed from the Berkeley mail program of the same name
 *      by Mike Obrien at RAND to run with the MMDF mail system.
 *      Rewritten by Doug Kingston, US Army Ballistics Research Laboratory
 *      Hacked a lot by Steve Bellovin (smb@unc)
 *
 *      This program runs SETUID to Opr so that it can set effective and
 *      real [ug]ids to mmdflogin.
 *
 *      Steve Kille -  Aug 84
 *              Take machete to it.  Use protocol mode,
 *              and always go thru submit - phew
 *      Lee McLoughlin Oct 84.
 *              Address munges some header lines into 822 format.
 *	Peter Collinson Dec 85.
 *		Lee's version was dependent on 
 *		(a) the uucp channel
 *		(b) the domain tables associated with the uucp channel
 *		This version costs more cycles (but there is generally
 *		only one of these running on a machine at a time) and
 *		uses mmdf's tables to dis-assemble bang routes converting
 *		them into a domain address.
 *	
 */
#undef  RUNALON

#include "util.h"
#include "mmdf.h"
#include "ap.h"
#include "ch.h"
#include "dm.h"				/* needed for host_equal */
#include <pwd.h>
#include <signal.h>
#include <sys/stat.h>

#define VERSION "3.0"

#define NAMESZ  256      /* Limit on component name size. LMCL was 64 */


char     debug;

extern  char    *locname;
extern  char    *Uchan;
extern  char    *mmdflogin;
extern  char    *sitesignature;
extern	char	*supportaddr;
extern  LLog    *logptr;

extern char *index();
extern char *rindex();
extern char *getenv();	/* get the Accounting system from the environment */
extern char *strdup();
extern char *malloc();
extern char *compress();

FILE *rm_msgf;             /* temporary out for message text */
Chan *chanptr;
int Tmpmode = 0600;
char *Msgtmp = "/tmp/rmail.XXXXXX";
char rm_date[LINESIZE];    /* date of origination from uucp header */
char rm_from[LINESIZE];    /* accumulated path of sender */
char origsys[NAMESZ];   /* originating system */
char origpath[LINESIZE];/* path from us to originating system */
char Mailsys[LINESIZE]; /* Mail system signature */
char tmpline[LINESIZE]; /* Temporary buffer for continuations and such */
char usetmp = 0;        /* Wether tmpline should be used */
#define ungetline(s) (strcpy(tmpline,s),usetmp=1)
char linebuf[LINESIZE]; /* scratchpad */
char nextdoor[LINESIZE];/* The site nextdoor who sent the mail */
char uchan[LINESIZE];	/* Channel to use for submission */
#define MAXADRS 6       /* Max no. of adrs to output on a single line. */

main(argc, argv)
char **argv;
{
	char    fromwhom[NAMESZ];       /* user on remote system */
	char    *fromptr;
	char    sys[NAMESZ];            /* an element in the uucp path */
	char    *cp, *d;
	struct passwd *pw, *getpwnam(), *getpwuid();
	int     fd;

	mmdf_init (argv[0]);

	if( argc > 2 && strcmp( argv[ 1 ], "-d" ) == 0 ){
		debug = 1;
		argc--;
		argv++;
	}

	if (argc < 2) {
		fprintf(stderr, "Usage: rmail user [user ...]\n");
		exit(1);
	}
	umask (0);

	/* First, change effective and real UID/GID into Mailsys owner */
	if ((pw = getpwnam (mmdflogin)) == NULL) {
		fprintf (stderr, "Cannot find mmdflogin\n");
		exit (99);
	}
	setgid (pw->pw_gid);
	setuid (pw->pw_uid);

	sprintf(Mailsys, "%s <%s@%s>", sitesignature, mmdflogin, locname);



	/* Create temp file for body of message */
	mktemp (Msgtmp);
	if ((fd = creat(Msgtmp, Tmpmode)) < 0)
		bomb ("Can't create %s\n", Msgtmp);
	close( fd );

	if ((rm_msgf = fopen(Msgtmp, "r+")) == NULL)
		bomb ("Can't reopen %s\n", Msgtmp);

	unlink (Msgtmp);

	/* Unravell the From and >From lines at the head of the message
	 * to work who sent it, the path it took to get here and when
	 * it was sent.
	 * Some or all of this info may be ignore depending on the 822 header
	 * info given.
	 */
	for (;;) {
		if( gets( linebuf ) == NULL )
			break;
		if(   strncmp(linebuf, "From ", 5)
		   && strncmp(linebuf, ">From ", 6) )
			break;

		cp = index (linebuf, ' ');      /* start of name */
		fromptr = ++cp;
		cp = index (cp, ' ');           /* cp at end of name */
		*cp++ = 0;                      /* term. name, cp at date */
		strcpy (fromwhom, fromptr);
		while( isspace( *cp ) ) cp++;   /* Skip any ws */
		/* The date is the rest of the line ending at \0 or remote */
		d = rm_date;
		while( *cp && strncmp( cp, " remote from ", 13 ) )
			*d++ = *cp++;
		*d = '\0';

		for (;;) {
			cp = index(cp+1, 'r');
			if (cp == NULL) {
				cp = rindex(fromwhom, '!');
				if (cp != NULL) {
					char *p;
					*cp = '\0';
					p = rindex(fromwhom, '!');
					if (p != NULL) strcpy(origsys, p+1);
					else strcpy(origsys, fromwhom);
					strcat(rm_from, fromwhom);
					strcat(rm_from, "!");
					strcpy(fromwhom, cp+1);
					goto out;
				}
				/*
				 * Nothing coherent found - so look in
				 * environment for ACCTSYS
				 */
				if ((cp = getenv("ACCTSYS")) && *cp)
				{	strcpy(origsys, cp);
					strcat(rm_from, cp);
					strcat(rm_from, "!");
					goto out;
				}
				cp = "remote from somewhere";
			}
			if (strncmp(cp, "remote from ", 12) == 0)
				break;
		}

		sscanf(cp, "remote from %s", sys);
		strcat(rm_from, sys);
		strcpy(origsys, sys);           /* Save for quick ref. */
		strcat(rm_from, "!");
out:;
	}
	if( fromwhom[0] == '\0' )               /* No from line, illegal */
		bomb ("No from lines in message\n");

	ungetline( linebuf );

	/* Complete the from field */
	strcat( rm_from, fromwhom );
	
	/* A uk special - see if the first two components of the constructed */
	/* bang address are in fact the same site. If so replace by their */
	/* official name */
	domain_cross(rm_from);

	/* Save a copy of the path to the original site.
	 * This is all the the path we were given - the user name after
	 * the last !.  NOTE: We keep the trailing !, hence the +1.
	 */
	strcpy( origpath, rm_from );
	*(rindex( origpath, '!' ) + 1) = '\0';

	/* Savepath is given a copy of the immediate neighbour */
	if( (d = index( rm_from, '!' )) != NULL ){
		*d = '\0';
		strcpy( nextdoor, rm_from );
		*d = '!';
	}
	else    strcpy( nextdoor, rm_from );
	
	/* find the channel depending in the nextdoor site */
	set_channel(nextdoor);
	ch_llinit(chanptr);

	/* Convert the from to Arpa format and leave it in from */
	fromcvt( rm_from, fromwhom );
	strcpy( rm_from, fromwhom );

	if( debug )
		printf( "from=%s, origpath=%s, date=%s\n",
			rm_from, origpath, rm_date );

	msgfix( rm_from, rm_date );

	xsubmit( rm_from, argv, argc );

	exit( 0 );
}

/*
 * Munge the message header.
 * All header lines with addresses in are munged into 822 format.
 * If no "From:" line is given supply one based on the UUCP Froms, do the
 * same if no "Date:".
 */
typedef	struct
{	char	*hname;
	int	hfound;
} Hdrselect;

Hdrselect hdrselect[] = {
	"date",	0,		/* this is a little naughty - we need */
				/* to register that we have had this */
				/* but not address munge it */
				/* so it has token 0 */
	"from",	0,
	"to",	0,
	"cc",	0,
	"bcc",	0,
	"sender",	0,
	"reply-to",	0,
	"resent-from",	0,
	"resent-sender",0,
	"resent-to",	0,
	0,	0
#define HDATE	0
#define HFROM	1
#define HTO	2
#define HCC	3
#define HBCC	4
#define HSENDER	5			
#define HREPLY	6
#define HRFROM	7
#define HRSENDER	8			
#define HRTO	9
};

/* Is this header in the list of those to have their addresses munged? */
/* return header value offset value */
/* notice that it returns 0 for the date header */
shouldmunge( name )
char *name;
{
	register Hdrselect *h;

	if( debug )
		printf( "in shouldmunge with %s\n", name );

	for( h = hdrselect; h->hname != NULL; h++ )
		if( lexequ( h->hname, name ) )
		{	h->hfound++;
			return( h - hdrselect );
		}
	return( 0 );
}

/* If this is a header line then grab the name of the header and stuff it
 * into name then return a pointer to the actually body of the line.
 * Otherwise return NULL.
 * NOTE: A header is a line that begins with a word formed of alphanums and
 * dashes (-) then possibly some whitespace then a colon (:).
 */
char *
grabheader( s, name )
register char *s, *name;
{	char tmpbuf[LINESIZE];
	/* Copy the name into name */
	while( isalpha( *s ) || isdigit( *s ) || *s == '-' )
		*name++ = *s++;
	*name = '\0';

	/* Skip any whitespace */
	while( isspace( *s ) )
		*s++;

	/* This is a header if the next char is colon */
	if( *s == ':' )
	{	s++;
		/* 
		 * This is probably illegal but we fail horribly if it
		 * happens - so we will guard against it
		 */
		if (*s != ' ' && *s != '\t' && *s != '\0')
		{	/* we need to add a space */
			strcpy(tmpbuf, s);
			sprintf(s, " %s", tmpbuf);
		}
		return( s );  /* Return a pointer to the rest of the line */
	}
	else    return( NULL );
}

msgfix( from, date )
char *from, *date;
{
	register char *s;
	char *rest;
	char *lkp;
	char name[ LINESIZE ];
	char tmpbuf[ LINESIZE];
	int haveheader = 0;    /* Do I have a header? */
	int headertoken;
	char *grabline();

	/* Loop through all the headers */
	while( 1 )
	{
		if( debug )
			printf( "in msgfix about to grabline\n" );

		s = grabline();

		if( debug )
			printf( "got %s\n", s );

		/* Is this the end of the header? */
		if( *s == '\0' || feof( stdin ) )
			break;


		/* Is this a continuation line? */
		if( haveheader && isspace( *s ) ){
			/* Note: Address munged headers handled specially */
			fprintf( rm_msgf, "%s\n", s );
		}
		else {

			/* Grab the header name from the line */
			if( (rest = grabheader( s, name )) == NULL )
				/* Not a header therefore all headers done */
				break;

			haveheader = 1;

			/* Should I address munge this? */
			if( headertoken = shouldmunge( name ) )
			{	char *finalstr;
				finalstr = "\n";
				fprintf( rm_msgf, "%s: ", name );
				/*
				 * deal specially with From lines
				 * the parser loses comments so we retain
				 * them specially here
				 */
				if (headertoken == HFROM
				 || headertoken == HRFROM)
				{	if (lkp = index(rest, '<'))
					{	*lkp = '\0';
						if (*rest == ' ')
							fprintf( rm_msgf, "%s<", rest+1);
						else
							fprintf( rm_msgf, "%s<", rest);
						*lkp = '<';
						finalstr = ">\n";
						strcpy(tmpbuf, lkp);
						/* notice that we copy from the < *
						/* to start with a space */
						tmpbuf[0] = ' ';
						rest = tmpbuf;
						lkp = rindex(rest, '>');
						if (lkp) *lkp = '\0';
						
					}
					if (lkp = index(rest, '('))
					{	*lkp = '\0';
						sprintf(tmpbuf, " (%s\n", lkp+1);
						finalstr = tmpbuf;
					}
				}
				hadr_munge( rest );
				fputs( finalstr, rm_msgf );
			}
			else    fprintf( rm_msgf, "%s\n", s);
		}
	}

	/* No From: line was given, create one based on the From's */
	if( hdrselect[HFROM].hfound == 0 ){
		fprintf( rm_msgf, "From: %s\n", from );
	}

	/* No Date: line was given, create one based on the From's */
	if( hdrselect[HDATE].hfound == 0 ){
		datecvt( date, tmpbuf );        /* Convert from uucp -> Arpa */
		fprintf( rm_msgf, "Date: %s\n", tmpbuf );
	}

	/* Copy the rest of the file, if there is any more */
	if( !feof( stdin ) ){
		/* If the first line of the message isn't blank
		 * output the blank separator line.
		 */
		if( *s )
			putc( '\n', rm_msgf );
		do {
			fprintf( rm_msgf, "%s\n", s );
			s = grabline();
		} while( !feof( stdin ) );
	}
}

char *
grabline()
{
	if( debug )
		printf( "in grabline " );

	/* Grab the next line.
	 * Remembering this might be the tmpline
	 */
	if( usetmp ){
		if( debug )
			printf( "using tmpline " );
		strcpy( linebuf, tmpline );
		usetmp = 0;
	}
	else    gets( linebuf );

	/* Anything wrong? */
	if( ferror( stdin ) )
		fputs( "\n  *** Problem during receipt from UUCP ***\n", rm_msgf )
;

	if( debug )
		printf( "returning %s\n ", linebuf );

	return( linebuf );

}

char *next = NULL;      /* Where nextchar gets the next char from */
char adrs[ LINESIZE ];  /* Partly munged addresses go here */
nextchar()
{
	register char *s;       /* scratch pointer. */

	if( next != NULL && *next != '\0' ){
		if( debug )
			printf( "<%c>", *next );
		return( *next++ );
	}

	/* The last buffer is now empty, fill 'er up */
	next = grabline();

	if( feof( stdin ) || !isspace( *next ) || *next == '\0' ){
		/* Yipee! We've reached the end of the address list. */
		ungetline( next );
		next = NULL;                    /* Force it to read buffer */
		return( -1 );
	}

	/* Zap excess whitespace */
	compress( next, adrs );

	/* This may be a slightly duff list generated by some of the
	 * UNIX mailers eg: "x y z" rather than "x,y,z"
	 * If it is in this ^^^^^^^ convert to  ^^^^^^^ 
	 * NOTE: This won't handle list: "x<x@y> y<y@z>" conversions!
	 */
	if( index( adrs, ' ' ) != NULL &&
	    index( adrs, ',' ) == NULL &&
	    index( adrs, '<' ) == NULL &&
	    index( adrs, '(' ) == NULL &&
	    index( adrs, '"' ) == NULL ){
		for( s = adrs; *s; s++ )
			if( *s == ' ' )
				*s = ',';
	}
	next = adrs;
	/* This a continuation line so return in a seperator just in case */
	return( ',' );
}


/* Munge and address list thats part of a header line.
 * Try and get the ap_* (MMDF) routines to do as much as possible for
 * us.
 */
hadr_munge( list )
char *list;
{
	AP_ptr the_addr;        /* ---> THE ADDRESS <--- */
	int adrsout = 0;        /* How many address have been output */

	ungetline( list );

	while( 1 ){
		AP_ptr ap_pinit();

		the_addr = ap_pinit( nextchar );

		if( the_addr == (AP_ptr)NOTOK )
			bomb( "cannot initialise address parser!" );

		ap_clear();

		switch( ap_1adr() ){
		case NOTOK:
			/* Something went wrong!! */
			ap_sqdelete( the_addr, (AP_ptr)0 );
			ap_free( the_addr );
			/* Emergency action in case of parser break down:
			 * print out the rest of the line (I hope).
			 */
			fprintf( rm_msgf, "%s", tmpline );
			continue;
		case OK:
			/* I've got an address to output! */
			/* output a comma seperator between the addresses
			 * and a newline every once in a while to make
			 * sure the lines aren't too long
			 */
			if( adrsout++ )
			    fprintf( rm_msgf, ",%s ", adrsout%MAXADRS?"":"\n");

			/* Munge will do all the work to output it */
			munge( the_addr );

			/* Reclaim the space */
			ap_sqdelete( the_addr, (AP_ptr)0 );
			ap_free( the_addr );

			if( debug )
				printf( "munged and space freed\n" );
			break;
		case DONE:
			if( debug )
				printf( "hadr_munge all done\n" );

			/* Reclaim the space */
			ap_sqdelete( the_addr, (AP_ptr)0 );
			ap_free( the_addr );

			return;

		}
	}
}

/* We now have a single address in the_addr to output.
 */
munge( the_addr )
AP_ptr the_addr;
{
	char adr[ LINESIZE ];
	char *s, *frees;
	AP_ptr local, domain, route;
	char *ap_p2s();

	if( debug )
		printf( "in munge with: " );

	/* Find where the important bits begin in the tree */
	ap_t2parts( the_addr, (AP_ptr *)0, (AP_ptr *)0,
			&local, &domain, &route );

	/* Convert from the tree back into a string */
	frees = s = ap_p2s( (AP_ptr)0, (AP_ptr)0, local, domain, route );

	if( debug )
		printf( "%s\n", s );

	/* Is it a uucp style address? */
	if ( domain == (AP_ptr) 0  && index (s, '!') != (char *) 0) {
		char adr2[ LINESIZE ];

		strcpy( adr, s );

		/* Stick the path it took to get here at the start.
		 * But try to avoid any duplicates by overlapping
		 * the matching parts of the address. Eg:
		 *  adr = 'x!y!z'  path = 'a!b!x!y!'
		 *  adr2 = 'a!b!x!y!z'
		 */
		for( s = origpath; *s; s++ ){
			/* Does it match? */
			if( strncmp( adr, s, strlen( s ) ) == 0 ){
				char c = *s;
				*s = '\0';
				strcpy( adr2, origpath );
				strcat( adr2, adr );
				*s = c;
				break;
			}
		}

		/* Did I just scan the whole path without finding a match! */
		if( *s == '\0' ){
			/* Append the adr to the path */
			strcpy( adr2, origpath );
			strcat( adr2, adr );
		}

		/* Munge the address into its shortest form and print it */
		fromcvt( adr2, adr );
		fprintf( rm_msgf, "%s", adr );

		if( debug )
			printf( "uucp munge gives %s\n", adr );
	}
	else {
		char *p;
		char *at;
		char *official;
		char *get_official();
		char *fmt;
		char *brace;
		AP_ptr norm;
		extern int ap_outtype;

		/* Normalise the address */
		ap_outtype = AP_733;    /* Hmm. Maybe should be from channel */
		norm = ap_normalize( nextdoor, (char *) 0, the_addr, chanptr );

		/* Convert it back in to a string and output it */
		ap_t2s( norm, &p );
		if (debug)
			printf("Normalised address: %s\n", p);
		/* Look for the last address component and re-write to the */
		/* official form. There are probably better ways of doing this */
		at = rindex(p, '@');
		fmt = "%s";
		if (at)
		{	brace = index(at+1, '>');
			if (brace) *brace = '\0';
			official = get_official(at+1, (int *)0);
			if (official)
			{	*at = '\0';
				fmt = brace ? "%s@%s>" : "%s@%s";
			}
			else if (brace)
				*brace = '>';
		}
		fprintf( rm_msgf, fmt, p , official);

		if( debug )
		{	printf( "arpa munge gives ");
			printf( fmt, p, official );
			printf( "\n");
		}

		free( p );
	}
	free( frees );
}



/*
 *      datecvt()  --  convert UNIX ctime() style date to ARPA style
 *                      (change done in place)
 */
datecvt (date, newdate)
char *date;
char *newdate;
{
	/* LMCL: Changed the default timezone, when none given, to be GMT */
	/*      012345678901234567890123456789          */
	/*      Wed Nov 18 22:23:17 1981        Unix    */
	/*      Wed, 18 Nov 81 22:23:45 GMT     ARPA    */
	/* or
	/*      Wed Nov 18 22:23:17 EDT 1981    Unix    */
	/*      Wed, 18 Nov 81 22:23:45 EDT     ARPA    */
	/* or even (LMCL yet again) */
	/*      Mon Apr 23 16:50 BST 1984       Unix    */

	if (isdigit(date[0]) || date[3] == ',' || isdigit(date[5]))
		strcpy (newdate, date);         /* Probably already ARPA */
	else if (isdigit(date[20]))
		sprintf( newdate, "%.3s, %.2s %.3s %.2s %.2s:%.2s:%.2s GMT",
		    date, date+8, date+4, date+22, date+11, date+14, date+17);
	else if (isalpha(date[17]))     /* LMCL. Bum Unix format */
		sprintf( newdate, "%.3s, %.2s %.3s %.2s %.2s:%.2s:00 GMT",
		    date, date+8, date+4, date+23, date+11, date+14 );
	else
		sprintf( newdate, "%.3s, %.2s %.3s %.2s %.2s:%.2s:%.2s %.3s",
		    date, date+8, date+4, date+26,
		    date+11, date+14, date+17, date+20);
}

/**/

msg_return (addr, dest, reason)
char *addr, *dest, *reason;
{
	ml_init (YES, NO, "UUCP (rmail)", "Failed Message");
	ml_adr (addr);
	ml_aend ();
	ml_tinit();
	ml_txt ("Your message was not delivered to:\n    ");
	ml_txt (dest);
	ml_txt ("\nFor the following reason:\n    ");
	ml_txt (reason);
	ml_txt ("\n\n--------------- Returned Mail ---------------\n\n");
	rewind (rm_msgf);
	ml_file (rm_msgf);
	ml_txt ("--------------- End of Returned Mail -------------\n");
	if (ml_end (OK)  != OK)
	{
		ml_init (YES, NO, "UUCP (rmail)", "Failed Err Message");
		ml_adr (supportaddr);
		ml_aend ();
		ml_tinit();
		ml_txt ("Error message failed to:\n    ");
		ml_txt (addr);
		ml_txt ("\n\nWhen failing to send to:\n    ");
		ml_txt (dest);
		ml_txt ("\nFor the following reason:\n    ");
		ml_txt (reason);
		ml_txt ("\n\n----------- Returned Mail --------------\n\n");
		rewind (rm_msgf);
		ml_file (rm_msgf);
		ml_txt ("-------- End of Returned Mail ---------------\n");
		if (ml_end (OK)  != OK)
		    bomb ("Failed to return error message to overseer");
	}
}

/*  */

xsubmit( from, argv, argc)
char *from;
char **argv;
int argc;
{
	char subargs[LINESIZE];
	char buf[LINESIZE];
	struct rp_bufstruct thereply;
	register char *p, *q;
	int len;

	rewind (rm_msgf);

	if( debug ){
		for( argv++; --argc > 0; argv++ )
			printf( "arg = %s\n", *argv );
		while( fgets( buf, LINESIZE-1, rm_msgf ) != NULL )
			printf( "T=%s", buf );
		if( ferror( rm_msgf ) )
			bomb( "Error reading messge file" );

		exit( 0 );
	}

	if (rp_isbad (mm_init()) || rp_isbad (mm_sbinit ()))
		bomb ("Failed to initialise submit");

	sprintf(subargs, "lvmti%s*h", uchan);

	if (nextdoor[0]) {
		q = subargs + strlen(subargs);
		for (p = nextdoor; *p; ) {
			if (!isprint(*p)) {
				p++;
				continue;
			}
			if (*p == '\'') *q++ = '\\';
			*q++ = *p++;
		}
		*q = '\0';
	}
	strcat(subargs, "*");

	if (rp_isbad (mm_winit ((char *) 0, subargs, from)))
		bomb ("mm_winit (%s, %s) failed", subargs, from);
	if (rp_isbad (mm_rrply (&thereply, &len)))
		bomb ("Failed to read address reply");
	if (rp_isbad (thereply.rp_val))
		bomb ("Initialization failure '%s'", thereply.rp_line);

	for (argv++; --argc > 0; argv++ )
	{
	    char *name;

	    name = *argv;
	    if (rp_isbad (mm_wadr("", *argv)))
		bomb ("failed to write address '%s'", name);

	    if (rp_isbad (mm_rrply (&thereply, &len)))
		bomb ("Failed to read address reply");

	    switch (rp_gval(thereply.rp_val))
	    {
		case RP_AOK:
		    break;
		case RP_USER:
		    msg_return (from, name, thereply.rp_line);
		    break;      /* LMCL: Was return */
		default:
		    bomb ("Address check failure '%s' (%s)", thereply.rp_line,
			rp_valstr (thereply.rp_val));
	    }
	}

	if (rp_isbad (mm_waend()))
	    bomb ("Failed to write address end");

	rewind (rm_msgf);
	while (fgets (buf, LINESIZE-1, rm_msgf) != NULL)
	    if (rp_isbad (mm_wtxt (buf, strlen (buf))))
		 bomb ("mm_wtxt failure");
	if (ferror (rm_msgf))
		bomb ("Error reading messge file");

	if (rp_isbad (mm_wtend ()))
		bomb ("mm_wtend error");

	if (rp_isbad (mm_rrply (&thereply, &len)))
		bomb ("mm_rrply at end of message");

	if (rp_isbad (thereply.rp_val))
		bomb ("Bad text reply val '%s' (%s)", thereply.rp_line,
				rp_valstr (thereply.rp_val));

	mm_sbend();
	return;
}


/*VARARGS1*/
bomb (fmt, a, b, c)
char *fmt, *a, *b, *c;
{
	ll_log (logptr, LLOGFAT, fmt, a, b, c);
	ll_close (logptr);
	fputs ("rmail: ", stderr);
	fprintf (stderr, fmt, a, b, c);
	putc ('\n', stderr);
	exit (99);
}

/*
 *      fromcvt()  --  trys to convert the UUCP style path into
 *                      an ARPA style name.
 */
fromcvt (from, newfrom)
char *from, *newfrom;
{
	register char *cp;
	register char *sp;
	register char *off;
	char	*at;
	char	*atoff;
	char	atstore[LINESIZE];
	char	buf[LINESIZE];
	char	*get_official();
	int	saveit;
	
	if( debug )
		printf( "fromcvt on %s\n", from );

	strcpy (buf, from);
	cp = rindex (buf, '!');
	if (cp == 0) {
		strcpy (newfrom, from);
		return;
	}
	/*
	 *	look for @site at the end of the name
	 */
	atoff = NULL;
	if ((at = index(cp, '@')) != NULL)
	{	/* got one - is it followed by a ! ? */
		if (index(at+1, '!') != NULL)
			 at = NULL;
		else
		{	/* look up the official name of the at site */
			atoff = get_official(at+1, &saveit);
			if (atoff)
			{	if (saveit)
				{	strcpy(atstore, atoff);
					atoff = atstore;
				}
			}
		}
	}
	*cp = 0;
	while( sp = rindex (buf, '!') )
	{	/*
		 * scan the path backwards looking for hosts that we
		 * know about
		 */
		if (off = get_official(sp+1, &saveit))
		{	if (atoff && lexequ(atoff, off))
				strcpy(newfrom, cp+1);
			else	sprintf(newfrom, "%s@%s", cp+1, off);
			return;
		}
		else
		if (at && lexequ(at+1, sp+1))
		{	strcpy(newfrom, cp+1);
			return;
		}
		*cp = '!';
		cp = sp;
		*cp = 0;
	}
	if (off = get_official(buf, (int *)0))
		sprintf(newfrom, "%s@%s", cp+1, off);
	else
		sprintf(newfrom, "%s@%s", cp+1, buf);
}

/*
 *	This piece added by Peter Collinson (UKC)
 *	rather than just making rmail submit into the uucp channel which
 *	is hard coded. First look up in a table 'rmail.chans' for entries
 *	of the form
 *	nextdoor:channel
 *	If the name exists then use that channel otherwise just default to
 *	the uucp channel. This is for authorisation purposes really
 */
set_channel(nxt)
char *nxt;
{	Table *tb_rmchans;
	char tuchan[LINESIZE];
#ifdef DEBUG
	ll_log(logptr, LLOGBTR, "set_channel (%s)", nxt);
#endif
	/* set default */
	strcpy(uchan, Uchan);
	
	if ((tb_rmchans = tb_nm2struct ("rmail.chans")) != (Table *) NOTOK)
	{	/* well we have the table */
		/* Extract name of current channel      */
		/* from list of channels          */
		if (tb_k2val (tb_rmchans, TRUE, nxt, tuchan) == OK)
			strcpy(uchan, tuchan);
	}

	/* Get the channel attributes */
	if (( chanptr = ch_nm2struct (uchan)) == (Chan *)NOTOK )
	{	/* fail safe - not sure that we should do this */
		/* find the default channel */
		if (( chanptr = ch_nm2struct (Uchan)) == (Chan *)NOTOK )
			bomb ("Cannot look up channel '%s'\n", uchan);
	}
}

/*
 *	Check if the from machine which we have constructed actually
 *	contains two references to the same machine of the form
 *	site.uucp!site.??.uk!something
 *	if so replace by the single official name
 */
domain_cross(route)
char *route;
{	char *second;
	char *endsecond;
	char *official;
	char *host_equal();
	
	second = index(route, '!');
	if (second == (char *)0)
		return;
	second++;
	endsecond = index(second, '!');
	if (endsecond == (char *)0)
		return;
	second[-1] = '\0';
	*endsecond = '\0';
	if (official = host_equal(route, second))
	{	strcpy(route, official);
		*endsecond = '!';
		strcat(route, endsecond);
	}
	else
	{	second[-1] = '!';
		*endsecond = '!';
	}
}

/*
 *	host_equal
 *	a routine to see whether the strings which are input
 *	are in fact the same host
 *	Returns the official name if they are and a null pointer if not
 */
char *
host_equal(n1, n2)
char *n1, *n2;
{	static char res1[LINESIZE];
	char	*o1, *o2;
	int	saveit;
	char	*get_official();
	
	o1 = get_official(n1, &saveit);
	if (o1 == NULL)
		return (NULL);
	if (saveit)
	{	strcpy(res1, o1);
		o1 = res1;
	}
	o2 = get_official(n2, &saveit);
	if (o2 == NULL)
		return (NULL);
	if (lexequ(o1, o2) == TRUE)
		return (o1);
	return (NULL);
}

/*
 *	These routines perform some caching on the host name/official
 *	name pair in and attempt to reduce overheads
 */
typedef struct
{	char	*o_host;		/* Host name */
	char	*o_official;		/* Official name */
} Official;

Official *off_cache;			/* pointer to the base of the cache */
#define OFFICIALSIZE	50		/* Chunk size for the cache */
int	off_size;			/* current size of the cache */
int	off_used;			/* current set which are used */
/*
 *	get a host from the domain tables
 *	if the cache has fallen over due to lack of memory
 *	then the saveit switch will be set showing that the
 *	address is in static memory
 */
char *
get_official(host, saveit)
char *host;
int *saveit;				/* set if the cache is not working */
{	register Official *off;
	static char res[LINESIZE];
	Domain *dmn;
	Dmn_route rbuf;
	Dmn_route *route = &rbuf;
	Domain *dm_v2route();
	int sadummy;
		
	if (debug)
		printf("get_official called for %s\n", host);
	if (saveit == (int *)0)
		saveit = &sadummy;
	*saveit = 0;			/* assume cache is working */
	
	if (off_size == 0)
	{	off_cache = (Official *)malloc(OFFICIALSIZE*sizeof(Official));
		off_size = OFFICIALSIZE;
		if (debug)
			printf("Cache%sset - %d items\n",
				off_cache ? " " : " not " ,
				off_cache ? off_size : 0 );
	}
	/*
	 * If we have managed to get space for the cache 
	 * search it for the name
	 */
	if (off_cache == (Official *)0)
		*saveit = 1;
	else
	{	for (off = off_cache; off < &off_cache[off_used]; off++)
			if (*host == *off->o_host && lexequ(host, off->o_host))
			{	if (debug)
					printf("Cache hit - %s\n", off->o_official);
				return (off->o_official);
			}
	}
	/*
	 * Bad news - we must do something to get the information
	 */
	dmn = dm_v2route(host, res, route);
	if (dmn == (Domain *) NOTOK)
	{	if (debug)
			printf("%s not in tables\n", host);
		return (NULL);
	}
	/*
	 * we have a hit in the tables
	 * save it in the cache
	 */
	if (off_used >= off_size)
	{	off_size += OFFICIALSIZE;
		/*NOSTRICT*/
		off_cache = (Official *)realloc(off_cache,off_size * sizeof(Official));
		if (debug)
		{	if (off_cache)
				printf("Cache grown to %d items\n", off_size);
			else	printf("Cache grow failed\n");
		}
	}
	if (off_cache)
	{	off = &off_cache[off_used];
		if ((off->o_host = strdup(host)) &&
		    (off->o_official = strdup(res)))
		{	off_used++;
			if (debug)
			{	printf("%s - %s stored in cache - cachesize %d\n",
					off->o_host,
					off->o_official,
					off_used);
			}
			return (off->o_official);
		}	
	}
	*saveit = 1;
	if (debug)
		printf("%s - %s not cached\n", host, res);
	return (res);
}
