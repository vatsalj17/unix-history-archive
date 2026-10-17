#include "util.h"
#include <sys/stat.h>

/* Perform process start-up tailoring */

extern int errno;

char *tai_eptr;         /* point to error text, if any  */

LOCVAR char *tai_data;	/* pointer to incore copy of tai file */
LOCVAR char *tai_ptr;   /* pointer to "next" tai record */

tai_init (filename)	/* prepare to get tailoring information */
	char filename[]; /* location of the info */
{
	extern char *malloc ();
	struct stat statbuf;
	unsigned taisize; /* number of bytes in the tai file */
	int fd;

	if (stat (filename, &statbuf) < OK)
		return (NOTOK);	 
	taisize = (unsigned) st_gsize (&statbuf);
	if ((tai_data = malloc (taisize + 1)) == 0)
		return (NOTOK);
	if ((fd = open (filename, 0)) < 0)
		return (NOTOK);
	if (read (fd, tai_data, taisize) != taisize)
	{
		(void) close (fd);
		errno = EIO;    /* i/o error */
		return (NOTOK);
	}
	tai_data[taisize] = '\0';
	(void) close (fd);
	tai_ptr = tai_data;
	return (OK);
}


tai_end ()
{
	tai_data = 0;
	return (OK);
}
/**/

tai_get (max, argv)	/* get next parsed tailoring line */
	int max;	/* maximum number of allowable fields */
	char *argv[];	/* array to hold pointers to field elements */
{
    int retval;
    register char *nxtptr;             /* beginning of next record     */

retry:
    if (tai_ptr == 0)
	    return (0);

    for (nxtptr = tai_ptr; *nxtptr != '\0'; nxtptr++)
	if (*nxtptr == '\n')
	{                       /* skip over to the next record */
	    *nxtptr = ' ';      /* no-op the newline            */
	    nxtptr++;
	    if (*nxtptr != ' ' && *nxtptr != '\t')
	    {                   /* continuation line            */
		nxtptr[-1] = '\0';
		break;
	    }
	}

    retval = str2arg (tai_ptr, max, argv, (char *) 0);
    if (retval < 0)
	tai_eptr = tai_ptr;        /* record where the error happened   */
    if (*nxtptr == '\0')
	tai_ptr = 0;
    else
	tai_ptr = nxtptr;

    if (retval == 0)                /* nothing but comments on line */
	goto retry;                 /* get something useful         */

    return (retval);
}
