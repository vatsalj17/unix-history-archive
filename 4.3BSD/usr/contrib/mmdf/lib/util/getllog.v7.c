#include "util.h"
#include <utmp.h>

/* get informtion about who is logged in */

/* this is modeled after the getpw v7 set of routines */

LOCVAR char LLPATH[]  = "/usr/adm/lastlog";
LOCVAR FILE *llogfp = NULL;
LOCVAR union tmpunion
{
    char        io[sizeof (struct utmp)];
    struct utmp entry;
}       llogentry;

setllog()
{
	if( llogfp == NULL )
		llogfp = fopen( LLPATH, "r" );
	else
		rewind( llogfp );
}

endllog()
{
	if( llogfp != NULL ){
		fclose( llogfp );
		llogfp = NULL;
	}
}

struct utmp *
getllog()
{
	if (llogfp == NULL) {
		if( (llogfp = fopen( LLPATH, "r" )) == NULL )
			return(0);
	}
	if (fread (llogentry.io, sizeof (struct utmp), 1, llogfp) != 1)
		return(0);
	return(&llogentry.entry);
}

struct utmp *
getllnam(name)
char name[];
{
	register struct utmp *p;

	while( (p = getllog()) && !equal(name,p->ut_name, strlen(name)));

	return(p);
}
