#include <stdio.h>
#include "ll_log.h"

/*          Augment ll_io routines to print system call errno value        */

extern int  sys_nerr;
extern char *sys_errlist[];
extern int  errno;
extern char *sprintf ();

/* VARARGS3 */
ll_err (loginfo, level, fmt, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9)
struct ll_struct  *loginfo;
int     level;
char   *fmt,
       *a0, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9;
{
    char newfmt[128];

    sprintf (newfmt,  "%s%s",  "[SYSERR (%d) %s]  ",  fmt);

    ll_log (loginfo,  level,  newfmt, errno,
	    (errno >= 0 && errno <= sys_nerr)
		? sys_errlist[errno]
		: "<- Illegal errno",
	    a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}
