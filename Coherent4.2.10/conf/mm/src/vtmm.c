/* $Header: /ker/io.386/RCS/vtmm.c,v 2.3 93/08/19 04:03:35 nigel Exp Locker: nigel $ */
/*
 * Memory Mapped Video
 * High level output routines.
 *
 * $Log:	vtmm.c,v $
 * Revision 2.3  93/08/19  04:03:35  nigel
 * Nigel's R83
 * 
 * Revision 2.2  93/07/26  15:33:01  nigel
 * Nigel's R80
 * 
 * Revision 1.2  92/07/16  16:35:31  hal
 * Kernel #58
 * 
 * Revision 1.4  92/04/09  10:25:38  hal
 * Call mmgo() from mmstart() at low priority.
 */

#include <sys/errno.h>
#include <sys/stat.h>

#include <kernel/timeout.h>
#include <sys/coherent.h>
#include <sys/uproc.h>
#include <sys/sched.h>
#include <sys/io.h>
#include <sys/tty.h>

#include <sys/kb.h>
#include <sys/vt.h>

/* For beeper */
#define	TIMCTL	0x43			/* Timer control port */
#define	TIMCNT	0x42			/* Counter timer port */
#define	PORTB	0x61			/* Port containing speaker enable */
#define	FREQ	((int)(1193181L/440))	/* Counter for 440 Hz. tone */
extern int con_beep;			/* 1=beep 0=silent */

int vtmmtime();

char vtmmbeeps;		/* number of ticks remaining on bell */
int  vtmmcrtsav = 1;	/* crt saver enabled */
int  vtmmvcnt   = 900;	/* seconds remaining before crt saver is activated */

extern TTY **vttty;

/*
 * Start the output stream.
 * Called from `ttwrite' and `isrint' routines.
 */
TIM vtmmtim;

vtmmstart(tp)
register TTY *tp;
{
	int c, s;
	IO iob;
	static int mmbegun;

	if (mmbegun == 0) {
		++mmbegun;
		timeout(&vtmmtim, HZ/10, vtmmtime, (char *)tp);
	}

	while ((tp->t_flags&T_STOP) == 0) {
		if ((c = ttout(tp)) < 0)
			break;
		iob.io_seg  = IOSYS;
		iob.io_ioc  = 1;
		iob.io.vbase = &c;
		iob.io_flag = 0;
#if 0
		vtmmwrite( ((VTDATA *)tp->t_ddp)->vt_dev, &iob );
#else
		s = splo();
		vtmmgo(&iob, tp->t_ddp, ((VTDATA *)(tp->t_ddp))->vt_ind);
		spl(s);
#endif
	}
}

vtmmtime(xp)
char *xp;
{
	register int s;
	register VTDATA *vp = (VTDATA *)((TTY *)xp)->t_ddp;

	s = sphi();

	if (con_beep) {
		if (vtmmbeeps < 0) {
			vtmmbeeps = 2;

			/* Timer 2, lsb, msb, binary */
			outb (TIMCTL, 0xB6);
			outb (TIMCNT, FREQ & 0xFF);
			outb (TIMCNT, FREQ >> 8);

			/* Turn speaker on */
			outb (PORTB, inb (PORTB) | 3);
		} else if (vtmmbeeps > 0 && -- vtmmbeeps == 0)
			outb (PORTB, inb (PORTB) & ~ 3);
	}


	if (vp->vmm_esc) {
		ismmfunc(vp->vmm_esc);
		vp->vmm_esc = 0;
	}
	spl(s);

	ttstart( (TTY *) xp );

	timeout(&vtmmtim, HZ/10, vtmmtime, xp);
}

/**
 *
 * void
 * mmwatch()	-- turn video display off after 15 minutes inactivity.
 */
void
vtmmwatch()
{
	if ( (vtmmcrtsav > 0) && (vtmmvcnt > 0) && (--vtmmvcnt == 0) ) {
		vtmm_voff(vtdata[vtactive]);
	}
}

vtmmwrite( dev, iop )
dev_t dev;
register IO *iop;
{
	int ioc;
	TTY *tp = vttty [vtindex(dev)];

	if (tp == NULL)
		printf ("mmwrite: bad dev %x", dev);

	/*
	 * Kernel writes.
	 */
	if (iop->io_seg == IOSYS) {
		while (vtmmgo (iop, tp->t_ddp, vtindex (dev)))
			/* DO NOTHING */ ;
		return;
	}

#if 0
	ioc = iop->io_ioc;
	/*
	 * Blocking user writes.
	 */
	if ( (iop->io_flag & IONDLY) == 0 ) {
		do {
			while (tp->t_flags & T_STOP) {
				s = sphi ();

				tp->t_flags |= T_HILIM;
				sleep ((char *) & tp->t_oq,
					CVTTOUT, IVTTOUT, SVTTOUT);
				spl (s);
			}
			if (curr_signal_pending ()) {
				/*
				 * Signal received.
				 */

				kbunscroll ();	/* update kbd LEDS */

				/*
				 * No data transferred yet.
				 */

				if (ioc == iop->io_ioc)
					set_user_error (EINTR);
				else {
					/*
					 * Transfer remaining data
					 * without pausing after scrolling.
					 */
					while (vtmmgo (iop, tp->t_ddp,
						       vtindex (dev)))
						/* DO NOTHING */ ;
				}
				return;
			}

			vtmmgo(iop, tp->t_ddp, vtindex(dev));
		} while (iop->io_ioc);
		return;
	}

	/*
	 * Non-blocking user writes with output stopped.
	 */

	if ((tp->t_flags & T_STOP) != 0) {
		set_user_error (EAGAIN);
		return;
	}

	/*
	 * Non-blocking user writes do not pause after scrolling.
	 */

	while (vtmmgo (iop, tp->t_ddp, vtindex (dev)))
		/* DO NOTHING */ ;
#else
	ttwrite (tp, iop);
#endif
}

/******************************************************************************
*
* The following routines are called by deferred isr's, i.e., no sleep() calls 
* allowed 
*
*******************************************************************************/

/*
 * update the screen to match vtactive
 */

vtupdscreen(index)
int index;
{
	register int pos, s;
	VTDATA *vp;

	vp = vtdata [index];
	pos = vp->vmm_voff;
	PRINTV( "update screen@%x {%d @%x|",
		vp->vmm_port, index, pos );

	s = sphi();

	/* update base of video memory */
	outb (vp->vmm_port, 0xC);
	outb (vp->vmm_port + 1, pos >> (8 + 1));
	outb (vp->vmm_port, 0xD);
	outb (vp->vmm_port + 1, pos >> (0 + 1));

	/* update the cursor */
	pos += vp->vmm_pos;

	pos |= vp->vmm_invis;		/* Mask cursor, if set */
	outb (vp->vmm_port, 0xE);
	outb (vp->vmm_port + 1, pos >> (8 + 1));
	outb (vp->vmm_port, 0xF);
	outb (vp->vmm_port + 1, pos >> (0 + 1));

	spl (s);
	PRINTV( "%x}\n", pos );
}
