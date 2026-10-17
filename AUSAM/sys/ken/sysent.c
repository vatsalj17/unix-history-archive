#
#include "../defines.h"
#include "../param.h"
/*
 */

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 * Each row contains the number of arguments expected
 * and a pointer to the routine.
 */
int	sysent[]
{
	0, &nosys,			/*  0 = indir  fix028 */
	0, &rexit,			/*  1 = exit */
	0, &fork,			/*  2 = fork */
	2, &read,			/*  3 = read */
	2, &write,			/*  4 = write */
	2, &open,			/*  5 = open */
	0, &close,			/*  6 = close */
	0, &wait,			/*  7 = wait */
	2, &creat,			/*  8 = creat */
	2, &link,			/*  9 = link */
	1, &unlink,			/* 10 = unlink */
	2, &exec,			/* 11 = exec */
	1, &chdir,			/* 12 = chdir */
	0, &gtime,			/* 13 = time */
	3, &mknod,			/* 14 = mknod */
	2, &chmod,			/* 15 = chmod */
	2, &chown,			/* 16 = chown */
	1, &sbreak,			/* 17 = break */
	2, &stat,			/* 18 = stat */
	2, &seek,			/* 19 = seek */
	0, &getpid,			/* 20 = getpid */
	3, &smount,			/* 21 = mount */
	1, &sumount,			/* 22 = umount */
	0, &setuid,			/* 23 = setuid */
	0, &getuid,			/* 24 = getuid */
	0, &stime,			/* 25 = stime */
	3, &ptrace,			/* 26 = ptrace */
	0, &tell,			/* 27 = tell  fix034 */
	1, &fstat,			/* 28 = fstat */
	0, &nosys,			/* 29 = x */
#ifdef SMDATE
	1, &smdate,			/* 30 = smdate */
#else
	1, &nullsys,			/* 30 = smdate */
#endif	SMDATE
	1, &stty,			/* 31 = stty */
	1, &gtty,			/* 32 = gtty */
#ifdef	ACCESS
	2, &saccess,			/* 33 = access */
#else
	0, &nosys,			/* 33 = x */
#endif	ACCESS
#ifdef	AUSAMSCHD
	1, &nice,			/* 34 = nice */
#else
	0, &nice,			/* 34 = nice */
#endif	AUSAMSCHD
	0, &sslep,			/* 35 = sleep */
	0, &sync,			/* 36 = sync */
	1, &kill,			/* 37 = kill */
	0, &getswit,			/* 38 = switch */
#ifdef	TIME_LIMITS
	0, &tlimit,			/* 39 = tlimit */
	0, &clktim,			/* 40 = clktim */
#else
	0, &nosys,			/* 39 = tlimit */
	0, &nosys,			/* 40 = clktim */
#endif	TIME_LIMITS
	0, &dup,			/* 41 = dup */
	0, &pipe,			/* 42 = pipe */
	1, &times,			/* 43 = times */
	4, &profil,			/* 44 = prof */
	0, &nosys,			/* 45 = tiu */
#ifdef	GROUP_ACCESS
	0, &setgid,			/* 46 = setgid */
	0, &getgid,			/* 47 = getgid */
#else
	0, &nullsys,			/* 46 = setgid */
	0, &nullsys,			/* 47 = getgid */
#endif	GROUP_ACCESS
	2, &ssig,			/* 48 = sig */
#ifdef	U_LOCK
	0, &slockon,			/* 49 = slockon */
	0, &slockoff,			/* 50 = slockoff */
#else
	0, &nosys,			/* 49 = x */
	0, &nosys,			/* 50 = x */
#endif	U_LOCK
#ifdef	U_DELAY
	0, &sdelay,			/* 51 = sdelay */
#else
	0, &nosys,			/* 51 = x */
#endif	U_DELAY
	0, &nosys,			/* 52 = x */
#ifdef	SHARED_DATA
	0, &enqueue,			/* 53 = pp */
	0, &dequeue,			/* 54 = vv */
#else
	0, &nosys,			/* 53 = pp */
	0, &nosys,			/* 54 = vv */
#endif	SHARED_DATA
#ifdef	SETPSW
	1, &setpsw,			/* 55 = setpsw */
#else
	0, &nosys,			/* 55 = x */
#endif	SETPSW
#ifdef	LOCKING
	1, &slock,			/* 56 = slock */
#else
	0, &nosys,			/* 56 = x */
#endif	LOCKING
	0, &nosys,			/* 57 = x */
	0, &nosys,			/* 58 = x */
#ifdef	TTY_CONNECT
	2, &ttyconnect,			/* 59 = ttyconnect */
#else
	0, &nosys,			/* 59 = x */
#endif	TTY_CONNECT
#ifdef	AUSAML
	2, &limits,			/* 60 = limits */
#else
	0, &nosys,			/* 60 = x */
#endif	AUSAML
#ifdef	GPROCS
	0, &gprocs,			/* 61 = getproc */
#else	GPROCS
#ifdef	GETTAB
	0, &gettable,			/* 61 = gettable */
#else	GETTAB
	0, &nosys,			/* 61 = getproc */
#endif	GETTAB
#endif	GPROCS
	0, &nosys,			/* 62 = x */
	0, &nosys			/* 63 = x */
};
