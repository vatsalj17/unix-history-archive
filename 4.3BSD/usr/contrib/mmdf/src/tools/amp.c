/*
 * This program acts as a filter which reformats message headers.
 * Normally the standard input is copied to the standard output,
 * with errors indicated by non-zero exit status.
 *      Arguments can be given, with the following meanings:
 *              % amp <channame> <output file> <top debug> <low debug>
 *      The 1st argument if present is taken to be a channel name
 *              to normalize with respect to.  If this is -, the
 *              NULL channel will be used.
 *      The 2nd argument if present specifies output file to use
 *              instead of standard output; useful when debugging.
 *              "-" will get std output anyway.
 *      A 3rd argument, if present, turns on top-level debug output,
 *              which always goes to the std output.
 *      A 4th argument, if present, turns on low-level debug output.
 *
 *      Hacked 12Mar81 by KLH @ SRI.  Uses Dave Crocker's RFC733
 *      address parsing package.  No warranty, express or implied,
 *      shall apply to this horrible kludge.
 */

#include <stdio.h>
#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include "ap.h"

#if DEBUG > 1
extern char tdebug;                   /* Top-level debug flag */
extern char debug;                /* Low-level debug flag */
#endif

extern LLog     *logptr;
extern int ap_outtype;
extern Chan *ch_nm2struct();
extern long amp_hdr();

/**/

main (argc, argv)
int     argc;
char   *argv[];
{
    register int   c;
    int     ofd;
    FILE * outf;
    Chan *chanptr;

    mmdf_init (argv[0]);
    if (argc < 2 || (chanptr = ch_nm2struct(argv[1])) == (Chan *)NOTOK) {
	chanptr = (Chan *)NULL;
	ap_outtype = AP_822;
    } else
	ap_outtype = chanptr->ch_apout;

    if (argc < 3 || (argv[2][0] == '-' && argv[2][1] == '\0')
	    || ((ofd = creat ("amp.out", 0400)) < 0))
	outf = stdout;            /* Output to std output */
    else
	outf = fdopen (ofd, "w");

#if DEBUG > 1
    if (argc >= 4)
	tdebug++;                 /* Top-level debugging */
    if (argc == 5)
	debug++;                  /* Low-level debugging */
#endif
    if (argc == 5)
	logptr->ll_level = LLOGFTR;

    switch (amp_hdr (chanptr, stdin, outf)) {
    case NOTOK:
    case MAYBE:
	fprintf (stderr, "\nReformat failed.\n");
	exit (-1);
    default:
	/* Reformat succeeded, Copy message text */
	while ((c = getchar ()) != EOF)
	    putc (c, outf);
    }
    exit (0);
}
