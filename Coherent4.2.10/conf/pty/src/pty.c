/* $Header: /ker/io.386/RCS/pty.c,v 2.4 93/08/19 10:39:20 nigel Exp Locker: nigel $ */
/*
 * Purpose:	pseudoterminal device driver
 *
 *	Master devices are named /dev/pty[pqrs][0-f].
 *	Corresponding slaves are /dev/tty[pqrs][0-f].
 *
 *	Minor numbers are 0..127 assigned in increasing order,
 *	plus 128 for master device.
 *
 *	Output written to master appears as input to slave and
 *	vice versa.  Data path is:
 *
 *		app	master		slave	line	app
 *		using	device	shunt	device	disc.	using
 *		master	driver		driver	module	slave
 *
 *      slave read does ttread() which is fed by ttin()
 *	master write does ttin()
 *
 *      slave write does ttwrite() which is fed by ttout()
 *	master read does ttout()
 *
 * $Log:	pty.c,v $
 * Revision 2.4  93/08/19  10:39:20  nigel
 * r83 ioctl (), corefile, new headers
 * 
 * Revision 2.3  93/08/19  04:03:06  nigel
 * Nigel's R83
 * 
 * Revision 2.2  93/07/26  15:32:12  nigel
 * Nigel's R80
 * 
 * Revision 1.4  93/04/14  10:12:10  root
 * r75
 * 
 * Revision 1.3  92/07/16  16:35:29  hal
 * Kernel #58
 * 
 * Revision 1.2  92/03/18  07:45:33  hal
 * master device polling added
 * 
 * Revision 1.1  92/03/16  12:57:31  hal
 * Initial revision
 */

#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/poll.h>
#include <sys/cmn_err.h>

#include <sys/coherent.h>
#include <sys/uproc.h>
#include <sys/proc.h>
#include <sys/tty.h>		/* indirectly includes sgtty.h */
#include <sys/con.h>
#include <sys/devices.h>
#include <sys/sched.h>		/* CVTTOUT, IVTTOUT, SVTTOUT */


#define channel(dev)	(dev & 0x7F)
#define master(dev)	(dev & 0x80)


/*
 * Explanation of p_mopen values:
 * 0 - master is closed
 * 1 - master open, waiting for slave to open
 * 2 - master and slave both open
 * 3 - master open, slave has closed
 */

typedef struct pty {
	TTY p_tp;
	event_t p_iev;
	event_t p_oev;
	char p_mopen;
	char p_asleep;	/* master is asleep in read or write awaiting slave */
	char p_ttwr;	/* slave is suspended in mid ttwrite() */
} PTY;

/*
 * Support functions (local functions).
 */
static void ptycycle();

/* Configurable variables - see /etc/conf/pty/Space.c. */
extern int NUPTY;

static PTY * p;

/*
 * ptystart()
 */
static void
ptystart(tp)
TTY * tp;
{
	int chan = (int)tp->t_ddp;
	PTY * pp = p + chan;

	if (chan >= 0 && chan < NUPTY) {
		if (pp->p_ttwr)
			wakeup (& pp->p_mopen);
	}
}

/*
 * ptyload()
 */
static void
ptyload()
{
	int i;
	TTY * tp;

	if ((p = (PTY *) kalloc (NUPTY * sizeof (PTY))) == 0) {
		cmn_err (CE_WARN, "ptyload: can't allocate %s pty's\n", NUPTY);
		return;
	}

	memset (p, 0, NUPTY * sizeof (PTY));

	for (i = 0 ; i < NUPTY ; i ++) {
		tp = & p [i].p_tp;
		tp->t_start   = ptystart;
		tp->t_param   = NULL;
		tp->t_ddp     = (char *) i;
	}
}

/*
 * ptyunload()
 */

static void
ptyunload()
{
	if (p)
		kfree(p);
}

/*
 * ptyopen()
 */

static void
ptyopen(dev, mode)
dev_t dev;
int mode;
{
	int chan = channel(dev);
	PTY * pp = p + chan;
	TTY * tp = & pp->p_tp;

	if (chan >= NUPTY) {
		set_user_error (ENXIO);
		return;
	}

	if (master (dev)) {
		if (pp->p_mopen) {
			set_user_error (EBUSY);
			return;
		}
		if (tp->t_open)
			pp->p_mopen = 2;
		else
			pp->p_mopen = 1;
		wakeup ((char *) & tp->t_open);
		ptycycle (chan);
		return;
	}

	/* If we get this far, we are opening a slave device. */

	tp->t_flags |= T_HOPEN | T_STOP;
	for (;;) {	/* wait for carrier */
		if (pp->p_mopen)
			break;

		/* PTY driver is waiting for carrier.  */
		if (x_sleep ((char *) & tp->t_open, pritty,
			     slpriSigCatch, "ptycd") == PROCESS_SIGNALLED) {
			set_user_error (EINTR);
			tp->t_flags &= ~(T_HOPEN | T_STOP);
			return;
		}
	}

	tp->t_flags |= T_CARR;
	tp->t_flags &= ~(T_HOPEN | T_STOP);
	ttopen (tp);

	/* Kluge to test why some X11 apps are confused. */
	/* Try forcing some kind of raw mode on first open. */
	if (tp->t_open == 0) {
		tp->t_sgttyb.sg_flags |= RAW;
		tp->t_sgttyb.sg_flags &= ~ECHO;

		tp->t_termio.c_iflag = 0;

		tp->t_termio.c_cflag &= ~ (PARODD | PARENB);
		tp->t_termio.c_cflag |= CS8 | CREAD;

		tp->t_termio.c_oflag &= ~ OPOST;

		tp->t_termio.c_lflag &=
		  ~ (ECHONL | ISIG | ICANON | XCASE | ECHO);
	}

	tp->t_open ++;
	ttsetgrp (tp, dev, mode);
	if (pp->p_mopen == 1 || pp->p_mopen == 3)
		pp->p_mopen = 2;
}

/*
 * ptyclose()
 */

static void
ptyclose (dev, mode)
dev_t dev;
int mode;
{
	int chan = channel(dev);
	PTY * pp = p + chan;
	TTY * tp = & pp->p_tp;

	if (chan >= NUPTY) {
		set_user_error (ENXIO);
		return;
	}

	if (master (dev)) {
		if (pp->p_mopen) {
			tp->t_flags &= ~ T_CARR;
			tthup (tp);
			pp->p_mopen = 0;
		}
		return;
	}

	if (-- tp->t_open == 0) {
		ttclose (tp);
		if (pp->p_mopen == 2)
			pp->p_mopen = 3;
		wakeup (& pp->p_mopen);
	}
}

/*
 * ptyread()
 */

static void
ptyread(dev, iop)
dev_t dev;
IO * iop;
{
	int chan = channel (dev);
	PTY * pp = p + chan;
	TTY * tp = & pp->p_tp;
	int c;

	if (master (dev)) {
		int char_read = 0;

		while (iop->io_ioc) {
			if ((c = ttout (tp)) == -1) {
				if (char_read) {
					ttstart (tp);
					return;
				}
				if (iop->io_flag & (IONDLY | IONONBLOCK)) {
					if (tp->t_group == 0 ||
					    (iop->io_flag & IONONBLOCK) != 0)
						set_user_error (EAGAIN);
					return;
				}
				if (pp->p_mopen == 3) {
					set_user_error (EIO);
					return;
				}
				ttstart (tp);
				pp->p_asleep = 1;
				/* The PTY driver is waiting for a read.  */
				if (x_sleep (& pp->p_mopen, pritty,
					     slpriSigCatch, "ptyread")
				    == PROCESS_SIGNALLED) {
					set_user_error (EINTR);
					return;
				}
			} else {
				ioputc (c, iop);
				char_read = 1;
			}
		}
		return;
	}

#if 0
	if (pp->p_asleep) {
		pp->p_asleep = 0;
		wakeup (& pp->p_mopen);
	}
	pollwake (& pp->p_oev);
	ttread (tp, iop);
#else
	pp->p_asleep = 0;	/* ttread0 will awaken the sleeper */
	ttread0 (tp,iop,wakeup, & pp->p_mopen, pollwake, & pp->p_oev);
#endif
}

/*
 * ptywrite()
 */

static void
ptywrite(dev, iop)
dev_t dev;
IO * iop;
{
	int chan = channel(dev);
	PTY * pp = p + chan;
	TTY * tp = &pp->p_tp;
	int c;

	if (master (dev)) {
		while (iop->io_ioc) {
			if (! ttinp (tp)) {
				if (iop->io_flag & (IONDLY | IONONBLOCK)) {
					set_user_error (EAGAIN);
					return;
				}
				if (pp->p_mopen == 3) {
					set_user_error (EIO);
					return;
				}
				pp->p_asleep = 1;

				/* The PTY driver is waiting for a write.  */
				if (x_sleep (& pp->p_mopen, pritty,
					     slpriSigCatch, "ptywrite")
				    == PROCESS_SIGNALLED) {
					set_user_error (EINTR);
					return;
				}
			}

			c = iogetc (iop);
			ttin (tp, c);
		}
		wakeup (& pp->p_mopen);
	} else {
		pp->p_ttwr = 1;
		ttwrite0 (tp, iop, wakeup, & pp->p_mopen, pollwake,
			  & pp->p_iev);
		pp->p_ttwr = 0;
	}
}

/*
 * ptyioctl()
 */

static void
ptyioctl(dev, com, vec)
dev_t	dev;
int	com;
struct sgttyb *vec;
{
	int chan = channel (dev);
	PTY * pp = p + chan;
	TTY * tp = & pp->p_tp;

	if (master (dev)) {
		set_user_error (EINVAL);
		return;
	}

	ttioctl (tp, com, vec);
}


/*
 * ptypoll()
 */

static int
ptypoll(dev, ev, msec)
dev_t dev;
int ev;
int msec;
{
	int chan = channel(dev);
	PTY * pp = p + chan;
	TTY * tp = &pp->p_tp;

	if (! master (dev))
		return ttpoll (tp, ev, msec);

	/*
	 * Priority polls not supported.
	 */
	ev &= (POLLIN | POLLOUT);

	/*
	 * Input poll with no data present.
	 */
	if ((ev & POLLIN) != 0 && ttoutp(tp) == 0) {

		/*
		 * Blocking input poll.
		 */
		if (msec != 0)
			pollopen (& pp->p_iev);

		/*
		 * Second look to avoid interrupt race.
		 */

		if (ttoutp (tp) == 0)
			ev &= ~POLLIN;
	}

	/*
	 * Output poll with no space.
	 */

	if ((ev & POLLOUT) != 0 && ttinp (tp) == 0) {
		/*
		 * Blocking output poll.
		 */

		if (msec != 0)
			pollopen (& pp->p_oev);

		/*
		 * Second look to avoid interrupt race.
		 */

		if (ttinp (tp) == 0)
			ev &= ~ POLLOUT;
	}

	return ev;
}


/*
 * ptycycle()
 *
 * Do a wakeup of any sleeping pty's at regular intervals.
 */

static void
ptycycle(chan)
int chan;
{
	PTY * pp = p + chan;
	TTY * tp = &pp->p_tp;

	/*
	 * Do wakeups.
	 */

	if (pp->p_asleep || pp->p_ttwr) {
		wakeup (& pp->p_mopen);
		pollwake (& pp->p_oev);
	}

	/*
	 * Schedule next cycle.
	 */
	if (pp->p_mopen)
		timeout (& tp->t_rawtim, HZ / 10, ptycycle, chan);
}


/*
 * Configuration table (export data).
 */

CON ptycon = {
	DFCHR | DFPOL,			/* Flags */
	PTY_MAJOR,			/* Major index */
	ptyopen,			/* Open */
	ptyclose,			/* Close */
	NULL,				/* Block */
	ptyread,			/* Read */
	ptywrite,			/* Write */
	ptyioctl,			/* Ioctl */
	NULL,				/* Powerfail */
	NULL,				/* Timeout */
	ptyload,			/* Load */
	ptyunload,			/* Unload */
	ptypoll				/* Poll */
};

