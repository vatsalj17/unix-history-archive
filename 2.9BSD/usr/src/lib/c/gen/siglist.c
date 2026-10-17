/*	@(#)siglist.c	4.2 (Berkeley) 2/10/83	*/

#include	<sys/localopts.h>
#include	<signal.h>

char	*sys_siglist[NSIG] = {
	"Signal 0",
	"Hangup",			/* SIGHUP */
	"Interrupt",			/* SIGINT */
	"Quit",				/* SIGQUIT */
	"Illegal instruction",		/* SIGILL */
	"Trace/BPT trap",		/* SIGTRAP */
	"IOT trap",			/* SIGIOT */
	"EMT trap",			/* SIGEMT */
	"Floating point exception",	/* SIGFPE */
	"Killed",			/* SIGKILL */
	"Bus error",			/* SIGBUS */
	"Segmentation fault",		/* SIGSEGV */
	"Bad system call",		/* SIGSYS */
	"Broken pipe",			/* SIGPIPE */
	"Alarm clock",			/* SIGALRM */
	"Terminated",			/* SIGTERM */
#ifdef	UCB_NET
	"Urgent I/O condition",		/* SIGURG */
#else
	"Signal 16",			/* unused */
#endif
#ifdef	MENLO_JCL
	"Stopped (signal)",		/* SIGSTOP */
	"Stopped",			/* SIGTSTP */
	"Continued",			/* SIGCONT */
	"Child exited",			/* SIGCHLD */
	"Stopped (tty input)",		/* SIGTTIN */
	"Stopped (tty output)",		/* SIGTTOU */
	"Signal 23",
	"Signal 24",
	"Signal 25",
	"Signal 26",
	"Signal 27",
	"Signal 28",
	"Signal 29",
	"Signal 30",
	"Signal 31"
#endif
};
