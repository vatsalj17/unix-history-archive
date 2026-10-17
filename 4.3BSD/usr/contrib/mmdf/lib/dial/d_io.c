#
#include	<stdio.h>

/*  This is a set of routines that is called to read characters from
 *  the tty line.  It stores them up so that they may be "replayed"
 *  if necessary.  The characters are stored in a circular buffer.
*/

/*  The following functions are implemented:
 *
 *	getc - returns 1 character from the appropriate place.
 *	mark - designates the next character to be read as the one
 *	       to start with if a replay is requested.
 *	replay - further calls to d_getc() will read characters from
 *		 the buffer until it is exhausted.
 *
 *
 *	Added the select call to adapt to 4.2's signal mechanism.
 *		DRockwell@BBN 31 Jan 84
 *	Backed out above change in favor of the s_io library.
 *		DRockwell@BBN 1 Mar 84
*/

#define		BUFFSIZE		256

static int nleft;	/*  number of chars remaining in this replay	*/
static int current;	/*  index of current char in this replay	*/
static int last;	/*  next position to insert character at	*/
static int nchars;	/*  total number of chars in buffer.  Note that */
			/*  this stays the same throughout a replay.	*/
static int first;	/*  index of the first character in the buffer  */
static int replay;	/*  set if we are doing a replay		*/

#ifdef V4_2BSD
static int rbuff[BUFFSIZE];
#else
static char rbuff[BUFFSIZE];
#endif

d_getc (channel)
register FILE *channel;
{
#ifdef V4_2BSD
    register int c;
#else
    char c;
#endif


    /*  Check to see if we are replaying the input  */
    if (replay != 0)
    {
	if (nleft == 0)
	{
	    replay = 0;
	    return (d_getc (channel));
	}

	c = rbuff[current];
	current = indexinc (current);
	nleft--;

	/*  Be careful not to return an EOF from the queue.  They
	 *  get in here when an alarm call from the timeout code
	 *  interrupts a read.  However, they should clearly not
	 *  be returned from the queue, as they may not represent
	 *  the current state of things.
	 */
	if (c == EOF)
	    return (d_getc (channel));
    }
    else
    {
	if (nchars == BUFFSIZE)
	    /*  Have run out of buffer space.  Rather then leave
	     *  a partial line, which could screw up 'rdport',
	     *  delete the first line in the buffer
	     */
	    skpline();
	else
	    nchars++;
	c = getc (channel);

	rbuff[last] = c;
	last = indexinc (last);
    }

    return (c);
}


d_replay ()
    {

    replay = 1;
    current = first;
    nleft = nchars;

}


d_mark ()
    {

    replay = 0;
    nchars = 0;
    first = 0;
    last = 0;
}


static indexinc (index)
  int index;
    {

    if (index == (BUFFSIZE-1))
	return (0);
    else
	return (index + 1);
}


static skpline ()
    {

    while ((rbuff[first] != '\n') &&
	   (nchars > 0          )   )
    {
	first++;
	nchars--;
    }

    if (nchars <= 0)
	d_mark ();
    else
    {
	first++;
	nchars--;
    }
}
