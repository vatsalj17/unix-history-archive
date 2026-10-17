#include "remind.h"

main(argc, argv)
int argc;
char **argv;
{       register int i;
	register char *p;
	int errflg;
	struct passwd *pw;

	signal  (SIGHUP,SIG_IGN);          /* Ignore Hangups */

	m.nrcvrs = m.msgbytes = m.bits = m.dirsize = errflg = 0;

	for (i = 1; i++ < argc;)
	{	p = *++argv;
		if (*p>='a' && *p<='z')
			puserid(p);	/* Alphabetic -- Receiver id */
		else if (*p == '-')
			pflags(&p[1]);	/* -flags */
		else if (getdt(p))	    /* Anything else - date or time */
			{       fprintf (stderr,invalid, p);
				errflg = -1;
			}
		/*
		while( *p ) *p++ = 0;
		**/
	}

	Uid = getuid();
	if((pw=getpwuid(Uid))==NULL)
		strxfer("??",m.sender,3);
	else
	{
		strcpy(m.sender,pw->pw_name);
		if(!pw->pw_shell[0])
		    strcpy(shell,"/bin/sh");
		else
		    strcpy(shell,pw->pw_shell);
	}
#ifdef DEBUG
{
    fprintf(stderr,"Original Uid = %d\n",Uid);
    fprintf(stderr,"Sender = %s\n",m.sender);
}
#endif DEBUG
	if (m.nrcvrs == 0)
		puserid(m.sender);
	if (checkusers() || errflg) exit(1);

	if (m.msgbytes == 0 && argc != 1) getmsg();

	m.nobytes = m.msgbytes + 8*m.nrcvrs + m.dirsize;

#ifndef DEBUG
	while ((i = fork()) == -1) sleep(5);
	if (i) exit(0);  /* The rest will be done in background mode */
#endif DEBUG

	/*
	setpgrp();
	*/
	if (dtime() && argc!=1)
		deliver();
	else
		enqueue();

}
