/* $Header: */
/*
 * This file contains supplementary definitions used to deal with or aid the
 * implementation of the automatically generated device configuration code
 * in the "conf.c" file.
 *
 * Note that we are in the _SYSV3 world because we touch <sys/uproc.h>
 *
 * $Log: $
 */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV3		1

/*
 *-IMPORTS:
 *	<common/ccompat.h>
 *		__USE_PROTO__
 *		__ARGS ()
 *	<common/xdebug.h>
 *		__LOCAL__
 *	<kernel/ddi_base.h>
 *		DESTROY_UIO ()
 *		MAKE_CRED ()
 *		MAKE_UIO ()
 *	<sys/debug.h>
 *		ASSERT ()
 *	<sys/types.h>
 *		cred_t
 *		n_dev_t
 *		o_dev_t
 *		makedevice ()
 *	<sys/cmn_err.h>
 *		CE_NOTE
 *		cmn_err ()
 *	<sys/file.h>
 *		FREAD
 *		FWRITE
 *		FNDELAY
 *		FNONBLOCK
 *		FEXCL
 *	<sys/open.h>
 *		OTYP_BLK
 *		OTYP_CHR
 *		OTYP_LYR
 *	<sys/uio.h>
 *		iovec_t
 *		uio_t
 *	<sys/errno.h>
 *		ENXIO
 *	<sys/poll.h>
 *		POLLNVAL
 *	<stddef.h>
 *		NULL
 */

#include <common/ccompat.h>
#include <common/xdebug.h>
#include <kernel/ddi_base.h>
#include <kernel/strmlib.h>
#include <sys/debug.h>
#include <sys/types.h>
#include <sys/cmn_err.h>
#include <sys/file.h>
#include <sys/open.h>
#include <sys/uio.h>
#include <sys/errno.h>
#include <sys/poll.h>
#include <sys/cred.h>
#include <stddef.h>

#include <sys/confinfo.h>

/*
 * These are Coherent header files! Treat with all the caution you would
 * normally use for handling toxic waste!
 * <sys/coherent.h>.
 */

#include <sys/inode.h>


__EXTERN_C_BEGIN__

uio_t	      *	MAKE_UIO	__PROTO ((uio_t * uiop, iovec_t * iovp,
					  int mode, IO * iop));
void		DESTROY_UIO	__PROTO ((uio_t * uiop, IO * iop));
int		MAKE_MODE	__PROTO ((int oldmode));

__EXTERN_C_END__


/*
 * Forward Coh open request to actual STREAMS driver-request-processing code.
 */

#if	__USE_PROTO__
void (_streams_open) (o_dev_t dev, int mode, int __NOTUSED (flags),
		      cred_t * credp, struct inode ** inodepp,
		      struct streamtab * stabp)
#else
void
_streams_open __ARGS ((dev, mode, flags, credp, inodepp, stabp))
o_dev_t		dev;
int		mode;
int		flags;
cred_t	      *	credp;
struct inode **	inodepp;
struct streamtab
	      *	stabp;
#endif
{
	n_dev_t		newdev = makedevice (major (dev), minor (dev));
	o_dev_t		clonedev;
	int		cloneflag;
	struct inode  *	temp;
	shead_t	      *	sheadp;
	int		err;

	cloneflag = (mode & IPCLONE) != 0;

	err = STREAMS_OPEN (& newdev, stabp, MAKE_MODE (mode), credp,
			    cloneflag);

	if (err != 0) {
		set_user_error (err);
		return;
	}

	if ((sheadp = SHEAD_FIND (newdev,  DEV_SLIST)) == NULL)
		return;

	clonedev = makedev (getemajor (newdev), geteminor (newdev));
	if (! cloneflag && clonedev == dev) {

		(* inodepp)->i_private = sheadp;
		return;
	}

	cmn_err (CE_NOTE, "cloning...");

	if ((temp = inode_clone (* inodepp, clonedev)) != NULL) {
		temp->i_private = sheadp;
		* inodepp = temp;
		return;
	}

	/*
	 * We are out of in-core inode space.
	 */

	set_user_error (ENOMEM);

	STREAMS_CLOSE (sheadp, MAKE_MODE (mode), credp);
}


/*
 * Forward Coh close request to actual STREAMS driver-request-processing code.
 */

#if	__USE_PROTO__
void (_streams_close) (o_dev_t dev, int mode, int __NOTUSED (flags),
		       cred_t * credp, __VOID__ * private)
#else
void
_streams_close __ARGS ((dev, mode, flags, credp, private))
o_dev_t		dev;
int		mode;
int		flags;
cred_t	      *	credp;
__VOID__      *	private;
#endif
{
	set_user_error (STREAMS_CLOSE ((shead_t *) private, MAKE_MODE (mode),
				       credp));
}


/*
 * Forward Coh read request to actual STREAMS driver-request-processing code.
 */

#if	__USE_PROTO__
void (_streams_read) (o_dev_t dev, IO * iop, cred_t * credp,
		      __VOID__ * private)
#else
void
_streams_read __ARGS ((dev, iop, credp, private))
o_dev_t		dev;
IO	      *	iop;
cred_t	      *	credp;
__VOID__      *	private;
#endif
{
	iovec_t		iov;
	uio_t		uio;

	set_user_error (STREAMS_READ ((shead_t *) private,
				      MAKE_UIO (& uio, & iov, FREAD, iop),
				      credp));
	DESTROY_UIO (& uio, iop);
}


/*
 * Forward Coh write request to actual STREAMS driver-request-processing code.
 */

#if	__USE_PROTO__
void (_streams_write) (o_dev_t dev, IO * iop, cred_t * credp,
		       __VOID__ * private)
#else
void
_streams_write __ARGS ((dev, iop, credp, private))
o_dev_t		dev;
IO	      *	iop;
cred_t	      *	credp;
__VOID__      *	private;
#endif
{
	iovec_t		iov;
	uio_t		uio;

	set_user_error (STREAMS_WRITE ((shead_t *) private,
				       MAKE_UIO (& uio, & iov, FWRITE, iop),
				       credp));
	DESTROY_UIO (& uio, iop);
}


/*
 * Forward Coh ioctl request to actual STREAMS driver-request-processing code.
 */

#if	__USE_PROTO__
void (_streams_ioctl) (o_dev_t dev, int cmd, __VOID__ * arg, int mode,
		       cred_t * credp, int * rvalp, __VOID__ * private)
#else
void
_streams_ioctl __ARGS ((dev, cmd, arg, mode, credp, rvalp, private))
o_dev_t		dev;
int		cmd;
__VOID__      *	arg;
int		mode;
cred_t	      *	credp;
int	      *	rvalp;
__VOID__      *	private;
#endif
{
	set_user_error (STREAMS_IOCTL ((shead_t *) private, cmd, arg,
				       MAKE_MODE (mode), credp, rvalp));
}


/*
 * Forward Coh poll request to actual STRAAMS driver-request-processing code.
 */

#if	__USE_PROTO__
int (_streams_chpoll) (o_dev_t dev, int events, int msec,
		       __VOID__ * private)
#else
int
_streams_chpoll __ARGS ((dev, events, msec, private))
o_dev_t		dev;
int		events;
int		msec;
__VOID__      *	private;
#endif
{
	short		revents;

	if (msec == 0) {
		/*
		 * Simply check to see what events are outstanding.
		 */

		set_user_error (STREAMS_CHPOLL ((shead_t *) private, events,
						1, & revents, NULL));
		return revents;
	}

	set_user_error (STREAMS_CHPOLL ((shead_t *) private, events, 0,
					& revents, get_next_phpp (events)));
	install_phpp ();
	return revents;
}
