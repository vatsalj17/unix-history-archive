/* $Header: /ker/coh.386/RCS/sys6.c,v 2.4 93/10/29 00:55:54 nigel Exp Locker: nigel $ */
/*
 * POSIX.1-oriented system calls for Coherent.
 *
 * Conventions: as elsewhere, system call handlers have the same name as the
 * user would use but prefixed by a 'u'. Internal data interfaces have the
 * same name as a user function if they have the same signature, which is a
 * good thing for testing, and can save on redundant prototypes. Wherever
 * possible, we use such function-call interfaces rather than get involved in
 * the disgusting mess that is the U area or process-table mechanisms.
 */
/*
 * $Log:	sys6.c,v $
 * Revision 2.4  93/10/29  00:55:54  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.3  93/08/19  03:26:57  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.2  93/07/26  14:55:33  nigel
 * Nigel's R80
 */

#define	_SYSV3		1
#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1

#include <common/ccompat.h>
#include <common/_tricks.h>
#include <common/_clktck.h>
#include <common/_gregset.h>
#include <kernel/sig_lib.h>
#include <kernel/proc_lib.h>
#include <kernel/cred_lib.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/cmn_err.h>
#include <sys/cred.h>
#include <unistd.h>
#include <limits.h>
#include <stddef.h>
#include <poll.h>

#include <sys/sched.h>

/*
 * Dip into the filth...
 */

#define	_KERNEL		1

#include <kernel/dir.h>
#include <sys/uproc.h>	/* for pathconf () to use ftoi () */
#include <sys/proc.h>
#include <sys/inode.h>
#include <sys/io.h>
#include <sys/fd.h>


/*
 * A brief note, before we begin: at the time of writing, the Coherent kernel
 * is a little screwed-up w.r.t. checking validity of memory references. While
 * in a sane system the user-kernel copying routines would perform all the
 * necessary checking, apparently this is not in fact the case. Sadly, rather
 * than do the right thing there is now an additional API for this which is
 * used or not used apparently at random.
 *
 * Well, screw that. The only sane course of action is to make the user-kernel
 * copy routines do everything they need to do to cope with checking user
 * address validity; the routines below assume this, and when I rewrite all
 * the MMU stuff I'll ensure that it is so.
 */

/*
 *-STATUS:
 *	POSIX.1
 *
 *-NAME:
 *	sigaction ()	Detailed signal management
 *
 *-SYNOPSIS:
 *	#include <signal.h>
 *
 *	int sigaction (int sig, const struct sigaction * act,
 *		       struct sigaction * oact);
 *
 *-DESCRIPTION:
 *	sigaction () allows the calling process to examine and/or specify the
 *	action to be taken on delivery of a specific signal.
 *
 *	"sig" specifies the signal and can be assigned any of the signals
 *	specified in signal(5) except SIGKILL and SIGSTOP.
 *
 *	If the argument "arg" is not NULL, it points to a structure specifying
 *	the new action to be taken when delivering "sig". If the argument
 *	"oact" is not NULL, it points to a structure where the action
 *	previously associated with "sig" is to be stored on return from
 *	sigaction ().
 *
 *	The "sigaction" structure includes the following members:
 *		void	     (*	sa_handler) ();
 *		sigset_t	sa_mask;
 *		int		sa_flags;
 *
 *	"sa_handler" specifies the disposition of the signal and may take any
 *	of the values specified in signal (5).
 *
 *	"sa_mask" specifies a set of signals to be blocked while the signal
 *	handler is active. On entry to the signal handler, that set of signals
 *	is added to the set of signals already being blocked when the signal
 *	is delivered. In addition, the signal that caused the handler to be
 *	executed will also be blocked, unless the SA_NODEFER flag has been
 *	specified. SIGSTOP and SIGKILL cannot be blocked (the system silently
 *	enforces this restriction).
 *
 *	"sa_flags" specifies a set of flags used to modify the behaviour of
 *	the signal. It is formed by a logical OR of any of the following
 *	values (only SA_CLDSTOP is available to System V, Release 3
 *	applications):
 *
 *	SA_NOCLDSTOP	If set and "sig" equals SIGCHLD, "sig" will not be
 *			sent to the calling process when its child processes
 *			stop or continue.
 *
 *	SA_NOCLDWAIT	If set and "sig" equals SIGCHLD, the system will not
 *			create zombie processes when children of the calling
 *			process exit. If the calling process subsequently
 *			issues a wait (), it blocks until all of the calling
 *			process's child processes terminate, and then returns
 *			a value of -1 with "errno" set to ECHILD.
 *
 *	SA_ONSTACK	If set and the signal is caught and an alternate
 *			signal stack has been declared with sigaltstack (),
 *			the signal is delivered to the calling process on that
 *			stack. Otherwise, the signal is delivered on the
 *			same stack as the main program.
 *
 *	SA_RESETHAND	If set and the signal is caught, the disposition of
 *			the signal is reset to SIG_DFL, and the signal will
 *			not be blocked on entry to the signal handler (SIGILL,
 *			SIGTRAP, and SIGPWR cannot be automatically reset when
 *			delivered; the system silently enforces this
 *			restriction).
 *
 *	SA_NODEFER	If set and the signal is caught, the signal will not
 *			be automatically blocked by the kernel while it is
 *			being caught.
 *
 *	SA_RESTART	If set and the signal is caught, a system call that
 *			is interrupted by the execution of this signal's
 *			handler is transparently restarted by the system.
 *			Otherwise, the system call returns an EINTR error.
 *
 *	SA_SIGINFO	If cleared and the signal is caught, "sig" is passed
 *			as the only argument to the signal-catching function.
 *			If set and the signal is caught, pending signals of
 *			type "sig" are reliably queued to the calling process
 *			and two additional arguments are passed to the signal-
 *			catching function. If the second argument is not equal
 *			to NULL, it points to a "siginfo_t" structure
 *			containing the reason why the signal was generated;
 *			the third argument points to a "ucontext_t" structure
 *			containing the receiving process's context when the
 *			signal was delivered.
 *
 *	sigaction () fails if any of the following is true:
 *
 *	EINVAL		The value of the "sig" argument is not a valid signal
 *			number or is equal to SIGKILL or SIGSTOP.
 *
 *	EFAULT		"act" or "oact" points outside the process's allocated
 *			address space.
 *
 *-DIAGNOSTICS:
 *	On success, sigaction () returns zero. On failure, it returns -1 and
 *	sets "errno" to indicate the error.
 */

#if	__USE_PROTO__
int usigaction (int sig, __CONST__ struct sigaction * act,
		struct sigaction * oact, gregset_t * regsetp)
#else
int
usigaction (sig, act, oact, regsetp)
int			sig;
__CONST__ struct sigaction
		      *	act;
struct sigaction      *	oact;
gregset_t	      *	regsetp;
#endif
{
	__sigaction_t	action;
	__sigmiscfl_t	flags = curr_signal_misc (0, 0);
	struct sigaction temp;

	if (! __IN_RANGE (1, sig, _SIGNAL_MAX) || sig == SIGKILL ||
	    sig == SIGSTOP) {
		set_user_error (EINVAL);
		return -1;
	}

	u.u_sigreturn = (__sighand_t *) regsetp->_i386._edx;

	if (act != NULL) {
		if (ukcopy (act, & temp, sizeof (temp)) != sizeof (temp))
			return -1;

		if (sig == SIGCHLD) {
			/*
			 * Deal with the child-related signal flags.
			 */

			(void) curr_signal_misc
				(__SF_NOCLDSTOP,
				 (temp.sa_flags & SA_NOCLDSTOP) == 0 ? 0 :
				 	__SF_NOCLDSTOP);
		}

		action.sa_flags = 0;
		action.sa_handler = temp.sa_handler;
		action.sa_mask._sigbits [0] = temp.sa_mask._sigbits [0];
#if	___SIGSET_LEN == 2
		action.sa_mask._sigbits [1] = 0;
#endif
		/*
		 * This needs to be done automatically unless SA_RESETHANDLER
		 * is true; so, for an SVR3 interface, we always do this...
		 */

		__SIGSET_ADDBIT (action.sa_mask, sig);
	}

	curr_signal_action (sig, act == NULL ? NULL : & action, & action);

	if (oact == NULL)
		return 0;

	temp.sa_handler = action.sa_handler;
	temp.sa_mask._sigbits [0] = action.sa_mask._sigbits [0];
	temp.sa_flags = (flags & __SF_NOCLDSTOP) != 0 ? SA_NOCLDSTOP : 0;

	if (kucopy (& temp, oact, sizeof (temp)) != sizeof (temp)) {
	    	/*
	    	 * If the user requested a change in the signal disposition,
	    	 * undo the change before returning in error.
	    	 */

	    	if (act != NULL) {
			curr_signal_action (sig, & action, NULL);
			curr_signal_misc (-1, flags);
		}
		return -1;
	}
	return 0;
}


/*
 *-STATUS:
 *	POSIX.1
 *
 *-NAME:
 *	sigpending ()	Examine signals that are blocked and pending
 *
 *-SYNOPSIS:
 *	#include <signal.h>
 *
 *	int sigpending (sigset_t * set);
 *
 *-DESCRIPTION:
 *	The sigpending () function retrieves those signals that have been sent
 *	to the calling process but are being blocked from delivery by the
 *	calling process's signal mask. The signals are stored in the space
 *	pointed to by the argument "set".
 *
 *	sigpending () fails if any of the following are true:
 *
 *	EFAULT		The "set" argument points outside the process's
 *			allocated address space.
 *
 *-DIAGNOSTICS:
 *	On success, sigpending () returns zero. On failure, it returns -1 and
 *	sets "errno" to indicate the error.
 */

#if	__USE_PROTO__
int usigpending (o_sigset_t * set)
#else
int
usigpending (set)
o_sigset_t    *	set;
#endif
{
	o_sigset_t	temp;

	temp._sigbits [0] = SELF->p_pending_signals._sigbits [0];

	return kucopy (& temp, set, sizeof (* set)) != sizeof (* set) ? -1 : 0;
}


/*
 *-STATUS:
 *	POSIX.1
 *
 *-NAME:
 *	sigprocmask ()	Change or examine signal mask
 *
 *-SYNOPSIS:
 *	#include <signal.h>
 *
 *	int sigprocmask (int how, const sigset_t * set, sigset_t * oset);
 *
 *-DESCRIPTION:
 *	The sigprocmask () function is used to examine and/or change the
 *	calling process's signal mask. If the value of "how" is SIG_BLOCK, the
 *	set pointed to by the argument "set" is added to the current signal
 *	mask. If "how" is SIG_UNBLOCK, the set pointed to by the argument
 *	"set" is removed from the current signal mask. If "how" is
 *	SIG_SETMASK, the current signal mask is replaced by the set pointed
 *	to by the argument "set". If the argument "oset" is not NULL, the
 *	previous mask is stored in the space pointed to by "oset". If the
 *	value of the argument "set" is NULL, the value "how" is not
 *	significant and the process's signal mask is unchanged; thus, the call
 *	can be used to enquire about currently blocked signals.
 *
 *	If there are any pending unblocked signals after the call to
 *	sigprocmask (), at least one of those signals will be delivered before
 *	the call to sigprocmask () returns.
 *
 *	It is not possible to block those signals that cannot be ignored [see
 *	sigaction ()]; this restriction is silently imposed by the system.
 *
 *	If sigprocmask () fails, the process's signal mask is not changed.
 *
 *	sigprocmask () fails if any of the following are true:
 *
 *	EINVAL		The value of the "how" argument is not equal to one of
 *			the defined values.
 * 
 *	EFAULT		The value of "set" or "oset" points outside the
 *			process's allocated address space.
 *
 *-DIAGNOSTICS:
 *	On success, sigprocmask () returns zero. On failure, it returns -1 and
 *	sets "errno" to indicate the error.
 */

#if	__USE_PROTO__
int usigprocmask (int how, __CONST__ o_sigset_t * set, o_sigset_t * oset)
#else
int
usigprocmask (how, set, oset)
int		how;
__CONST__ o_sigset_t
	      *	set;
o_sigset_t    *	oset;
#endif
{
	o_sigset_t	temp;
	__sigset_t	mask;

	if (set != NULL) {
		if (ukcopy (set, & temp, sizeof (temp)) != sizeof (temp))
			return -1;

		curr_signal_mask (NULL, & mask);

		switch (how) {

		case SIG_UNBLOCK:
			mask._sigbits [0] &= ~ temp._sigbits [0];
			break;

		case SIG_BLOCK:
			mask._sigbits [0] |= temp._sigbits [0];
			break;

		case SIG_SETMASK:
			mask._sigbits [0] = temp._sigbits [0];
			break;

		default:
			set_user_error (EINVAL);
			return -1;
		}
	}

	curr_signal_mask (set == NULL ? NULL : & mask, & mask);

	temp._sigbits [0] = mask._sigbits [0];

	if (oset == NULL)
		return 0;

	if (kucopy (& temp, oset, sizeof (* oset)) != sizeof (* oset)) {
		/*
		 * If the user requested a change to the signal mask, we undo
		 * it before returning in error.
		 */

		if (set != NULL)
			curr_signal_mask (& mask, NULL);
		return -1;
	}
	return 0;
}


/*
 *-STATUS:
 *	POSIX.1
 *
 *-NAME:
 *	sigsuspend ()	install a signal mask and suspend process until signal
 *
 *-SYNOPSIS:
 *	#include <signal.h>
 *
 *	int sigsuspend (const sigset_t * set);
 *
 *-DESCRIPTION:
 *	sigsuspend () replaces the process's signal mask with the set of
 *	signals pointed to by the argument "set" and then suspends the process
 *	until delivery of a signal whose action is either the execute a signal
 *	catching function or to terminate the process.
 *
 *	If the action is to terminate the process, sigsuspend () does not
 *	return. If the action is to execute a signal catching function,
 *	sigsuspend () returns are the signal catching function returns. On
 *	return, the signal mask is restored to the set the existed before the
 *	call to sigsuspend ().
 *
 *	It is not possible to block those signals that cannot be ignored; this
 *	restriction is silently imposed by the system.
 *
 *	sigsuspend () fails if either of the following is true:
 *
 *	EINTR		A signal is caught by the calling process and control
 *			is returned from the signal catching function.
 *
 *	EFAULT		The "set" argument points outside the process's
 *			allocated address space.
 *
 *-DIAGNOSTICS:
 *	Since sigsuspend () suspends process execution indefinitely, there is
 *	no successful completion return value. On failure, it returns -1 and
 *	sets "errno" to indicate the error.
 */
#if	__USE_PROTO__
int usigsuspend (__CONST__ o_sigset_t * set, gregset_t * regsetp)
#else
int
usigsuspend (set, regsetp)
__CONST__ o_sigset_t
	      *	set;
gregset_t     *	regsetp;
#endif
{
	__sigset_t	mask;

	if (ukcopy (set, & mask, sizeof (* set)) != sizeof (* set))
		return - 1;

	curr_signal_mask (& mask, & mask);

	/*
	 * Like pause (), sleep such that we will only be woken by a signal.
	 */

	x_sleep ((caddr_t) & u, prilo, slpriSigCatch, "sigsuspend ()");

	/*
	 * Set up the signal context now rather than in the return-to-user-
	 * mode code so that the signal dispatch is based on the "set" mask,
	 * but ensure that the old mask is put into the return-from-signal
	 * context.
	 *
	 * Note also that because the user signal frame is constructed here
	 * rather than on the way out, we get to set up the user return value
	 * from the sigsuspend () system call right here - and since we must
	 * return an error indication from sigsuspend (), we have to do the
	 * magic for the user-level library stub.
	 */

	/* Fix from Nigel 94/07/03 */
	regsetp->_i386._eax = EINTR;
	__FLAG_REG (regsetp) = __FLAG_SET_FLAG (__FLAG_REG (regsetp), __CARRY);

	curr_check_signals (regsetp, & mask);
	curr_signal_mask (& mask, NULL);

	/* Fix from Nigel 94/07/03 */
	return 0;	/* return any damn thing... */
}



/*
 * Local support routine for [f]pathconf (), defined below.
 */

#if	__USE_PROTO__
__LOCAL__ int _pathconf (INODE * ip, int name)
#else
__LOCAL__ int
_pathconf (ip, name)
INODE	      *	ip;
int		name;
#endif
{
	switch (name) {

	case _PC_LINK_MAX:
		return LINK_MAX;

	case _PC_MAX_CANON:
	case _PC_MAX_INPUT:
		return 255;		/* CANON_MAX */

	case _PC_NAME_MAX:
		return DIRSIZ;

	case _PC_PATH_MAX:
		return PATH_MAX;

	case _PC_PIPE_BUF:
		return PIPE_MAX;

	case _PC_CHOWN_RESTRICTED:
		return 1;		/* always true for Coh 4.x */

	case _PC_NO_TRUNC:
		return 0;		/* never true for Coh 4.x */

	case _PC_VDISABLE:
#ifndef	_POSIX_VDISABLE
		return -1;		/* never true for Coh 4.x */
#else
		return _POSIX_VDISABLE;
#endif

	default:
		set_user_error (EINVAL);
		return -1;
	}
}


/*
 *-STATUS:
 *	POSIX.1
 *
 *-NAME:
 *	fpathconf ()
 *	pathconf ()	get configurable pathname variables
 *
 *-SYNOPSIS:
 *	#include <unistd.h>
 *
 *	long fpathconf (int fildes, int name);
 *	long pathconf (const char * path, int name);
 *
 *-DESCRIPTION:
 *	The functions fpathconf () and pathconf () return the current value of
 *	a configurable limit or option associated with a file or directory.
 *	The "path" argument points to the pathname of a file or directory;
 *	"fildes" is an open file descriptor; and "name" is the symbolic
 *	constant (defined in <unistd.h>) representing the configurable system
 *	limit or option to be returned.
 *
 *	The values returned by pathconf () or fpathconf () depend on the type
 *	of file specified by "path" or "fildes". The following table contains
 *	the symbolic constants supported by pathconf () and fpathconf ()
 *	along with the POSIX defined return value. The return value is based
 *	on the type of file specified by "path" or "fildes".
 *
 *	_PC_LINK_MAX	The maximum value of a file's link count. If "path" or
 *			"fildes" refers to a directory, the value returned
 *			applies to the directory itself.
 *
 *	_PC_MAX_CANON	The number of bytes in a terminal canonical input
 *			queue. The behaviour is undefined if "path" or
 *			"fildes" does not refer to a terminal file.
 *
 *	_PC_MAX_INPUT	The number of bytes for which space will be available
 *			in a terminal input queue. The behaviour is undefined 
 *			"path" or "fildes" does not refer to a terminal file.
 *
 *	_PC_NAME_MAX	The number of bytes in a filename. The behaviour is
 *			undefined if "path" or "fildes" does not refer to a
 *			directory. The value returned applies to the filenames
 *			within the directory.
 *
 *	_PC_PATH_MAX	The number of bytes in a pathname. The behaviour is
 *			undefined if "path" or "fildes" does not refer to a
 *			directory. The value returned is the maximum length of
 *			a relative pathname when the specified directory is
 *			the working directory.
 *
 *	_PC_PIPE_BUF	The number of bytes that can be written atomically
 *			when writing to a pipe. If "path" or "files" refers to
 *			a pipe or FIFO, the value returned applies to the FIFO
 *			itself. If "path" or "fildes" refers to a directory,
 *			the value returned applies to any FIFOs that exist or
 *			can be created within the directory. If "path" or
 *			"fildes" refer to any other type of file, the
 *			behaviour is undefined.
 *
 *	_PC_CHOWN_RESTRICTED	The use of the chown () function is restricted
 *			to a process with appropriate priveleges, and to
 *			changing the group ID of a file only to the effective
 *			group ID of the process or to one of its supplementary
 *			group IDs. If "path" or "fildes" refers to a
 *			directory, the value returned applies to any files,
 *			other than directories, that exist or can be created
 *			within the directory.
 *
 *	_PC_NO_TRUNC	Pathname components longer than NAME_MAX generate an
 *			error. The behaviour is	undefined if "path" or
 *			"fildes" does not refer to a directory. The value
 *			returned applies to the filenames within the
 *			directory.
 *	_PC_VDISABLE	Terminal special characters can be disabled using this
 *			character value, if it is defined. The behaviour is
 *			undefined if "path" or "filedes" does not refer to a
 *			terminal file.
 *
 *	The value of the configurable system limit or option specified by
 *	"name" does not change during the lifetime of the calling process.
 *
 *	fpathconf () fails if the following is true:
 *
 *	EBADF		"fildes" is not a valid file descriptor.
 *
 *	pathconf () fails if any of the following are true:
 *
 *	EACCES		Search permission is denied for a component of the
 *			path prefix.
 *
 *	ELOOP		Too many symbolic links are encountered while
 *			translating "path".
 *
 *	EMULTIHOP	Components of "path" require hopping to multiple
 *			remote machines and the file system type does not
 *			allow it.
 *
 *	ENAMETOOLONG	The length of a pathname component exceeds PATH_MAX,
 *			or a pathname component is longer than NAME_MAX while
 *			_POSIX_NO_TRUNC is in effect.
 *
 *	ENOENT		"path" is needed for the command specified and the
 *			named file does not exist or if the "path" argument
 *			points to an empty string.
 *
 *	ENOLINK		"path" points to a remote machine and the link to that
 *			machine is no longer active.
 *
 *	ENOTDIR		A component of the path prefix is not a directory.
 *
 *	Both fpathconf () and pathconf () fail if the following is true:
 *
 *	EINVAL		if "name" is an invalid value.
 *
 *-DIAGNOSTICS:
 *	If fpathconf () or pathconf () is invoked with an invalid symbolic
 *	constant or the symbolic constant corresponds to a configurable system
 *	limit or option not supported on the system, a value of -1 is returned
 *	to the invoking process. If the function fails because the
 *	configurable system limit or option corresponding to "name" is not
 *	supported on the system the value of "errno" is not changed.
 */

#if	__USE_PROTO__
int ufpathconf (int fildes, int name)
#else
int
ufpathconf (fildes, name)
int		fildes;
int		name;
#endif
{
	__fd_t	      *	fdp;

	if ((fdp = fd_get (fildes)) == NULL) {
		set_user_error (EBADF);
		return -1;
	}

	return _pathconf (fdp->f_ip, name);
}


#if	__USE_PROTO__
int upathconf (__CONST__ char * path, int name)
#else
int
upathconf (path, name)
__CONST__ char * path;
int		name;
#endif
{
	struct direct	dir;
	int		ret;

	if (ftoi (path, 'r', IOUSR, NULL, & dir, SELF->p_credp))
		return -1;

	ret = _pathconf (u.u_cdiri, name);
	idetach (u.u_cdiri);
	return ret;
}


/*
 *-STATUS:
 *	POSIX.1
 *
 *-NAME:
 *	sysconf ()	get configurable system variables
 *
 *-SYNOPSIS:
 *	#include <unistd.h>
 *
 *	long sysconf (int name);
 *
 *-DESCRIPTION:
 *	The sysconf () function provides a method for the application to
 *	determine the current value of a configurable system limit or option.
 *
 *	The "name" argument represents the system variable to be queried. The
 *	following table lists the minimal set of system variables from
 *	<limits.h> and <unistd.h> that can be returned by sysconf (), and the
 *	symbolic constants that are the corresponding values used for "name":
 *
 *	NAME:			RETURN VALUE:
 *	_SC_ARG_MAX		ARG_MAX
 *	_SC_CHILD_MAX		CHILD_MAX
 *	_SC_CLK_TCK		CLK_TCK
 *	_SC_NGROUPS_MAX		NGROUPS_MAX
 *	_SC_OPEN_MAX		OPEN_MAX
 *	_SC_PASS_MAX		PASS_MAX
 *	_SC_PAGESIZE		PAGESIZE
 *	_SC_JOB_CONTROL		_POSIX_JOB_CONTROL
 *	_SC_SAVED_IDS		_POSIX_SAVED_IDS
 *	_SC_VERSION		_POSIX_VERSION
 *	_SC_XOPEN_VERSION	_XOPEN_VERSION
 *	_SC_LOGNAME_MAX		LOGNAME_MAX
 *
 *	The value of CLK_TCK may be variable and it should not be assumed that
 *	CLK_TCK is a compile-time constant. The value of CLK_TCK is the same
 *	as the value of sysconf (_SC_CLK_TCK).
 *
 *-DIAGNOSTICS:
 *	If "name" is an invalid value, sysconf () will return -1 and set
 *	"errno" to indicate the error. If sysconf () fails due to a value of
 *	"name" that is not defined on the system, the function will return
 *	a value of -1 without changing the value of "errno".
 *
 *-NOTES:
 *	A call to setrlimit () may cause the value of OPEN_MAX to change on
 *	System V, Release 4-compatible systems.
 */

#if	__USE_PROTO__
long usysconf (int name)
#else
long
usysconf (name)
int		name;
#endif
{
	switch (name) {

	case _SC_ARG_MAX:
		/*
		 * Actually, Coherent has no limit here, but we say we do for
		 * iBCS2 compatibility.
		 */
		return ARG_MAX;

	case _SC_CHILD_MAX:
		return CHILD_MAX;

	case _SC_CLK_TCK:
		return CLK_TCK;

	case _SC_NGROUPS_MAX:
		return NGROUPS_MAX;		/* set usetgroups () */

	case _SC_OPEN_MAX:
		return NOFILE;

#ifdef	_SC_PASS_MAX
	case _SC_PASS_MAX:
		return PASS_MAX;
#endif

#ifdef	_SC_PAGESIZE
	case _SC_PAGESIZE:
		return NBPC;
#endif

	case _SC_JOB_CONTROL:
#if	_POSIX_JOB_CONTROL
		return 1;
#else
		return 0;
#endif

	case _SC_SAVED_IDS:
#if	_POSIX_SAVED_IDS
		return 1;
#else
		return 0;
#endif

	case _SC_VERSION:
		return 199009L;

#ifdef	_SC_LOGNAME_MAX
	case _SC_LOGNAME_MAX:
		return LOGNAME_MAX;
#endif

	default:
		set_user_error (EINVAL);
		return -1;
	}
}


/*
 *-STATUS:
 *	POSIX.1
 *
 *-NAME:
 *	getgroups ()	get supplementary groups access list IDs
 *
 *-SYNOPSIS:
 *	#include <unistd.h>
 *
 *	int getgroups (int gidsetsize, gid_t * grouplist);
 *
 *-DESCRIPTION:
 *	getgroups () gets the current supplemental group access list of the
 *	calling process and stores the result into the array of group IDs
 *	specified by "grouplist". This array has "gidsetsize" entries and must
 *	be large enough to contain the entire list. This list cannot be
 *	greater than {NGROUPS_MAX}. If "gidsetsize" equals 0, getgroups ()
 *	will return the number of groups to which the calling process belongs
 *	without modifying the array pointed to by "grouplist".
 *
 *	getgroups () will fail if:
 *
 *	EINVAL		The value of "gidsetsize" is non-zero and less than
 *			the number of supplementary groups IDs set for the
 *			calling process.
 *
 *	EFAULT		A referenced part of the array pointed to by
 *			"grouplist" is outside of the allocated address space
 *			of the process.
 *
 *-DIAGNOSTICS:
 *	Upon successful completion, getgroups () returns the number of
 *	supplementary groups IDs set for the calling process. Otherwise, a
 *	value of -1 is returned and "errno" is set to indicate the error.
 */

#if	__USE_PROTO__
int ugetgroups (int gidsetsize, __gid_t * grouplist)
#else
int
ugetgroups (gidsetsize, grouplist)
int		gidsetsize;
__gid_t	      *	grouplist;
#endif
{
	__gid_t		groups [NGROUPS_MAX];
	int		i;

	if (gidsetsize < 0) {
		set_user_error (EINVAL);
		return -1;
	}
	if (gidsetsize > SELF->p_credp->cr_ngroups)
		gidsetsize = SELF->p_credp->cr_ngroups;

	/*
	 * Convert group ID formats.
	 */

	for (i = 0 ; i < gidsetsize ; i ++)
		groups [i] = SELF->p_credp->cr_groups [i];

	kucopy (groups, grouplist, gidsetsize * sizeof (__gid_t));
	return SELF->p_credp->cr_ngroups;
}


/*
 *-STATUS:
 *	SVR4
 *
 *-NAME:
 *	setgroups ()	set supplementary groups access list IDs
 *
 *-SYNOPSIS:
 *	#include <unistd.h>
 *
 *	int setgroups (int ngroups, const gid_t * grouplist);
 *
 *-DESCRIPTION:
 *	setgroups () sets the supplementary group access list of the calling
 *	process from the array of group IDs specified by "grouplist". The
 *	number of entries is specified by "ngroups" and can not be greater
 *	than {NGROUPS_MAX}. This function may be invoked only by the super-
 *	user.
 *
 *	setgroups () will fail if:
 *
 *	EINVAL		The value of "ngroups" is greater than {NGROUPS_MAX}.
 *
 *	EPERM		The effective user ID is not the super-user.
 *
 *	EFAULT		A referenced part of the array pointed to by
 *			"grouplist" is outside of the allocated address space
 *			of the process.
 *
 *-DIAGNOSTICS:
 *	Upon successful completion, setgroups () returns the value 0.
 *	Otherwise, a value of -1 is returned and "errno" is set to indicate
 *	the error.
 */

#if	__USE_PROTO__
int usetgroups (int ngroups, __gid_t * grouplist)
#else
int
usetgroups (ngroups, grouplist)
int		ngroups;
__gid_t	      *	grouplist;
#endif
{
	cred_t	      *	credp;
	__gid_t		groups [NGROUPS_MAX];
	int		i;

	if (! __IN_RANGE (0, ngroups, NGROUPS_MAX)) {
		set_user_error (EINVAL);
		return -1;
	}
	if (! super ())
		return -1;

	if (ukcopy (grouplist, groups, ngroups * sizeof (__gid_t)) !=
	    ngroups * sizeof (__gid_t))
		return -1;

	if ((credp = cred_setgrp (SELF->p_credp, ngroups)) == NULL) {
		set_user_error (EAGAIN);
		return -1;
	}
	SELF->p_credp = credp;

	/*
	 * Convert group ID formats.
	 */

	for (i = 0 ; i < ngroups ; i ++)
		SELF->p_credp->cr_groups [i] = groups [i];
	return 0;
}


/*
 *-STATUS:
 *	ANSI
 *
 *-NAME:
 *	rename ()	change the name of a file
 *
 *-SYNOPSIS:
 *	#include <stdio.h>
 *
 *	int rename (const char * old, const char * new);
 *
 *-DESCRIPTION:
 *	rename () renames a file. "old" is a pointer to the pathname of the
 *	file or directory to be ranamed. "new" is a pointer to the new
 *	pathname of the file or directory. Both "old" and "new" must be of the
 *	same type (either both files, or both directories) and must reside on
 *	the same file system.
 *
 *	If "new" already exists, it is removed. Thus, if "new" names an
 *	existing directory, the directory must not have any entries other than
 *	possibly '.' and '..'. When renaming directories, the "new" pathname
 *	must not name a descendant of "old". The implementation of rename ()
 *	ensures that upon successful completion a link named "name" will
 *	always exist.
 *
 *	If the final component of "old" is a symbolic link, the symbolic link
 *	is renamed, not the file or directory to which it points.
 *
 *	Write permission is required for both the directory containing "old"
 *	and the directory containing "new".
 *
 *	rename () fails, "old" is not changed, and no "new" file is created if
 *	one or more of the following are true:
 *
 *	EACCES		A component of either path prefix denies search
 *			permission; one of the directories containing "old"
 *			or "new" denies write permission; or one of the
 *			directories pointed to by "old" or "new" denies write
 *			permission.
 *
 *	EBUSY		"new" is a directory and the mount point for a mounted
 *			filesystem.
 *
 *	EDQUOT		The directory in which the entry for the new name is
 *			being placed cannot be extended because the user's
 *			quota of disk blocks on the file system containing the
 *			directory has been exhausted.
 *
 *	EEXIST		The link named by "new" is a directory containing
 *			entries other than '.' and '..'.
 *
 *	EFAULT		"old" or "new" points outside the process's allocated
 *			address space.
 *
 *	EINVAL		"old" is a parent directory of "new", or an attempt is
 *			made to rename '.' or '..'.
 *
 *	EINTR		A signal was caught during execution of the rename ()
 *			system call.
 *
 *	EIO		An I/O error occurred while making or updating a
 *			directory entry.
 *
 *	EISDIR		"new" points to a directory by "old" points to a file
 *			that is not a directory.
 *
 *	ELOOP		Too many symbolic links were encountered in translating
 *			"old" or "new".
 *
 *	EMULTIHOP	Components of pathnames require hopping to multiple
 *			remote machines and the filesystem type does not allow
 *			it.
 *
 *	ENAMETOOLONG	The length of the "old" or "new" argument exceeds
 *			{PATH_MAX}, or the length of an "old" or "new"
 *			component exceeds {NAME_MAX} while _POSIX_NO_TRUNC is
 *			in effect.
 *
 *	ENOENT		A component of either "old" or "new" does not exists,
 *			or the file referred to by either "old" or "new" does
 *			not exist.
 *
 *	ENOLINK		Pathnames point to a remote machine and the link to
 *			that machine is no longer active.
 *
 *	ENOSPC		The directory that would contain "new" is out of
 *			space.
 *
 *	ENOTDIR		A component of either path prefix is not a directory;
 *			or the "old" parameter names a directory and the "new"
 *			parameter names a file.
 *
 *	EROFS		The requested operation requires writing in a
 *			directory on a read-only file system.
 *
 *	EXDEV		The links named by "old" and "new" are on different
 *			file systems.
 *
 *-DIAGNOSTICS:
 *	Upon successful completion, a value of 0 is returned. Otherwise, a
 *	value of -1 is returned and "errno" is set to indicate the error.
 *
 *-NOTES:
 *	The system can deadlock if there is a loop in the file system graph.
 *	Such a loop takes the form of an entry in directory "a", say "a/foo",
 *	being a hard link to directory "b", and an entry in directory "b", say
 *	"b/bar", being a hard link to directory "a". When such a loop exists
 *	and two separate processes attempt to perform rename () "a/foo" to
 *	"b/bar" and rename () "b/bar" to "a/foo" respectively, the system may
 *	deadlock attempting to lock both directories for modification. The
 *	system administrator should replace hard links to directories with
 *	symbolic links.
 */

#if	__USE_PROTO__
int urename (__CONST__ char * old, __CONST__ char * new)
#else
int
urename (old, new)
__CONST__ char * old;
__CONST__ char * new;
#endif
{
	/*
	 * NIGEL: I am putting this in as a quick hack simply because Steve's
	 * C-library version of rename () is absolutely pathetic. We don't
	 * comply with the semantics above w.r.t. directories; only regular
	 * files can be renamed.
	 *
	 * Believe me, even this is a massive improvement, not to mention
	 * having been required for iBCS2 compliance. In theory, we should do
	 * all the permissions checks before doing any links...
	 */

	if (do_link (old, new, IOUSR, 1) != 0)
		return -1;

	(void) do_unlink (old, IOUSR);

	if (get_user_error ()) {
		/*
		 * Remove the link to "new"; the original C-library version
		 * didn't even do this much.
		 */

		set_user_error (do_unlink (new, IOUSR));
		return -1;
	}

	return 0;
}


/*
 *-STATUS:
 *	POSIX.1
 *
 *-NAME:
 *	getpgrp ()	Get process group
 *
 *-SYNOPSIS:
 *	#include <sys/types.h>
 *	#include <unistd.h>
 *
 *	pid_t getpgrp (void);
 *
 *-DESCRIPTION:
 *	getpgrp () returns the process group ID of the calling process.
 *
 *-DIAGNOSTICS:
 *	getpgrp () is always successful, and no return value is reserved to
 *	indicate an error.
 */

#if	__USE_PROTO__
__pid_t getpgrp (void)
#else
__pid_t
getpgrp ()
#endif
{
	return SELF->p_group;
}


/*
 *-STATUS:
 *	SVR4 deprecated
 *
 *-NAME:
 *	setpgrp ()	set process group ID
 *
 *-SYNOPSIS:
 *	#include <sys/types.h>
 *	#include <unistd.h>
 *
 *	pid_t setpgrp (void);
 *
 *-DESCRIPTION:
 *	If the calling process is not already a session leader, setpgrp ()
 *	sets the process group ID and session ID of the calling process to the
 *	process ID of the calling process, and releases the calling process's
 *	controlling terminal.
 *
 *-DIAGNOSTICS:
 *	setpgrp () returns the value of the new process group ID.
 *
 *-NOTES:
 *	setpgrp () is being phased out on favour of the setsid () function.
 */

#if	__USE_PROTO__
__pid_t setpgrp (void)
#else
__pid_t
setpgrp ()
#endif
{
	if (SELF->p_group != SELF->p_pid)
		SELF->p_ttdev = NODEV;

	return SELF->p_sid = SELF->p_group = SELF->p_pid;
}


/*
 *-STATUS:
 *	POSIX.1
 *
 *-NAME:
 *	setsid ()	set session ID
 *
 *-SYNOPSIS:
 *	#include <sys/types.h>
 *	#include <unistd.h>
 *
 *	pid_t setsid (void);
 *
 *-DESCRIPTION:
 *	If the calling process is not already a process group leader,
 *	setsid () sets the process group ID and session ID of the calling
 *	process to the process ID of the calling process, and releases the
 *	process's controlling terminal.
 *
 *	setsid () will fail and return an error if the following is true:
 *
 *	EPERM		The calling process is already a process group leader,
 *			or there are processes other than the calling process
 *			whose process group ID is equal to the process ID of
 *			the calling process.
 *
 *-WARNING:
 *	If the calling process is the last member of a pipeline started by a
 *	job control shell, the shell may make the calling process a process
 *	group leader. The other processes of the pipeline become members of
 *	that process group. In this case, the call to setsid () will fail. For
 *	this reason, a process that calls setsid () and expects to be part of
 *	a pipeline should first fork; the parent should exit and the child
 *	should call setsid (), thereby ensuring that the process will work
 *	reliably when started by both job control shells and non-jon control
 *	shells.
 *
 *-DIAGNOSTICS:
 *	Upon successful completion, setsid () returns the calling process's
 *	session ID. Otherwise, a value of -1 is returned and "errno" is set to
 *	indicate the error.
 */

#if	__USE_PROTO__
__pid_t setsid (void)
#else
__pid_t
setsid ()
#endif
{
	if (SELF->p_pid == SELF->p_group ||
	    process_find_pgid (SELF->p_pid, iter_find_any, NULL)) {
		set_user_error (EPERM);
		return -1;
	}

	SELF->p_ttdev = NODEV;
	return SELF->p_sid = SELF->p_group = SELF->p_pid;
}


/*
 *-STATUS:
 *	POSIX.1
 *
 *-NAME:
 *	setpgid ()	set process group ID
 *
 *-SYNOPSIS:
 *	#include <sys/types.h>
 *	#include <unistd.h>
 *
 *	int setpgid (pid_t pid, pid_t pgid);
 *
 *-DESCRIPTION:
 *	setpgid () set the process group ID of the process with ID "pid" to
 *	"pgid". If "pgid" is equal to "pid", the process becomes a process
 *	group leader. If "pgid" is not equal to "pid", the process becomes a
 *	member of an existing process group.
 *
 *	If "pid" is equal to 0, the process ID of the calling process is used.
 *	If "pgid" is qual to 0, the process specified by "pid" becomes a
 *	process group leader.
 *
 *	setpgid () fails and returns an error if one or more of the following
 *	are true:
 *
 *	EACCES		"pid" matches the process ID of a child process of the
 *			calling process and the child process has successfully
 *			executed an exec () function.
 *
 *	EINVAL		"pgid" is less than "(pid_t) 0", or greater than or
 *			equal to {PID_MAX}.
 *
 *	EINVAL		The calling process has a controlling terminal that
 *			does not support job control.
 *
 *	EPERM		The process indicated by the "pid" argument is a
 *			session leader.
 *
 *	EPERM		"pid" matches the process ID of a child process of the
 *			calling process, and the child process is not in the
 *			same session as the calling process.
 *
 *	EPERM		"pgid" does not match the process ID of the process
 *			indicated by the "pid" argument and there is no
 *			process with a process group ID that matches "pgid" in
 *			the same session as the calling process.
 *
 *	ESRCH		"pid" does not match the process ID of the calling
 *			process or of a child process of the calling process.
 *
 *-DIAGNOSTICS:
 *	Upon successful completion, setpgid () returns a value of 0.
 *	Otherwise, a value of -1 is returned and "errno" is set to indicate
 *	the error.
 */

#if	__USE_PROTO__
int setpgid (__pid_t pid, __pid_t pgid)
#else
int
setpgid (pid, pgid)
__pid_t		pid;
__pid_t		pgid;
#endif
{
	__proc_t      *	procp;

	if (! __IN_RANGE (0, pid, PID_MAX - 1)) {
		set_user_error (EINVAL);
		return -1;
	}

	if (pid == 0)
		pid = SELF->p_pid;

	if ((procp = process_find_pid (pid)) == NULL ||
	    (procp != SELF && procp->p_ppid != SELF->p_pid)) {
	    	/*
	    	 * Either cannot find the process, or the process is not us
	    	 * or one of our children.
	    	 */

		set_user_error (ESRCH);
		return -1;
	}

	if (procp != SELF && (procp->p_flags & PFEXEC) != 0) {
		/*
		 * The process is a child that has performed an exec ().
		 */

		set_user_error (EACCES);
		return -1;
	}

	if (pgid == 0)
		pgid = procp->p_pid;

	if (procp->p_sid == procp->p_pid || procp->p_sid != SELF->p_sid ||
	    (procp->p_pid != pgid &&
	     session_find_pgid (SELF, pgid, iter_find_any, NULL) == 0)) {
		/*
		 * Ether the process is a session leader or the process is not
		 * in our session, or the process is trying to join a process
		 * group but there is no member of the group in our session.
		 */

		set_user_error (EPERM);
		return -1;
	}

	/*
	 * POSIX.1 would allow us to proceed at this point; SVR4 says that we
	 * should give EINVAL because our controlling terminal does not
	 * support job control.
	 */

#if	1
	set_user_error (EINVAL);
	return -1;
#else
	procp->p_group = pgid;
	return 0;
#endif
}


/*
 * iBCS2 system-call entry point which dispatches to the process-group-related
 * functions.
 */

#if	__USE_PROTO__
int upgrpsys (int flag, __pid_t arg1, __pid_t arg2)
#else
int
upgrpsys (flag, arg1, arg2)
int		flag;
__pid_t		arg1;
__pid_t		arg2;
#endif
{
	switch (flag) {
	case 0:	return getpgrp ();
	case 1:	return setpgrp ();
	case 2:	return setpgid (arg1, arg2);
	case 3:	return setsid ();

	case 4:	/* reserved */
	case 5:	/* reserved */
	default:
		set_user_error (ENOSYS);
		return -1;
	}
}


/*
 * Poll devices for input/output events.
 */

int
upoll(pollfds, npoll, msec)
struct pollfd * pollfds;
unsigned long npoll;
int msec;
{
	struct pollfd * pollp;	/* current poll pointer */
	int		i;
	int		ret;

	/*
	 * Validate number of polls.
	 */

	if (! __IN_RANGE (0, npoll, NOFILE)) {
		set_user_error (EINVAL);
		return -1;
	}

	/*
	 * If there are any fd's to poll
	 *   validate address of polling information.
	 * npoll of 0 is legal, allows user a short delay.
	 */

	if (npoll != 0 &&
	    (pollfds == NULL ||
	     ! useracc (pollfds, npoll * sizeof (struct pollfd), 1))) {
		set_user_error (EFAULT);
		return -1;
	}

	if ((ret = pwalloc (npoll, msec)) != 0) {
		set_user_error (ret);
		return -1;
	}

	for (;;) {
		/*
		 * Service each poll in turn.
		 */

		ret = 0;

		for (i = npoll, pollp = pollfds; i > 0; -- i, pollp ++) {
			fd_t		fd;
			__fd_t	      *	fdp;
			short		events;
			short		revents;

			/*
			 * Fetch file descriptor.
			 */

			fd = getuwd (& pollp->fd);
			events = getusd (& pollp->events);

			/*
			 * Ignore negative file descriptors.
			 */

			if (! __IN_RANGE (0, fd, NOFILE - 1)) {
				revents = 0;
				goto remember;
			}

			/*
			 * Validate file descriptor.
			 */

			if ((fdp = fd_get (fd)) == NULL) {
				revents = POLLNVAL;
				goto remember;
			}

			switch (fdp->f_ip->i_mode & __IFMT) {
			case __IFCHR:
				revents = dpoll (fdp->f_ip->i_rdev,
						 events, msec,
						 fdp->f_ip->i_private);
				break;

			case __IFPIP:
				revents = ppoll (fdp->f_ip, events, msec);
				break;

			default:
				revents = POLLNVAL;
				break;
			}

			/*
			 * Remember reponses.
			 */
remember:
			putusd (& pollp->revents, revents);

			/*
			 * Record number of non-zero responses.
			 */

			if (revents) {
				msec = 0;
				ret ++;
			}
		}

		/*
		 * Non-blocking poll or poll response received.
		 */

		if (msec == 0)
			break;

		/*
		 * Wait for polled event, poll timeout, or signal.
		 */

		switch (pollsleep (msec)) {

		case __POLL_SIGNALLED:
			set_user_error (EINTR);
			break;

		case __POLL_NOMEM:
			set_user_error (EAGAIN);
			break;

		case __POLL_TIMEOUT:
			ret = 0;
			break;

		default:
			continue;
		}

		break;
	}

	pwfree ();
	return ret;
}

