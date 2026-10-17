#
# include  <stdio.h>
# include  "d_proto.h"
# include  "d_returns.h"
# include  "d_structs.h"

# define	MAXFILES	15

extern char *strcpy ();
extern char *strdup ();
extern unsigned d_scline;
extern char d_scfile[];
extern int d_nfields;
extern char *d_fields[];
extern int errno;

FILE  *d_scfp;            /*  file pointer for script file  */
static struct opfile *d_files[MAXFILES];
static int d_nopen;

/*
 *     D_SCOPEN
 *
 *     this routine opens the script file and sets up a few globals for
 *     use by later routines.
 *
 *     scriptfile -- path name of the script file to be used.
*/

d_scopen (scriptfile, nfields, fields)
  int nfields;
  char scriptfile[], *fields[];
    {
    char *calloc();
    register int index, i;

    /*  Don't let them open so many files that they overrun my array  */
    if (d_nopen > MAXFILES)
    {
#ifdef D_LOG
	d_log ("d_scopen", "Too many files opened");
#endif D_LOG
	return (D_FATAL);
    }

    /*  If this is any other than the first file to be opened,
     *  then I alloc space for it and save status info.
     */
    if (d_nopen != 0)
    {
	index = d_nopen - 1;
	/*NOSTRICT*/
	d_files[index] = (struct opfile *)calloc (1, sizeof (struct opfile));
	if (d_files[index] == 0)
	{
	    d_log ("d_scopen", "couldn't alloc space for struct");
	    return (D_FATAL);
	}
	d_files[index]->o_line = d_scline;
	d_files[index]->o_chan = d_scfp;
	(void) strcpy (d_files[index]->o_fname, d_scfile);
	d_files[index]->o_nfields = d_nfields;
	for (i=0; i < d_nfields; i++)
	{
	    d_files[index]->o_fields[i] = d_fields[i];
	    free(d_fields[i]);
	}
	d_dbglog ("d_scopen", "Saving %d fields; loading %d new ones", d_nfields, nfields);
	for (i=0; i < nfields; i++)
	    d_fields[i] = strdup(fields[i+1]);
	d_nfields = nfields-1;

    }
    d_nopen++;

    if ((d_scfp = fopen(scriptfile, "r")) == NULL)
    {
#ifdef D_LOG
	d_log ("d_scopen", "Can't open script file '%s' (errno %d)",
						scriptfile, errno);
#endif D_LOG
	/*  restore the old file setup incase this was part of a failed
	 *  alternate.
	 */
	d_scclose();
	return (D_FATAL);
    }

    d_scline = 0;
    (void) strcpy (d_scfile, scriptfile);

#ifdef D_LOG
    d_log ("d_scopen", "script file '%s' being used", scriptfile);
#endif D_LOG

    return (D_OK);
}

d_scclose()
    {
    register int index, i;

    if (d_nopen <= 0)
	return (0);


    fclose (d_scfp);
#ifdef D_LOG
    d_log ("d_scclose", "Closing script file '%s'", d_scfile);
#endif D_LOG
    if (--d_nopen > 0)
    {
	index = d_nopen - 1;
	d_scfp = d_files[index]->o_chan;
	(void) strcpy (d_scfile, d_files[index]->o_fname);
	d_scline = d_files[index]->o_line;
	d_nfields = d_files[index]->o_nfields;
	for (i = 0; i < d_nfields; i++)
	    d_fields[i] = d_files[index]->o_fields[i];
	free (d_files[index]);
#ifdef D_LOG
	d_log ("d_scclose", "Resuming use of script file '%s'", d_scfile);
#endif D_LOG
	return (d_nopen);
    }

    return (0);
}



/*
 *     D_SCRGETLINE
 *
 *     this routine is called to read a line in from the script file.  it
 *     handles the possibility of an escaped newline.
 *
 *     linebuf -- place to load the line
 */

d_scgetline(linebuf, channel)
  char  *linebuf;
  FILE *channel;
    {
    register int  c, count;
    register char *cp;

    count = 0;
    cp = linebuf;
    d_scline++;
    while ((c = getc(channel)) != EOF)
    {
	if (c == '\n')
	{                   /*  if newline, see if it's escaped  */
	    if ((cp > linebuf) && (cp[-1] == '\\'))
	    {
		cp--;
		count--;
		d_scline++;
	        continue;
	    }
	    break;
        }

	if (++count > MAXSCRLINE)
	{                   /*  line is too long */
	    d_scerr("line too long");
	    return(D_FATAL);
        }

	*cp++ = c;
    }

    /*  The line parsing routine actually needs two nulls at the
     *  end to guarentee that it doesn't skip it.
     */
    *cp++ = '\0';
    *cp = '\0';
    return(count);
}
