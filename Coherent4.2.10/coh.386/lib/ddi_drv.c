/* $Header: $ */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV3		1

/*
 * This file contains supplementary definitions used to deal with or aid the
 * implementation of the automatically generated device configuration code
 * in the "conf.c" file.
 *
 * Note that we are in the _SYSV3 world because we touch <sys/uproc.h>
 *
 * $Log: $
 */

#include <common/ccompat.h>
#include <common/xdebug.h>
#include <kernel/strmlib.h>
#include <sys/debug.h>
#include <sys/types.h>
#include <sys/cred.h>
#include <sys/cmn_err.h>
#include <sys/file.h>
#include <sys/open.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/poll.h>
#include <stddef.h>

#include <sys/confinfo.h>
#include <kernel/ddi_base.h>

/*
 * These are Coherent header files! Treat with all the caution you would
 * normally use for handling toxic waste!
 *
 *	<sys/stat.h>
 *		major ()
 *		minor ()
 *	<sys/con.h>
 *		DFBLK
 *
 * Note that the actual magic u area variable 'u' is extern'ed in
 * <kernel/ddi_base.h>
 */

#include <sys/stat.h>
#include <sys/inode.h>

/*
 * Convert the broken Coherent file-mode flags to something sane.
 */

#if	__USE_PROTO__
int MAKE_MODE (int mode)
#else
int
MAKE_MODE (mode)
int		mode;
#endif
{
	int		newmode = 0;

	/*
	 * These are the flags documented by the Coherent driver kit. In
	 * actual fact, there are more (corresponding to the DDI/DKI flags)
	 * that are passed down from the system file table's flag entry. The
	 * fact that the others are not documented probably reflects the fact
	 * that calls to dread () either pass only IPR or IPW directly, while
	 * the existence of the other flags can only be discovered by tracing
	 * many layers of calls.
	 */

	if ((mode & IPR) != 0)
		newmode |= FREAD;
	if ((mode & IPW) != 0)
		newmode |= FWRITE;

	/*
	 * And the other flags.
	 */

	if ((mode & IPEXCL) != 0)
		newmode |= FEXCL;
	if ((mode & IPNDLY) != 0)
		newmode |= FNDELAY;
	if ((mode & IPNONBLOCK) != 0)
		newmode |= FNONBLOCK;

	return newmode;
}


/*
 * This local function is called to build a "uio_t" structure from Coherent's
 * UIO structure. This function would be local, but it is called by the
 * STREAMS equivalent of the mapping routines.
 */

#if	__USE_PROTO__
uio_t * (MAKE_UIO) (uio_t * uiop, iovec_t * iovp, int mode, IO * iop)
#else
uio_t *
MAKE_UIO __ARGS ((uiop, iovp, mode, iop))
uio_t	      *	uiop;
iovec_t	      *	iovp;
int		mode;
IO	      *	iop;
#endif
{
	uiop->uio_iov = iovp;
	uiop->uio_iovcnt = 1;
	uiop->uio_offset = iop->io_seek;
	uiop->uio_segflg = iop->io_seg == IOSYS ? UIO_SYSSPACE : UIO_USERSPACE;
	if ((iop->io_flag & IONDLY) != 0)
		mode |= FNDELAY;
	if ((iop->io_flag & IONONBLOCK) != 0)
		mode |= FNONBLOCK;
	uiop->uio_fmode = mode;
	uiop->uio_resid = iop->io_ioc;

#ifdef	_I386
	iovp->iov_base = iop->io.vbase;
#else
	iovp->iov_base = iop->io_base;
#endif
	iovp->iov_len = iop->io_ioc;

	return uiop;
}


/*
 * This function is used to copy back the information from a "uio_t" structure
 * into a Coherent IO structure.
 */

#if	__USE_PROTO__
void (DESTROY_UIO) (uio_t * uiop, IO * iop)
#else
void
DESTROY_UIO __ARGS ((uiop, iop))
uio_t	      *	uiop;
IO	      *	iop;
#endif
{
	ASSERT (uiop->uio_iov != NULL);
	ASSERT (uiop->uio_iov->iov_len == uiop->uio_resid);

	iop->io_ioc = uiop->uio_resid;
	iop->io_seek = uiop->uio_offset;
#ifdef	_I386
	iop->io.vbase = uiop->uio_iov->iov_base;
#else
	iop->io_base = uiop->uio_iov->iov_base;
#endif
}


/*
 * This function is called upon by the machine-generated code in "conf.c" to
 * forward a Coherent device open request to a System V device.
 */

#if	__USE_PROTO__
void (_forward_open) (o_dev_t dev, int mode, int flags, cred_t * credp,
		      struct inode ** inodepp, ddi_open_t funcp,
		      ddi_close_t closep)
#else
void
_forward_open __ARGS ((dev, mode, flags, credp, inodepp, funcp, closep))
o_dev_t		dev;
int		mode;
int		flags;
cred_t	      *	credp;
struct inode **	inodepp;
ddi_open_t	funcp;
ddi_close_t	closep;
#endif
{
	n_dev_t		newdev = makedevice (major (dev), minor (dev));
	o_dev_t		clonedev;
	struct inode  *	temp;

	/*
	 * Character devices do not specifically support clone semantics.
	 */

	if ((mode & IPCLONE) != 0) {
		set_user_error (ENXIO);
		return;
	}

	flags = (flags & DFBLK) != 0 ? OTYP_BLK : OTYP_CHR;

	set_user_error ((* funcp) (& newdev, MAKE_MODE (mode), flags, credp));

	clonedev = makedev (getemajor (newdev), geteminor (newdev));
	if (clonedev != dev)
		return;

	if ((temp = inode_clone (* inodepp, clonedev)) != NULL) {
		* inodepp = temp;
		return;
	}

	/*
	 * We are out of in-core inode space.
	 */

	if (closep != NULL)
		(* closep) (newdev, MAKE_MODE (mode), flags, credp);
}


/*
 * This function is called upon by the machine-generated code in "conf.c" to
 * forward a Coherent device close request to a System V device.
 */

#if	__USE_PROTO__
void (_forward_close) (o_dev_t dev, int mode, int flags, cred_t * credp,
		       ddi_close_t funcp)
#else
void
_forward_close __ARGS ((dev, mode, flags, credp, funcp))
o_dev_t		dev;
int		mode;
int		flags;
cred_t	      *	credp;
ddi_close_t	funcp;
#endif
{
	flags = (flags & DFBLK) != 0 ? OTYP_BLK : OTYP_CHR;

	set_user_error ((* funcp) (makedevice (major (dev), minor (dev)),
				   MAKE_MODE (mode), flags, credp));
}


/*
 * This function is called upon by the machine-generated code in "conf.c" to
 * forward a Coherent device read request to a System V device.
 */

#if	__USE_PROTO__
void (_forward_read) (o_dev_t dev, IO * iop, cred_t * credp,
		      ddi_read_t funcp)
#else
void
_forward_read __ARGS ((dev, iop, credp, funcp))
o_dev_t		dev;
IO	      *	iop;
cred_t	      *	credp;
ddi_read_t	funcp;
#endif
{
	iovec_t		iov;
	uio_t		uio;

	set_user_error ((* funcp) (makedevice (major (dev), minor (dev)),
				   MAKE_UIO (& uio, & iov, FREAD, iop),
				   credp));
	DESTROY_UIO (& uio, iop);
}


/*
 * This function is called upon by the machine-generated code in "conf.c" to
 * forward a Coherent device write request to a System V device.
 */

#if	__USE_PROTO__
void (_forward_write) (o_dev_t dev, IO * iop, cred_t * credp,
		       ddi_write_t funcp)
#else
void
_forward_write __ARGS ((dev, iop, credp, funcp))
o_dev_t		dev;
IO	      *	iop;
cred_t	      *	credp;
ddi_write_t	funcp;
#endif
{
	iovec_t		iov;
	uio_t		uio;

	set_user_error ((* funcp) (makedevice (major (dev), minor (dev)),
				   MAKE_UIO (& uio, & iov, FWRITE, iop),
				   credp));
	DESTROY_UIO (& uio, iop);
}


/*
 * This function is called upon by the machine-generated code in "conf.c" to
 * forward a Coherent device ioctl request to a System V device.
 */

#if	__USE_PROTO__
void (_forward_ioctl) (o_dev_t dev, int cmd, _VOID * arg, int mode,
		       cred_t * credp, int * rvalp, ddi_ioctl_t funcp)
#else
void
_forward_ioctl __ARGS ((dev, cmd, arg, mode, credp, rvalp, funcp))
o_dev_t		dev;
int		cmd;
_VOID	      *	arg;
int		mode;
cred_t	      *	credp;
int	      *	rvalp;
ddi_ioctl_t	funcp;
#endif
{
	set_user_error ((* funcp) (makedevice (major (dev), minor (dev)),
				   cmd, arg, mode, credp, rvalp));
}



#if 0
/*
 * This function is called upon by the machine-generated code in "conf.c" to
 * forward a Coherent device block request to a System V device.
 */

#if	__USE_PROTO__
int (_forward_strategy) (BUF * buf, ddi_strategy_t funcp)
#else
int
_forward_strategy __ARGS ((buf, funcp))
BUF	      *	buf;
ddi_strategy_t	funcp;
#endif
{
	cmn_err (CE_PANIC, "DDI/DKI block drivers not yet supported");
}
#endif


/*
 * This function is called upon by the machine-generated code in "conf.c" to
 * forward a Coherent device poll request to a System V device.
 */

#if	__USE_PROTO__
int (_forward_poll) (o_dev_t dev, int events, int msec, ddi_chpoll_t funcp)
#else
int
_forward_poll __ARGS ((dev, events, msec, funcp))
o_dev_t		dev;
int		events;
int		msec;
ddi_chpoll_t	funcp;
#endif
{
	short		revents;

	if (msec == 0) {
		/*
		 * Simply check to see what events are outstanding.
		 */

		set_user_error ((* funcp) (makedevice (major (dev),
						       minor (dev)),
					   events, 1, & revents, NULL));
		return revents;
	}

	set_user_error ((* funcp) (makedevice (major (dev), minor (dev)),
				   events, 0, & revents,
				   get_next_phpp (events)));
	install_phpp ();
	return revents;
}
