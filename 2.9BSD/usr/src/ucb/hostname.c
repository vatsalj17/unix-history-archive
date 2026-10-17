/*
 * hostname - get (or set) hostname
 */

#include	<stdio.h>
#include	<sys/types.h>

char hostname[32];
extern int errno;

main(argc, argv)
char	*argv[];
{
	int myerrno;

	argc--;
	argv++;
	if (argc) {
		if (sethostname(*argv))
			perror("sethostname");
		myerrno	= errno;
	}
	else	{
		gethostname(hostname, sizeof(hostname));
		myerrno	= errno;
		printf("%s\n", hostname);
	}
	exit(myerrno);
}

sethostname(s)
char *s;
{
	FILE	*fopen();
	register FILE	*fp;

	if ((fp = fopen("/etc/localhostname", "w")) != (FILE *) NULL) {
		fprintf(fp, "%s\n", s);
		fclose(fp);
		(void) chmod("/etc/localhostname", 0644);
		if (ferror(fp))
			return(-1);
		else
			return(0);
	}
	else
		return(-1);
}
