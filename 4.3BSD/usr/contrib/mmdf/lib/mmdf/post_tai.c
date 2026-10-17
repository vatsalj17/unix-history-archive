#include "util.h"
#include "mmdf.h"

extern LLog *logptr;

/*      fake garbage collection of unused external tailor info  */

post_tai (argc, argv)
	int argc;
	char *argv[];
{
    char errline[LINESIZE];

    arg2vstr (0, sizeof errline, errline, argv);
    ll_log (logptr, LLOGGEN, "Unprocessed tailor entry '%s'", errline);

    return (YES);   /* we like everthing */
}
