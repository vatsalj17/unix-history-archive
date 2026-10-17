#include "ded.h"
#include "char.h"
#include "tty.h"
#include "signal.h"

/* map of the screen - set up in the first place by
 * ttysetup
 */
char **rowmap 0, **auxscreen 0;
int nrows, ncols;

/* terminal characteristics */
int ttytype;
char c_MODIFY 0, c_BLOB 0, c_CONTROL 0;

/* mode-remembering stuff */
int setup_done false;
int saveflags;

struct TTYMODE
 { char ispeed, ospeed, erase, kill;
    int flags;
    char nldelay, crdelay, tbdelay, vtdelay;
    char width, length;
    int brktab[8];
 } tmode;

struct TTYBUF ttybuf, dlogbuf, ttyinbuf;

ttyinterrupt()
 { signal(SIGINTR, ttyinterrupt);
    ttyzonk();
 }

ttyzonk()
 { ttyinbuf.n_in = 0;
    ttyinbuf.nxfree = ttyinbuf.tbuf;
    complain();
 }

ttyquit()
 {  signal(SIGQUIT, 1);
    ttyzonk();
    diag("do you really want a core dump?");
    if ( lcase(ttyin()) == 'y' )
     { ttyreset(); abort(); }
    else
     { diag("I'll let you off this time, then");
       signal(SIGQUIT, ttyquit);
       fixpos();
       ttyflush();
     }
 }

/* start up funny tty mode */
ttysetup()
 { ttybuf.n_in = 0;
    ttybuf.nxfree = ttybuf.tbuf;

    dlogbuf.n_in = 0;
    dlogbuf.nxfree = dlogbuf.tbuf;

    ttyinbuf.n_in = 0;
    ttyinbuf.nxfree = ttyinbuf.tbuf;

    ttysetmode();
 }

ttysetmode()
 { register int row, width;
   register char oldmod;

    if (!setup_done)
    /* set up exotic tty modes */
     { gtty(2,&tmode);
	saveflags = tmode.flags;
	tmode.flags =& ~ (T_ITT | T_INDCTL | T_ECHO | T_TABS | T_CRLF);
	tmode.flags =| (T_RAW | T_RARE);
	ncols = 80; nrows = 24;
	tmode.width = tmode.length = 0;
     }

    stty(2,&tmode);
    setup_done=true;
    signal(SIGINTR, ttyinterrupt);
    signal(SIGQUIT, ttyquit);

    /* set up teletype characteristics */
    oldmod = c_MODIFY==0 ? '@' : c_MODIFY;

	switch (ttytype = TTY)
	{
	case itt:	c_BLOB = '{';
			c_CONTROL = '\032';
			set_modifier(oldmod);
			break;
	case hazeltine: c_BLOB = 0140;
			c_CONTROL = '^' + 0200;
			set_modifier(0);
			ictab[c_LEADIN] = ectab[c_LEADIN] = TWO;
			set_modifier(oldmod);
			break;
	case VC404:
	case T1061:	c_BLOB = '@';
			c_CONTROL = '\032';
			break;
	default:	editerror("ttytype not set up");
	}

    /* get space for screen of relevant size */
    if (rowmap==0)
     { if (nrows==0 || ncols==0)
	  editerror("what screen size do you have? (ttysetup)");
	else
	if (ncols+1>ENOUGH)
	  editerror("this screen is too big for ded (ttysetup)");
	else
	  rowmap = newscreen(); /* don't set up auxscreen yet */
     }
 }

ttyreset()
 { ttyflush(); dlogflush();

    if (setup_done)
     { tmode.flags = saveflags;
	tmode.width = ncols;
	tmode.length = nrows;

	stty(2,&tmode);
     }

    setup_done=false;
 }

/* stuff characters in buffer, to save too many system calls */
ttyout(c)
int c;
 {  register char *nxf;
    register int rc;

    if (ttybuf.n_in >= TTYBUFSIZE-10) ttyflush();

    if ((rc=c)==int_CONTROL && ttytype==hazeltine)
     { nxf = ttybuf.nxfree;
	*nxf++=c_LEADIN; *nxf++ = 037;
	*nxf++ = c_CONTROL;
	*nxf++ = c_LEADIN; *nxf++ = 031;
	ttybuf.nxfree = nxf;
	ttybuf.n_in =+ 5;
     }
    else
     { *(ttybuf.nxfree++)=rc; ttybuf.n_in++; }
 }

/* empty buffer */
ttyflush()
 { bufflush(&ttybuf, 2); }

dlogflush()
 { if (bufflush(&dlogbuf, dlogger) < 0)
      editerror("dlogwrite failed - not enough space?");
 }

bufflush(abuf, chan)
struct TTYBUF *abuf;
int chan;
 { register struct TTYBUF *rbuf;
    register int count;

    rbuf = abuf;
    if ((count = rbuf->n_in) != 0)
     { if (write(chan, rbuf->tbuf, count) != count) return(-1);
	rbuf->nxfree = rbuf->tbuf;
	rbuf->n_in=0;
     }
    return(1);
 }

/* a procedure to read in from the teletype - now also
 * provides an logfile (.dlog) facility
 */
int tty_input 2;
int dlogger;

char t1061map[4] { '\032','\037','\025','\010' };

ttyin()
 { register int count, input;
    int icount;
    register char c;

    input = tty_input;

    icount = (input==2 ? 1 : 512);
    if (--ttyinbuf.n_in < 0)
     { while ((count = read(input, ttyinbuf.tbuf, icount)) <= 0)
	if (input!=2)
	 { input=tty_input=2;    /* end of dlog input */
	     diag("** end of %s **", dlogname);
	     fixpos(); complain(); ttyflush();
	 }

	ttyinbuf.n_in = --count; ttyinbuf.nxfree = ttyinbuf.tbuf;
     }

    c = *(ttyinbuf.nxfree++);

	if ( c == '\033')
		if ( ttytype == T1061 )
		{	c = *(ttyinbuf.nxfree++);
			if ( c == 'H' ) c = '\036';
			else
			if ( (c >= 'A') || (c <= 'D'))
			c =t1061map[c-'A'];
		}
    if (input==2)
     { if (dlogbuf.n_in >= TTYBUFSIZE) dlogflush();
	*(dlogbuf.nxfree++) = c;
	dlogbuf.n_in++;
	if (c<040) dlogflush();
     }

    return(c);
 }
