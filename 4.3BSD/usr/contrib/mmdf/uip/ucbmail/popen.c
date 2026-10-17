/*
 *  P O P E N . C 
 *
 *  EE/CIS Computer Lab
 *  Department of Computer and Information Sciences
 *  Department of Electrical Engineering
 *  University of Delaware
 *
 *  REVISION HISTORY:
 *
 *  $Revision: 1.3 $
 *
 *  $Log:	popen.c,v $
 * Revision 1.3  85/11/16  15:19:15  galvin
 * Added define for sigmask for backward compatibility from 4.3bsd to 4.2bsd.
 * 
 * Revision 1.2  85/11/16  14:43:38  galvin
 * Added comment header for revision history.
 * 
 *
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char *sccsid = "@(#)popen.c	5.2 (Berkeley) 6/21/85";
#endif not lint

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include "./sigretro.h"
#define	tst(a,b)	(*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1
static	int	popen_pid[20];

#ifndef VMUNIX
#define vfork	fork
#endif VMUNIX
#ifndef	SIGRETRO
#define	sigchild()
#endif

FILE *
popen(cmd,mode)
char	*cmd;
char	*mode;
{
	int p[2];
	register myside, hisside, pid;

	if(pipe(p) < 0)
		return NULL;
	myside = tst(p[WTR], p[RDR]);
	hisside = tst(p[RDR], p[WTR]);
	if((pid = vfork()) == 0) {
		/* myside and hisside reverse roles in child */
		sigchild();
		close(myside);
		dup2(hisside, tst(0, 1));
		close(hisside);
		execl("/bin/csh", "sh", "-c", cmd, 0);
		_exit(1);
	}
	if(pid == -1)
		return NULL;
	popen_pid[myside] = pid;
	close(hisside);
	return(fdopen(myside, mode));
}

pclose(ptr)
FILE *ptr;
{
	register f, r;
	int status, omask;
	extern int errno;

	f = fileno(ptr);
	fclose(ptr);
# ifdef V4_2BSD
	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
# endif V4_2BSD
# ifdef V4_1BSD
	sighold(SIGINT);
	sighold(SIGQUIT);
	sighold(SIGHUP);
# endif V4_2BSD
	while((r = wait(&status)) != popen_pid[f] && r != -1 && errno != EINTR)
		;
	if(r == -1)
		status = -1;
# ifdef V4_2BSD
	sigsetmask(omask);
# endif V4_2BSD
# ifdef V4_1BSD
	sigrelse(SIGINT);
	sigrelse(SIGQUIT);
	sigrelse(SIGHUP);
# endif V4_2BSD
}
