/* msgstrip:  remove lines of control-A and all message header text       */

#include "util.h"
#include "mmdf.h"

extern char *delim1;

FILE *tmpout,
     *filin;
int msgnum;
int filnum;

char    tmpfil[] = ",msgstrip";

char tmpstr[64];
char    buf[512];
char doingheader;
char hadtext;


main (argc, argv)
int     argc;
char   *argv[];
{
    char  *thefile;

    mmdf_init (argv[0]);
    for (argc--; filnum++ < argc; )
    {                             /* process a message file/folder        */
	printf ("%s: ", thefile = argv[filnum]);
	if ((tmpout = fopen (tmpfil, "w")) == NULL)
	    endit ("Unable to creat temp");

	if ((filin = fopen (thefile, "r")) == NULL)
	{
	    printf ("Unable to open file\n");
	    continue;
	}

	for (msgnum = 1; !feof (filin); )
	{                         /* process a message                    */
	    for (doingheader = TRUE, hadtext = FALSE;  ; )
		switch (fgets (buf, sizeof buf, filin))
		{                 /* process a line                       */
		    case NULL:
			if (ferror (filin))
			    printf ("Error reading during header skip\n");
			goto nxtmsg;

		    default:
			if (doingheader && buf[0] == '\n')
			{   doingheader = FALSE;
			    continue;
			}         /* drop on through                      */
			if (strcmp (buf, delim1) == 0)
			    goto nxtmsg;
			if (doingheader)
			    continue;
			hadtext = TRUE;
			fputs (buf, tmpout);
		}
nxtmsg:
	    if (hadtext)
		printf ("%d, ", msgnum++);
	}

	fflush (tmpout);
	if (ferror (tmpout))
	    endit ("Error writing");
	sprintf (tmpstr, ",%s", thefile);

	if (link (thefile, tmpstr) < 0 &&
		unlink (tmpstr) &&
		link (thefile, tmpstr) < 0)
	    endit ("Unable to link to backup");
	if (unlink (thefile) < 0)
	    endit ("Unable to unlink file");
	if (link (tmpfil, thefile) < 0)
	    endit ("Unable to link temporary into old file name");
	if (unlink (tmpfil) < 0)
	    endit ("Unable to unlink temporary file");
	sprintf (tmpstr, ".{%s", thefile);
	unlink (tmpstr);          /* get rid of parsefile                 */
	printf ("ok\n");

	fclose (tmpout);
	fclose (filin);
    }
}

endit (str)
char    str[];
{
    printf ("%s\n", str);
    fflush (stdout);
    exit (-1);
}
