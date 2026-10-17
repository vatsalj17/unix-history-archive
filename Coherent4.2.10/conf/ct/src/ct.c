/* $Header: /ker/io.386/RCS/ct.c,v 2.3 93/08/19 04:02:18 nigel Exp Locker: nigel $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 2.3.37
 *	Copyright (c) 1982, 1983, 1984.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * Coherent
 * Console terminal driver.
 *
 * $Log:	ct.c,v $
 * Revision 2.3  93/08/19  04:02:18  nigel
 * Nigel's R83
 * 
 * Revision 2.2  93/07/26  15:28:02  nigel
 * Nigel's R80
 * 
 * Revision 1.2  92/01/06  12:26:48  hal
 * Compile with cc.mwc.
 * 
 * Revision 1.1	88/03/24  16:18:09	src
 * Initial revision
 * 
 * 86/11/19	Allan Cornish		/usr/src/sys/drv/ct.c
 * Added support for System V.3 compatible polls.
 */

#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/cred.h>

#include <sys/coherent.h>
#include <sys/uproc.h>
#include <sys/con.h>
#include <sys/proc.h>

/*
 * Open.
 */

#if	__USE_PROTO__
__LOCAL__ void ctopen (dev_t dev, int mode, int flags, cred_t * credp,
		       struct inode ** inodepp)
#else
__LOCAL__ void
ctopen (dev, mode, flags, credp, inodepp)
dev_t		dev;
int		mode;
int		flags;
cred_t	      *	credp;
struct inode **	inodepp;
#endif
{
	dev_t ttdev;

	if ((ttdev = SELF->p_ttdev) == NODEV) {
		set_user_error (ENXIO);
		return;
	}
	* inodepp = dopen (ttdev, mode, flags, * inodepp);
}


/*
 * Close.
 */

#if	__USE_PROTO__
__LOCAL__ void ctclose (dev_t dev, int mode, int flags, cred_t * credp,
			__VOID__ * private)
#else
__LOCAL__ void
ctclose (dev, mode, flags, credp, private)
dev_t		dev;
int		mode;
int		flags;
cred_t	      *	credp;
__VOID__      *	private;
#endif
{
	dclose (SELF->p_ttdev, mode, flags, private);
}


/*
 * Read.
 */

#if	__USE_PROTO__
__LOCAL__ void ctread (dev_t dev, IO * iop, cred_t * credp,
		       __VOID__ * private)
#else
__LOCAL__ void
ctread (dev, iop, credp, private)
dev_t 		dev;
IO	      *	iop;
cred_t	      * credp;
__VOID__      *	private;
#endif
{
	dread (SELF->p_ttdev, iop, private);
}


/*
 * Write.
 */

#if	__USE_PROTO__
__LOCAL__ void ctwrite (dev_t dev, IO * iop, cred_t * credp,
			__VOID__ * private)
#else
__LOCAL__ void
ctwrite (dev, iop, credp, private)
dev_t		dev;
IO	      *	iop;
cred_t	      *	credp;
__VOID__      *	private;
#endif
{
	dwrite (SELF->p_ttdev, iop, private);
}


/*
 * Ioctl.
 */

#if	__USE_PROTO__
__LOCAL__ void ctioctl (dev_t dev, int com, _VOID * arg, int mode,
			cred_t * credp, int * rvalp, __VOID__ * private)
#else
__LOCAL__ void
ctioctl (dev, com, arg, mode, credp, rvalp, private)
dev_t		dev;
int		com;
_VOID	      *	arg;
int		mode;
cred_t	      *	credp;
int	      *	rvalp;
__VOID__      *	private;
#endif
{
	* rvalp = dioctl (SELF->p_ttdev, com, arg, mode, private, NULL);
}


/*
 * Poll.
 */

#if	__USE_PROTO__
__LOCAL__ int ctpoll (dev_t dev, int events, int msec, __VOID__ * private)
#else
__LOCAL__ int
ctpoll (dev, events, msec, private)
dev_t 		dev;
int		events;
int		msec;
__VOID__      *	private;
#endif
{
	return dpoll (SELF->p_ttdev, events, msec, private);
}


/*
 * Configuration table.
 */

CON ctcon = {
	DFCHR | DFPOL,			/* Flags */
	1,				/* Major index */
	(driver_open_t) ctopen,		/* Open */
	(driver_close_t) ctclose,	/* Close */
	NULL,				/* Block */
	(driver_read_t) ctread,		/* Read */
	(driver_write_t) ctwrite,	/* Write */
	(driver_ioctl_t) ctioctl,	/* Ioctl */
	NULL,				/* Powerfail */
	NULL,				/* Timeout */
	NULL,				/* Load */
	NULL,				/* Unload */
	(driver_poll_t) ctpoll		/* Poll */
};
