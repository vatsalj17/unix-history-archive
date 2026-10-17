/* @(#)errlst.c	4.4 (Berkeley) 82/04/01 */
char	*sys_errlist[] = {
	"Error 0",
	"Not owner",				/*  1 - EPERM */
	"No such file or directory",		/*  2 - ENOENT */
	"No such process",			/*  3 - ESRCH */
	"Interrupted system call",		/*  4 - EINTR */
	"I/O error",				/*  5 - EIO */
	"No such device or address",		/*  6 - ENXIO */
	"Arg list too long",			/*  7 - E2BIG */
	"Exec format error",			/*  8 - ENOEXEC */
	"Bad file number",			/*  9 - EBADF */
	"No children",				/* 10 - ECHILD */
	"No more processes",			/* 11 - EAGAIN */
	"Not enough core",			/* 12 - ENOMEM */
	"Permission denied",			/* 13 - EACCES */
	"Bad address",				/* 14 - EFAULT */
	"Block device required",		/* 15 - ENOTBLK */
	"Exclusive use facility busy",		/* 16 - EBUSY */
	"File exists",				/* 17 - EEXIST */
	"Cross-device link",			/* 18 - EXDEV */
	"No such device",			/* 19 - ENODEV */
	"Not a directory",			/* 20 - ENOTDIR */
	"Is a directory",			/* 21 - EISDIR */
	"Invalid argument",			/* 22 - EINVAL */
	"File table overflow",			/* 23 - ENFILE */
	"Too many open files",			/* 24 - EMFILE */
	"Inappropriate ioctl for device",	/* 25 - ENOTTY */
	"Text file busy",			/* 26 - ETXTBSY */
	"File too large",			/* 27 - EFBIG */
	"No space left on device",		/* 28 - ENOSPC */
	"Illegal seek",				/* 29 - ESPIPE */
	"Read-only file system",		/* 30 - EROFS */
	"Too many links",			/* 31 - EMLINK */
	"Broken pipe",				/* 32 - EPIPE */

/* math software */
	"Argument too large",			/* 33 - EDOM */
	"Result too large",			/* 34 - ERANGE */

/* quotas */
	"Disk quota exceeded",			/* 35 - EQUOT */

/* symbolic links */
	"Too many levels of symbolic links",	/* 36 - ELOOP */

/* non-blocking and interrupt i/o */
	"Operation would block",		/* 37 - EWOULDBLOCK */
#ifdef	UCB_NET
	"Operation now in progress",		/* 38 - EINPROGRESS */
	"Operation already in progress",	/* 39 - EALREADY */

/* ipc/network software */

	/* argument errors */
	"Socket operation on non-socket",	/* 40 - ENOTSOCK */
	"Destination address required",		/* 41 - EDESTADDRREQ */
	"Message too long",			/* 42 - EMSGSIZE */
	"Protocol wrong type for socket",	/* 43 - EPROTOTYPE */
	"Protocol not available",		/* 44 - ENOPROTOOPT */
	"Protocol not supported",		/* 45 - EPROTONOSUPPORT */
	"Socket type not supported",		/* 46 - ESOCKTNOSUPPORT */
	"Operation not supported on socket",	/* 47 - EOPNOTSUPP */
	"Protocol family not supported",	/* 48 - EPFNOSUPPORT */
	"Address family not supported by protocol family",
						/* 49 - EAFNOSUPPORT */
	"Address already in use",		/* 50 - EADDRINUSE */
	"Can't assign requested address",	/* 51 - EADDRNOTAVAIL */

	/* operational errors */
	"Network is down",			/* 52 - ENETDOWN */
	"Network is unreachable",		/* 53 - ENETUNREACH */
	"Network dropped connection on reset",	/* 54 - ENETRESET */
	"Software caused connection abort",	/* 55 - ECONNABORTED */
	"Connection reset by peer",		/* 56 - ECONNRESET */
	"No buffer space available",		/* 57 - ENOBUFS */
	"Socket is already connected",		/* 58 - EISCONN */
	"Socket is not connected",		/* 59 - ENOTCONN */
	"Can't send after socket shutdown",	/* 60 - ESHUTDOWN */
	"Too many references: can't splice",	/* 61 - ETOOMANYREFS */
	"Connection timed out",			/* 62 - ETIMEDOUT */
	"Connection refused",			/* 63 - ECONNREFUSED */
	"File name too long",			/* 64 - ENAMETOOLONG */
	"Host is down",				/* 65 - EHOSTDOWN */
	"Host is unreachable",			/* 66 - EHOSTUNREACH */
};
#endif	UCB_NET
int	sys_nerr = { sizeof sys_errlist/sizeof sys_errlist[0] };
