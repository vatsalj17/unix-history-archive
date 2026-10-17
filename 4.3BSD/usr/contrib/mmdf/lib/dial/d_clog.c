# include  "util.h"
# include  "d_clogcodes.h"
# include  "d_returns.h"
# include  <stdio.h>
# include  "d_proto.h"
# include  "d_structs.h"

extern	time_t	time();
extern	char	*ctime();
extern	char	*d_calllog;
extern	char	d_uname[];
extern	int	d_errno;

static time_t d_cstime;                /*  time call started  */

static char  d_cnumber[30];           /*  the number called  */
static char  d_linetype[30];          /*  the type of dialout line used */

/*
 *     D_CSTART
 *
 *     routine called to note the beginning of a call.  the current time is
 *     saved, and if the phone number and user name are given, they are
 *     saved also.
 *
 *     number -- character string which is phone number called
 */

d_cstart(number, linetype)
  register char  *number;
  register char  *linetype;
    {

/*  get the time and save the number if given  */

    time(&d_cstime);

    if (number)
      (void) strcpy(d_cnumber, number);

    if (linetype)
      (void) strcpy(d_linetype, linetype);

    return(D_OK);
    }
/**/

/*
 *     D_CLOG
 *
 *     routine which logs calls.  we have to be careful here that
 *     sensitive numbers are protected, but still allow us to see all
 *     long distance calls.  the strategy that's used is to write X's
 *     over the last 4 digits of number which have more than 7 digits.
 *
 *     code -- a character that is written in the file to indicate the
 *             status of call completion
 *     
 */

d_clog(code)
  int  code;
    {
    FILE  *clogfptr;
    long duration;
    long  stoptime;
    int  seconds;
    register int  hours, minutes;

/* check start time--if zero, this is an extraneous call to d_clog */
    if (d_cstime == 0L) return;


/*  open the call log file  */

    if ((clogfptr = fopen(d_calllog, "a")) == NULL)
      {
      d_errno = D_CALLLOG;
      d_log("d_clog", "can't open call log file"); 
      return;
      }
  
/*  get the stop time and calculate the duration of the session  */

    time(&stoptime);
    duration = stoptime - d_cstime;
    /*NOSTRICT*/
    d_cstime = 0L;
    /*NOSTRICT*/
    hours = (int) (duration / 3600);
    /*NOSTRICT*/
    minutes = (int) ((duration - (hours * 3600)) / 60l);
    /*NOSTRICT*/
    seconds = (int) (duration - (hours * 3600) - (minutes * 60));

    fprintf(clogfptr, "%-8s  %c  ", d_uname, code);
    fprintf(clogfptr, "%-16.16s  ", d_cnumber);

/*  now the time data.  considerable craziness to make a pretty duration  */

    fprintf(clogfptr, "%-19.19s  ", ctime(&stoptime));

    if (hours > 0)
      fprintf(clogfptr, "%2d:", hours);
    else
      fprintf(clogfptr, "   ");

    if (minutes > 0)
      fprintf(clogfptr, "%2d:", minutes);
    else
      if (hours > 0)
        fprintf(clogfptr, "00:", minutes);
      else
        fprintf(clogfptr, " 0:", minutes);

    if (seconds > 9)
      fprintf(clogfptr, "%2d   %s\n", seconds, d_linetype);
    else
      fprintf(clogfptr, "0%1d   %s\n", seconds, d_linetype);

    fclose(clogfptr);
    }
