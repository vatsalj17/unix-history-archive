/*
 * This file contains the entry points into the STREAMS system called by the
 * Coherent kernel. This includes the startup and the system-call entry
 * points; part of the reason for bundling this stuff together is that when
 * bound into a Driver.a the system-call entry points are referred to by
 * code that comes later in the link, and so are not pulled in unless the
 * requirement for the init routine forces them to be present.
 */

#define	_DDI_DKI	1
#define	_SYSV3		1

#include <common/ccompat.h>
#include <kernel/strmlib.h>
#include <sys/cmn_err.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <stropts.h>
#include <stddef.h>

#define	_KERNEL		1

#include <sys/inode.h>
#include <sys/fd.h>
#include <sys/proc.h>

extern	long		_streams_maxctl;
extern	long		_streams_maxdata;
extern	size_t		_streams_size;
extern	uchar_t		_streams_heap [];

__EXTERN_C__	int	STRMEM_INIT	__PROTO ((uchar_t * _mem,
						  size_t _size));


/*
 *-STATUS:
 *	SVR3
 *
 *-NAME:
 *	getmsg ()	Get next message off a stream
 *
 *-SYNOPSIS:
 *	#include <stropts.h>
 *
 *	int getmsg (int fd, struct strbuf * ctlptr, struct strbuf * dataptr,
 *		    int * flagsp);
 *
 *-DESCRIPTION:
 *	getmsg () retrieves the contents of a message located at the stream
 *	head read queue from a STREAMS file, and places the contents into user
 *	specified buffer(s). The message must contain either a data part, a
 *	control part, or both. The data and control parts of the message are
 *	placed into separate buffers, as described below. The semantics of
 *	each part is defined by the STREAMS module that generated the message.
 *
 *	"fd" specifies a file descriptor referencing an open stream. "ctlptr"
 *	and "dataptr" each point to a "strbuf" structure, which contains the
 *	following members:
 *		int	maxlen;		Maximum buffer length
 *		int	len;		Length of data
 *		void  *	buf;		Pointer to buffer
 *
 *	"buf" points to a buffer in which the data or control information is
 *	to be placed, and "maxlen" indicates the maximum number of bytes this
 *	buffer can hold. On return, "len" contains the number of bytes of data
 *	or control information actually received, or 0 if there is a zero-
 *	length control or data part, or -1 if no data or control information
 *	is present in the message. "flagsp" should point to an integer that
 *	indicates the type of message the user is able to receive. This is
 *	described later.
 *
 *	"ctlptr" is user to hold the control part from the message and
 *	"dataptr" is used to hold the data part from the message. If "ctlptr"
 *	or "dataptr" is NULL or the "maxlen" field is -1, the control or data
 *	part of the message is not processed and is left on the stream head
 *	read queue. If "ctlptr" or "dataptr" is not NULL and there is no
 *	corresponding control or data part of the message on the stream head
 *	read queue, "len" is set to -1. If the "maxlen" field is set to 0
 *	and there is a zero-length control or data part, the zero-length part
 *	is removed from the read queue and "len" is set to 0. If the "maxlen"
 *	field is set to 0 and there are more than zero bytes of control or
 *	data information, that information is left on the read queue and "len"
 *	is set to 0. If the "maxlen" field in "ctlptr" or "dataptr" is less
 *	than, respectively, the control or data part of the message, "maxlen"
 *	bytes are retrieved. In this case, the remainder of the message is
 *	left on the stream head read queue and a non-zero return value is
 *	provided, as described below under DIAGNOSTICS.
 *
 *	By default, getmsg () processes the first available message on the
 *	stream head read queue. However, a user may choose to retrieve only
 *	high priority messages by setting the integer pointed to by "flagsp"
 *	to RS_HIPRI. In this case, getmsg () processes the next message only
 *	if it is a high priority message. If the integer pointed to by
 *	"flagsp" is 0, getmsg () retrieves any message available on the stream
 *	head read queue. In this case, the integer pointed to by "flagsp" will
 *	be set to RS_HIPRI if a high priority message was received, or 0
 *	otherwise.
 *
 *	If O_NDELAY and O_NONBLOCK are clear, getmsg () blocks until a message
 *	of the type specified by "flagsp" is available on the stream head read
 *	queue. If O_NDELAY or O_NONBLOCK has been set and a message of the
 *	specified type is not present on the read, queue, getmsg () fails and
 *	set errno to EAGAIN.
 *
 *	If a hangup occurs on the stream from which messages are to be
 * 	retrieved, getmsg () continues to operate normally, as described
 *	above, until the stream head read queue is empty. Thereafter, it
 *	returns 0 in the "len" fields of "ctlptr" and "dataptr".
 *
 *	getmsg () will fail if one or more of the following are true:
 *
 *	EAGAIN		The O_NDELAY or O_NONBLOCK flag is set and no messages
 *			are available.
 *
 *	EBADF		"fd" is not a valid file descriptor open for reading.
 *
 *	EBADMSG		Queued message to be read is not valid for getmsg ().
 *
 *	EFAULT		"ctlptr", "dataptr" or "flagsp" points outside the
 *			process's allocated address space.
 *
 *	EINTR		A signal was caught during the getmsg () system call.
 *
 *	EINVAL		An illegal value was specified in "flagsp", or the
 *			stream referenced by "fd" is linked under a
 *			multiplexor.
 *
 *	ENOSTR		A stream is not associated with "fd".
 *
 *	getmsg () can also fail if a STREAMS error message had been received
 *	at the stream header before the call to getmsg (). The error returned
 *	is the value contained in the STREAMS error message.
 *
 *-DIAGNOSTICS:
 *	Upon sucessful completion, a non-negative value is returned. A value
 *	of 0 indicates that a full message was read successfully. A return
 *	value of MORECTL indicates that more control information is waiting
 *	for retrieval. A return value of MOREDATA indicates that more data are
 *	waiting for retrieval. A return value of MORECTL | MOREDATA indicates
 *	that both types of information remain. Subsequent getmsg () calls
 *	retrieve the remainder of the information. However, if a message of
 *	higher priority has come in on the stream head read queue, the next
 *	call to getmsg () will retrieve that higher priority message before
 *	retrieving the remainder of the previously received partial message.
 */


#if	__USE_PROTO__
int ugetmsg (fd_t fd, struct strbuf * ctlptr, struct strbuf * dataptr,
	     int * flagsp)
#else
int
ugetmsg (fd, ctlptr, dataptr, flagsp)
fd_t		fd;
struct strbuf *	ctlptr;
struct strbuf *	dataptr;
int	      *	flagsp;
#endif
{
	struct strbuf	ctlbuf;
	struct strbuf *	kctlptr;
	struct strbuf	databuf;
	struct strbuf *	kdataptr;
	__fd_t	      *	fdp;
	int		flags;
	int		retval;
	int		band;

	if ((fdp = fd_get (fd)) == NULL) {
		set_user_error (EBADF);
		return -1;
	}

	if (! S_ISCHR (fdp->f_ip->i_mode) ||
	    fdp->f_ip->i_private == NULL) {
		set_user_error (ENOSTR);
		return;
	}

	if (flagsp == NULL ||
	    ukcopy (flagsp, & flags, sizeof (flags)) != sizeof (flags)) {
	    	set_user_error (EFAULT);
	    	return -1;
	}

	if ((flags & ~ RS_HIPRI) != 0) {
		set_user_error (EINVAL);
		return -1;
	}

	if (ctlptr != NULL) {
		if (ukcopy (ctlptr, & ctlbuf, sizeof (ctlbuf)) !=
		    sizeof (ctlbuf)) {
		    	set_user_error (EFAULT);
		    	return -1;
		}
		kctlptr = & ctlbuf;
	} else
		kctlptr = NULL;

	if (dataptr != NULL) {
		if (ukcopy (dataptr, & databuf, sizeof (databuf)) !=
		    sizeof (databuf)) {
		    	set_user_error (EFAULT);
		    	return -1;
		}
		kdataptr = & databuf;
	} else
		kdataptr = NULL;

	flags = (flags & RS_HIPRI) != 0 ? MSG_HIPRI : MSG_BAND;

	retval = 0;
	band = 0;
	set_user_error (STREAMS_GETPMSG ((shead_t *) fdp->f_ip->i_private,
					 kctlptr, kdataptr, & band, & flags,
					 MAKE_MODE (fdp->f_flag),
					 SELF->p_credp, & retval));

	flags = (flags & MSG_HIPRI) != 0 ? RS_HIPRI : 0;
	kucopy (& flags, flagsp, sizeof (flags));
	if (ctlptr != NULL)
		kucopy (kctlptr, ctlptr, sizeof (ctlbuf));
	if (dataptr != NULL)
		kucopy (kdataptr, dataptr, sizeof (databuf));
	return retval;
}


/*
 *-STATUS:
 *	SVR3
 *
 *-NAME:
 *	putmsg ()	send a message on a stream
 *
 *-SYNOPSIS:
 *	#include <stropts.h>
 *
 *	int putmsg (int fd, const struct strbuf * ctlptr,
 *		    const struct strbuf * dataptr, int flags);
 *
 *-DESCRIPTION:
 *	putmsg () creates a message from user-specified buffer(s) and sends
 *	the message to a STREAMS file. The message may contain either a data
 *	part, a control part, or both. The data and control parts to be sent
 *	are distinguished by placement in separate buffers, as described
 *	below. The semantics of each part is defined by the STREAMS module
 *	that receives the message.
 *
 *	"fd" specifies a file descriptor referencing an open stream. "ctlptr"
 *	and "dataptr" each point to a "strbuf" structure containing the
 *	following members:
 *		int	maxlen;		Not used
 *		int	len;		Length of data
 *		void  *	buf;		Pointer to buffer
 *
 *	"ctlptr" points to the structure describing the control part, if any,
 *	to be included in the message. The "buf" field in the "strbuf"
 *	structure points to the buffer where the control information resides,
 *	and the "len" field indicates the number of bytes to be sent. The
 *	"maxlen" field is not used in putmsg (). In a similar manner,
 *	"dataptr" specifies the data, if any, to be included in the message.
 *	"flags" indicates what type of message should be sent and is described
 *	later.
 *
 *	To send the data part of a message, "dataptr" must not be NULL and the
 *	"len" field of "dataptr" bust have a value of 0 or greater. To send
 *	the control part of a message, the corresponding values must be set
 *	for "ctlptr". No data or control) part is sent if either "dataptr"
 *	or "ctlptr" is NULL or if the len field of "dataptr" or "ctlptr" is
 *	set to -1.
 *
 *	For putmsg (), if a control part is specified, and flags is set to
 *	RS_HIPRI, a high priority message is sent. If no control part is
 *	specified, and flags is set to RS_HIPRI, putmsg () fails and sets
 *	"errno" to EINVAL. If "flags" is set to 0, a normal priority message
 *	is sent. If no control part and no data part are specified, and
 *	flags is set to 0, no message is sent, and 0 is returned.
 *
 *	The stream head guarantees that the control part of a message
 *	generated by putmsg () is at least 64 bytes in length.
 *
 *	Normally, putmsg () will block if the stream head write queue is full
 *	due to internal flow control conditions. For high-priority messages,
 *	putmsg () does not block on this condition. For other messages,
 *	putmsg () does not block when the write queue is full and O_NDELAY or
 *	O_NONBLOCK has been specified. No partial message is sent.
 *
 *	putmsg () fails if one or more of the following is true:
 *
 *	EAGAIN		A non-priority message was specified, the O_NDELAY or
 *			O_NONBLOCK flag is set and the stream write queue is
 *			full due to internal flow control conditions.
 *
 *	EBADF		"fd" is not a valid file descriptor open for writing.
 *
 *	EFAULT		"ctlptr" or "dataptr" points outside the process's
 *			allocated address space.
 *
 *	EINTR		A signal was caught during the putmsg () system call.
 *
 *	EINVAL		An undefined value was specified in "flags" or "flags"
 *			is set to RS_HIPRI and no control part was supplied.
 *
 *	EINVAL		The stream referenced by "fd" is linked below a
 *			multiplexor.
 *
 *	ENOSR		Buffers could not be allocated for the message that
 *			was to be send due to insufficient STREAMS memory
 *			resources.
 *
 *	ENOSTR		A stream is not associated with "fd".
 *
 *	ENXIO		A hangup condition was generated downstream for the
 *			specified stream, or the other end of the pipe is
 *			closed.
 *
 *	ERANGE		The size of the data part of the message does not fall
 *			within the minimum and maximum packet sizes of the
 *			topmost stream module. This value is also returned if
 *			the control part of the message is larger than the
 *			maximum configured size for the control part of a
 *			message, or if the data part of a message is larger
 *			than the maximum configured size for the data part of
 *			a message.
 *
 *	putmsg () also fails is a STREAMS error message had been processed by
 *	the stream head before the call to putmsg (). The error returned is
 *	the value contained in the STREAMS error message.
 *
 *-DIAGNOSTICS:
 *	Upon successful completion, a value of 0 is returned. Otherwise, a
 *	value of -1 is returned and "errno" is set to indicate the error.
 */

#if	__USE_PROTO__
int uputmsg (fd_t fd, __CONST__ struct strbuf * ctlptr,
	     __CONST__ struct strbuf * dataptr, int flags)
#else
int
uputmsg (fd, ctlptr, dataptr, flags)
fd_t		fd;
__CONST__ struct strbuf
	      *	ctlptr;
__CONST__ struct strbuf
	      *	dataptr;
int		flags;
#endif
{
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	__fd_t	      *	fdp;
	int		retval;

	if ((fdp = fd_get (fd)) == NULL) {
		set_user_error (EBADF);
		return -1;
	}

	if ((flags & ~ RS_HIPRI) != 0) {
		set_user_error (EINVAL);
		return -1;
	}

	if (! S_ISCHR (fdp->f_ip->i_mode) ||
	    fdp->f_ip->i_private == NULL) {

		set_user_error (ENOSTR);
		return;
	}
	if (ctlptr != NULL) {
		if (ukcopy (ctlptr, & ctlbuf, sizeof (ctlbuf)) !=
		    sizeof (ctlbuf)) {
		    	set_user_error (EFAULT);
		    	return -1;
		}
		ctlptr = & ctlbuf;
	}
	if (dataptr != NULL) {
		if (ukcopy (dataptr, & databuf, sizeof (databuf)) !=
		    sizeof (databuf)) {
		    	set_user_error (EFAULT);
		    	return -1;
		}
		dataptr = & databuf;
	}
	flags = (flags & RS_HIPRI) != 0 ? MSG_HIPRI : MSG_BAND;

	retval = 0;
	set_user_error (STREAMS_PUTPMSG ((shead_t *) fdp->f_ip->i_private,
					 ctlptr, dataptr, 0, flags,
					 MAKE_MODE (fdp->f_flag),
					 SELF->p_credp, & retval));
	return retval;
}


/*
 * Start things up, in the right order. If we can't proceed, panic.
 */

#if	__USE_PROTO__
void (streamsinit) (void)
#else
void
streamsinit __ARGS (())
#endif
{
	/*
	 * Bootstrap the message allocator. Note that memory management in
	 * Coherent is sane, this is quite simple.
	 */

	while (STRMEM_INIT (_streams_heap, _streams_size) != 0)
		cmn_err (CE_PANIC, "Unable to initalise STREAMS heap");

	str_mem->sm_maxctlsize = _streams_maxctl;
	str_mem->sm_maxdatasize = _streams_maxdata;
}
