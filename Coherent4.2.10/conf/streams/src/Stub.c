/*
 * This file contains the entry points into the STREAMS system called by the
 * Coherent kernel. In this file, they are all stubs, for use when other parts
 * of the kernel have had STREAMS support enabled but you really don't want
 * STREAMS.
 */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV3		1

#include <common/ccompat.h>
#include <sys/inline.h>
#include <sys/errno.h>
#include <stddef.h>
#include <stropts.h>


#if	__USE_PROTO__
void (STREAMS_SCHEDULER) (void)
#else
void
STREAMS_SCHEDULER __ARGS (())
#endif
{
}


/*
 * Shut things down, in the right order.
 */

#if	__USE_PROTO__
void (STREAMS_EXIT) (void)
#else
void
STREAMS_EXIT __ARGS (())
#endif
{
}


/*
 * Start things up, in the right order. If we can't proceed, panic.
 */

#if	__USE_PROTO__
void (STREAMS_INIT) (void)
#else
void
STREAMS_INIT __ARGS (())
#endif
{
}


/*
 * stub getmsg () and putmsg () system calls.
 */

#if	__USE_PROTO__
int ugetmsg (int fd, struct strbuf * ctlptr, struct strbuf * dataptr,
	     int * flagsp)
#else
int
ugetmsg (fd, ctlptr, dataptr, flagsp)
int		fd;
struct strbuf *	ctlptr;
struct strbuf *	dataptr;
int	      *	flagsp;
#endif
{
	set_user_error (ENOSTR);
	return -1;
}


#if	__USE_PROTO__
int uputmsg (int fd, __CONST__ struct strbuf * ctlptr,
	     __CONST__ struct strbuf * dataptr, int flags)
#else
int
uputmsg (fd, ctlptr, dataptr, flags)
int		fd;
__CONST__ struct strbuf
	      *	ctlptr;
__CONST__ struct strbuf
	      *	dataptr;
int		flags;
#endif
{
	set_user_error (ENOSTR);
	return -1;
}
