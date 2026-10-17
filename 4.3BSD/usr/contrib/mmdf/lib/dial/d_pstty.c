# include  "util.h"
#ifdef SYS5
# include <termio.h>
#else
# include  <sgtty.h>
#endif SYS5
# include  <sys/stat.h>
# include  "d_proto.h"
# include  "d_returns.h"
# include  "d_structs.h"

/*
 *  Routines for controlling the tty settings on the control line
 *
 *  Jun 81  D. Crocker      Rewritten to work on v6 & v7
 *  Jul 81  D. Crocker      Make explicit TIOCHPCL call
 *  Dec 82  Doug Kinston    Chmods no longer control return values (DPK@BRL)
 *  Jan 84  D. Rockwell     Make d_ttrestore heed d_savalid
 *                          Save d_savfd instead of FILE *
 */

extern int errno;

LOCVAR int d_savfd;            /*  handle on the opened tty          */
LOCVAR char d_savfname[25];      /*  name of tty                       */
#ifdef SYS5
LOCVAR struct termio d_savtty
#else
LOCVAR struct sgttyb d_savtty;   /*  where to stash original setting   */
#endif SYS5
LOCVAR int d_savmode;            /*  protections on tty                */
LOCVAR int d_savalid;


d_ttsave (fp, fname)              /* save the current setting           */
	FILE *fp;
	char fname[];
{
    struct stat statbuf;
    register int retval;

    d_savfd = fileno(fp);
    if (fname != 0)
	(void) strcpy (d_savfname, fname);
    else
	d_savfname[0] = '\0';

#ifdef D_DBGLOG
    d_dbglog("d_ttsave", "saving terminal modes, fd = %d", d_savfd);
#endif D_DBGLOG

#ifdef SYS5
    if ((retval = ioctl (d_savfd, TCGETA, &d_savtty)) >= 0)
#else
    if ((retval = ioctl (d_savfd, TIOCGETP, &d_savtty)) >= 0)
#endif SYS5
    {
	if ((retval = fstat (d_savfd, &statbuf)) >= 0)
	{
	    d_savalid++;
	    d_savmode = statbuf.st_mode & ~S_IFMT;
	}
    }
    return (retval);
}


d_ttrestore ()                    /*  restore initial setting           */
{
    int retval;

#ifdef D_DBGLOG
    d_dbglog ("d_ttrestore", "restoring terminal characteristics, fd = %d",
	d_savfd);
#endif D_DBGLOG

    if (d_savalid == 0) {
#ifdef D_DBGLOG
	d_dbglog("d_ttrestore", "didn't have to");
#endif D_DBGLOG
    	return(0);
    }
#ifdef SYS5
    retval = ioctl (d_savfd, TCSETAW, &d_savtty);
#else
    retval = ioctl (d_savfd, TIOCSETP, &d_savtty);
#endif
    if (!isnull (d_savfname[0]))
	chmod( d_savfname, d_savmode );

    d_savalid = 0;
    return (retval);
}

d_ttscript (baudrate)             /* tty will be following call script  */
    int baudrate;
{
#ifdef SYS5
    struct termio sttybuf;
    extern unsigned short d_scbitc, d_scbiti, d_scbito, d_scbitl;
#else
    struct sgttyb sttybuf;
    extern int d_scon, d_scoff;
#endif SYS5

    if (!isnull (d_savfname[0]))
	chmod( d_savfname, 0600 );

#ifdef SYS5
    ioctl(d_savfd, TCGETA, &sttybuf);
#endif SYS5

    if (baudrate != 0)
    {                             /*  use special speed                 */
#ifdef SYS5
	sttybuf.c_cflag = (baudrate & CBAUD);
#else
	sttybuf.sg_ispeed = baudrate;
	sttybuf.sg_ospeed = baudrate;
#endif SYS5
    }
    else
    {                             /*  use current speed                 */
#ifdef SYS5
	ssybuf.c_cflag = (d_savtty.c_cflag & CBAUD);
#else
	sttybuf.sg_ispeed = d_savtty.sg_ispeed;
	sttybuf.sg_ospeed = d_savtty.sg_ospeed;
#endif SYS5
    }

#ifdef SYS5
    sttybuf.c_cc[VERASE] = '\010';	/* erase control-h */
    sttybuf.c_cc[VKILL] = '\030';	/* kill control-x */
    sttybuf.c_cc[VTIME] = 0;		/* delay */
    sttybuf.c_cc[VMIN] = 1;		/* hi water mark */

    sttybuf.c_cflag |= d_scbitc;
    sttybuf.c_iflag = d_scbiti;
    sttybuf.c_oflag = d_scbito;
    sttybuf.c_lflag = d_scbitl;

#else
    sttybuf.sg_erase  = '\010';   /* erase = control-H                  */
    sttybuf.sg_kill   = '\030';   /* kill = control-X                   */
    sttybuf.sg_flags  |=  d_scon;
				  /* force on  special bits for script  */
    sttybuf.sg_flags  &=  ~((int) d_scoff);
				  /* force off special bits for script  */
#endif SYS5

#ifdef SYS5
    return (ioctl (d_savdf, TCSETAW, &sttybuf));
#else
    return (ioctl (d_savfd, TIOCSETP, &sttybuf));
#endif SYS5
}


d_ttproto (baudrate)
    int baudrate;
{
#ifdef SYS5
    struct termio sttybuf;
    extern unsigned short d_prbitc, d_prbiti, d_prbito, d_prbitl;
#else
    struct sgttyb sttybuf;
    extern int d_pron, d_proff;
#endif SYS5

    if (!isnull (d_savfname[0]))
	chmod( d_savfname, 0600 );

#ifdef SYS5
    ioctl (d_savfd, TCGETA, &sttybuf);
#endif

    if (baudrate != 0)
    {                             /*  use special speed                 */
#ifdef SYS5
	sttybuf.c_cflag = (baudrate & CBAUD);
#else
	sttybuf.sg_ispeed = baudrate;
	sttybuf.sg_ospeed = baudrate;
#endif SYS5
    }
    else
    {                             /*  use current speed                 */
#ifdef SYS5
	sttybuf.c_cflags = (d_savtty.c_cflags & CBAUD);
#else
	sttybuf.sg_ispeed = d_savtty.sg_ispeed;
	sttybuf.sg_ospeed = d_savtty.sg_ospeed;
#endif SYS5
    }

#ifdef SYS5
    sttybuf.c_cc[VERASE] = '\010';	/* erase control-h */
    sttybuf.c_cc[VKILL] = '\030';	/* kill control-x */
    sttybuf.c_cc[VEOL] = '\n';
    sttybuf.c_cc[VEOF] = '\004';

    sttybuf.c_cflag |= d_prbitc;
    sttybuf.c_iflag = d_prbiti;
    sttybuf.c_oflag = d_prbito;
    sttybuf.c_lflag = d_prbitl;

#else

    sttybuf.sg_erase  = '\010';   /* erase = control-H                  */
    sttybuf.sg_kill   = '\030';   /* kill = control-X                   */
    sttybuf.sg_flags  |= d_pron;  
				  /* force on  special bits for protocol*/
    sttybuf.sg_flags  &=  ~((int) d_proff);
				  /* force off special bits for protocol*/

#endif SYS5

#ifdef SYS5
    return( ioctl (d_savfd, TCSETAW, &sttybuf) );
#else
    return( ioctl (d_savfd, TIOCSETP, &sttybuf) );
#endif SYS5
}
