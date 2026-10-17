#include "util.h"

/* Perform process start-up tailoring */

/*  This version is a No-Op.  It presumes that the tailoring
 *  information has been compiled-in and that an external
 *  file is not to be consulted.
 */

tai_init (filename)	/* prepare to get tailoring information */
	char filename[]; /* location of the info */
{
	return (OK);
}

tai_get (max, argv)	/* get next parsed tailoring line */
	int max;	/* maximum number of allowable fields */
	char *argv[];	/* array to hold pointers to field elements */
{
	return (0);    /* return end-of-list */
}

tai_end ()
{
	return (OK);
}
