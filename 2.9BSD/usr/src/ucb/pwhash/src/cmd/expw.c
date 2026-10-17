static	char	sccsid[] = "@(#)expw.c	2.8";	/*	SCCS id keyword	*/

#include <stdio.h>
#define PASSWD	"/etc/passwd"
#include <signal.h>

main(argc, argv)
char *argv[];
{
	char *editor, *p;
	
	if (pwlock("w")) {
		printf("Temp file busy\n");
		exit(-1);
	}
	
	p = rindex(argv[0], '/');
	if (*++p == 'v')
		editor = "vi";
	else
		editor = "ex";
	
	if (!fork()) {
		execlp(editor, editor, PASSWD, 0);
		printf("No ex!\n");
		exit(-2);
	}
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	while (wait((int *) 0) != -1);
	printf("Making map files\n");
	fflush(stdout);
	pwunlock();
	printf("Passwd file unlocked\n");
	exit(0);
}
