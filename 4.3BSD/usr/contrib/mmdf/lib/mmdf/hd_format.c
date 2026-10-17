#include "util.h"
#include "mmdf.h"
#include "ch.h"

/* reformat addresses in the headers of a message       */

extern LLog *logptr;
extern long amp_hdr();
FILE *hd_fmtfp;         /* handle on the formatted header               */

LOCVAR FILE *hd_fpdup;  /*  stream pointers for original and alternate file*/
LOCVAR char hd_fn[] = "/tmp/am.hdrXXXXXX"; /* file for mapped headers */


long
	hd_init (chanptr, fd)   /* ready to process one message         */
    Chan *chanptr;              /* channel to NOT reformat refs to      */
    int fd;                     /* handle on file with message          */
{
    long hd_seek;
    char hd_cur[sizeof hd_fn];
    extern char *mktemp ();

/*  sri transform */
/*  begin special code for header translation, channel initialization phase
 *      (dhc) only call mktemp on the first time through this routine.
 *      the following assumes that x_fn is compile-time specified within
 *      this routine; otherwise, turn the sizeof into a strlen.
 *      a cleaner approach would be to keep the base string somewhere else
 *      and then strcpy it into x_fn and then call mktemp each time.
 */

    hd_seek = 0L;
    hd_fpdup = fdopen (dup (fd), "r");
				  /* open stream to msg text  */

    (void) strcpy (hd_cur, hd_fn);
    mktemp (hd_cur);   /*  create name for translated header file */

    if (close(creat(hd_cur, 0600)) < 0) {
	ll_err (logptr, LLOGTMP, "** '%s' hd_format file open err", hd_cur);
    } else if ((hd_fmtfp = fopen(hd_cur, "a+")) == NULL) {
	ll_err (logptr, LLOGTMP, "Header file re-open failure");
	hd_seek = 0L;
    } else {
	if ( (hd_seek = amp_hdr (chanptr, hd_fpdup, hd_fmtfp)) <= 0L ||
	      hd_seek == (long)MAYBE)
	{
	    ll_log (logptr, LLOGFAT, "Failure in header parse");
	    fclose (hd_fmtfp);      /* give it back                       */
	    hd_fmtfp = (FILE *) NULL;  /* signal no xlated file to read   */
	    if(hd_seek != (long)MAYBE)
		hd_seek = 0L;
	} else                      /* setup for the readings             */
	    fseek (hd_fmtfp, 0L, 0);
    }
    (void) unlink (hd_cur);        /*  Blow it away!!  */

    if (hd_fpdup != (FILE *) NULL)
    {
	fclose (hd_fpdup);         /* toss unneeded stream pointer     */
	hd_fpdup = (FILE *) NULL;
    }
    return (hd_seek);
}

hd_minit ()                         /* ready to process one copy        */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "hd_minit ()");
#endif

    if (hd_fmtfp != (FILE *) NULL)
	rewind (hd_fmtfp);                /* rewind xlated file */
}

hd_read (buf, max)
    char *buf;                      /* where to put the data            */
    int max;                        /* maximum size of buf              */
{
    int nread;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "hd_read ()");
#endif

/* sri transform */
/* special code for header translation, each address phase  */

    if (hd_fmtfp != (FILE *) NULL)  {
	/*
	 *  we have a translated file
	 */
	nread = fread (buf, sizeof (char), max, hd_fmtfp);
	if (nread == 0 && ferror(hd_fmtfp))
	    return (NOTOK);
	else
	    return (nread);
    }
    return (0);
}

hd_end ()
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "hd_end ()");
#endif

    if (hd_fmtfp != (FILE *) NULL)
    {
	fclose (hd_fmtfp);      /*  done reading xlated file          */
	hd_fmtfp = (FILE *) NULL;
    }
}
