# include  <stdio.h>
# include  "d_returns.h"

/*  maintain a transcript of data i/o over the port.  * the transcript is
 *  not buffered, so that it will reflect the * latest activity, for
 *  debugging.  dhc.
 */

static int  d_tsfd;           /*  file descriptor for transcript file  */

/*
 *     D_TSOPEN
 *
 *     this routine creates the transcript file.
 *
 *     filename -- name of the transcript file to be created.  if this arg
 *                 is zero, then the standard output is used.
 */

d_tsopen(filename)
  register char  *filename;
    {
    extern int  d_errno, errno;

    if (filename == 0)
      {
#ifdef D_LOG
      d_log("d_tsopen", "transcript started on standard output");
#endif D_LOG
      d_tsfd = 1;
      return(D_OK);
      }

    if ((d_tsfd = creat(filename, 0662)) < 0)
      {
#ifdef D_LOG
      d_log("d_tsopen", "can't create transcript file '%s'", filename);
#endif D_LOG
      d_errno = D_TSOPEN;
      return(D_FATAL);
      }

    chmod (filename, 0662);
#ifdef D_LOG
    d_log("d_tsopen", "transcript file '%s' created", filename);
#endif D_LOG
    return(D_OK);
    }

/**/
/*
 *     D_TSCLOSE
 *
 *     this routine is called to stop transcribing
 */

d_tsclose()
    {

/*  do nothing if transcribing is not active  */

    if (d_tsfd <= 0)
      return(D_OK);

    if (d_tsfd != 1)
      (void) close(d_tsfd);

#ifdef D_LOG
    d_log("d_tsclose", "transcript stopped");
#endif D_LOG

    return(D_OK);
    }


/*
 *     D_TSCRIBE
 *
 *     routine which writes on the transcript file
 *
 *     text -- pointer to byets to be written
 *
 *     length -- number of bytes to be written
 */

d_tscribe(text, length)
  register char  *text;
  register int  length;
    {
    extern int  d_errno;

/*  if there's a transcript file and something to write, do it  */

    if ((d_tsfd > 0) && (length > 0))
    {
	if (write(d_tsfd, text, length) != length)
	  {
#ifdef D_LOG
	  d_log("d_tscribe", "error writing on transcript file");
#endif D_LOG
	  d_errno = D_TSWRITE;
	  return(D_NONFATAL);
	  }
    }
    return(D_OK);
    }
