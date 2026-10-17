#include "util.h"
#include <pwd.h>
#include <utmp.h>
#include <lastlog.h>

/* get informtion about who is logged in */

extern char *blt();

LOCVAR char LLPATH[]  = "/usr/adm/lastlog";
LOCVAR FILE *llogfp = NULL;
LOCVAR struct utmp llogentry;
LOCVAR int lloguid;       /* keep track of current uid */

int setllog()
{
	if( llogfp == NULL )
		{
		llogfp = fopen( LLPATH, "r" );
		if ( llogfp == NULL )
			return(NOTOK);
		}
	else
		rewind( llogfp );

	lloguid = 0;
	return (OK);
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
	union tmpunion
	{
	    char        io[sizeof (struct lastlog)];
	    struct lastlog entry;
	}       llogbuf;
	struct passwd *pwdptr,
		      *getpwuid ();

	if (llogfp == NULL) {
		if( (llogfp = fopen( LLPATH, "r" )) == NULL )
			return(0);

		lloguid = 0;
	}

	if (fread (llogbuf.io, sizeof (struct lastlog), 1, llogfp) != 1)
		return(0);

	llogentry.ut_time = (long) llogbuf.entry.ll_time;
	blt (llogbuf.entry.ll_line, llogentry.ut_line, 8);

	if ((pwdptr = getpwuid (lloguid)) == 0)
	    strcpy (llogentry.ut_name, "????");
	else
	    blt (pwdptr -> pw_name, llogentry.ut_name, 8);

	lloguid++;
	return(&llogentry);
}

struct utmp *
getllnam(name)
char name[];
{
	struct passwd *pwdptr,
		      *gpwlnam ();

	if ((pwdptr = gpwlnam (name)) == 0)
	    return (0);

	if (setllog () == NOTOK)
		return(NULL);

	lloguid = pwdptr->pw_uid;
	fseek(llogfp, (long)pwdptr->pw_uid * sizeof (struct lastlog), 0);

	return (getllog ());
}
