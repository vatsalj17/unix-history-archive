# include  <stdio.h>
# include  "d_returns.h"


/*
 *     D_CHKACCESS
 *
 *     this routine checks for the user name given as the argument string
 *     in the access file given as the second argument.
 *
 *     username -- the user's name that we're looking for
 *
 *     accessfile -- path name of the access file.  this file has one user name
 *                   per line
 */

d_chkaccess(username, accessfile)
char  *username, *accessfile;
{
    extern int  d_errno;
#ifdef D_LOG
    register int  linenum = 0;
#endif D_LOG
    register int  length;
    char  *fields[1], linebuf[128];
    FILE  *accfp;

#ifdef D_DBGLOG
    d_dbglog("d_chkaccess", "user %s accessfile %s", username, accessfile);
#endif D_DBGLOG

/*  open the access list file.  if we can't, just return quietly  */

    if ((accfp = fopen(accessfile, "r")) == NULL)
      return(0);

    while (fgets(linebuf, sizeof linebuf - 1, accfp) != NULL)
    {
#ifdef D_LOG
	linenum++;
#endif D_LOG
/*  make sure we got the whole line  */
	length = strlen(linebuf);
	if (linebuf[length - 1] != '\n')
	{
#ifdef D_LOG
            d_log("d_chkaccess", "access file '%s', line %d too long",
		accessfile, linenum);
#endif D_LOG
        d_errno = D_ACCERR;
        return(D_FATAL);
        }

/*  parse the line  */

      switch (d_linparse(fields, linebuf, 1))
        {
        case  0:
            continue;

        case  1:
            if (strcmp(username, fields[0]) == 0)
              {
	      fclose(accfp);
              return(D_OK);
              }
            continue;

        case  -1:
#ifdef D_LOG
            d_log("d_chkaccess", "access file '%s', line %d: too many fields",
                accessfile, linenum);
#endif D_LOG
            d_errno = D_ACCERR;
            return(D_FATAL);

        case  -2:
#ifdef D_LOG
            d_log("d_chkaccess",
                "access file '%s', line %d: qupted string too long", accessfile,
                linenum);
#endif D_LOG
            d_errno = D_ACCERR;
            return(D_FATAL);

        default:
#ifdef D_LOG
            d_log("d_chkaccess", "access file '%s', line %d: unknown error",
                accessfile, linenum);
#endif D_LOG
            d_errno = D_ACCERR;
            return(D_FATAL);
        }
      }

#ifdef D_DBGLOG
    d_dbglog("d_chkaccess", "name not found");
#endif D_DBGLOG
    fclose(accfp);
    return(D_NO);

}


/*
 *     D_TYPELIST
 *
 *     this routine returns a list of acceptable dial port types based on the
 *     given phone number.  
 *
 *     Currently, the type is gotten out of the phone number and the
 *     phone number is munged to exclude the type field.
 *
 *     numptr -- pointer to the phone number being called
 *      
*/

char **
d_typelist(numptr)
  char  **numptr;
    {

    extern char *strdup ();
    char tempbuf[25];
    char *tpt, *newnum, *number;
    static char *typelist[] = {
        "LOCAL",
	0
        };

    tpt = tempbuf; 
    number = *numptr;
    newnum = number;

#ifdef D_DBGLOG
    d_dbglog("d_typelist", "number %s", *numptr);
#endif D_DBGLOG

    if (d_instring('<', number) != 0)
       {
       while (*number)
          if (*number != '<')
             *newnum++ = *number++;
          else
             { /* pick off type */
             while (*++number != '>')
                *tpt++ = *number; 
             number++;
             }

       *newnum = '\0';
       }

    *tpt = '\0';

#ifdef D_DBGLOG
    d_dbglog("d_typelist", "new number %s", *numptr);
#endif D_DBGLOG

    if (tempbuf[0] != '\0')
       typelist[0] = strdup(tempbuf);

#ifdef D_DBGLOG
    d_dbglog("d_typelist", "returning type %s", typelist[0]);
#endif D_DBGLOG

    return(typelist);
}
