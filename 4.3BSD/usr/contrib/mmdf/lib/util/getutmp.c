#include "util.h"
#include <utmp.h>

/* get informtion about who is logged in */

/* this is modeled after the getpw v7 set of routines */

LOCVAR char UTPATH[]  = "/etc/utmp";
LOCVAR FILE *utmpfp = NULL;
LOCVAR union tmpunion
{
    char        io[sizeof (struct utmp)];
    struct utmp entry;
}       utmp;

setutmp()
{
	if( utmpfp == NULL )
		utmpfp = fopen( UTPATH, "r" );
	else
		rewind( utmpfp );
}

endutmp()
{
	if( utmpfp != NULL ){
		fclose( utmpfp );
		utmpfp = NULL;
	}
}

struct utmp *
getutmp()
{
	if (utmpfp == NULL) {
		if( (utmpfp = fopen( UTPATH, "r" )) == NULL )
			return(0);
	}
	if (fread (utmp.io, sizeof (struct utmp), 1, utmpfp) != 1)
		return(0);
	return(&utmp.entry);
}

struct utmp *
getutnam(name)
char name[];
{
	register struct utmp *p;

	while( (p = getutmp()) && !equal(name,p->ut_name, strlen(name)));

	return(p);
}
