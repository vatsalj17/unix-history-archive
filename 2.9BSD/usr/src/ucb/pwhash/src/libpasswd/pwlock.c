/*	@(#)pwlock.c	2.1	SCCS id keyword	*/
#include <whoami.h>
#include <stdio.h>

static	char	lockmode;
static	char	TEMP[] =	"/etc/plXXXXXX";
static	char	LOCK[] =	"/etc/ptmp";

char	*mktemp();

pwlock(mode)
char	*mode;
{
	register	result;

	lockmode = *mode;
	close(creat(mktemp(TEMP), 0600));
	result =  link(TEMP, LOCK);
	unlink(TEMP);
	return(result);
}

pwunlock()
{
#ifdef UCB_PWHASH
	if (lockmode != 'r')
		pwtable();
#endif
	unlink(LOCK);
	return;
}
