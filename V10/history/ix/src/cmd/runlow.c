#include <errno.h>

char *noargs[] = { 0 };
extern char **environ;

main(argc,argv)
char **argv;
{
	if(argc != 2) {
		fprint(2,"usage: runlow name\n");
		exit(1);
	}
	execv0(argv[1]);
	perror("runlow");
	return 1;
}
/*
 *	execv0 is a private version of execvp, with empty
 *	arglist and empty environment
 */
#define	NULL	0

static	char shell[] =	"/bin/sh";
char	*execat(), *getenv(), *strchr();
extern	errno;


execv0(name)
{
	char *pathstr;
	register char *cp;
	char fname[128];
	register unsigned etxtbsy = 1;
	register eacces = 0;

	if ((pathstr = getenv("PATH")) == NULL)
		pathstr = ":/bin:/usr/bin";
	cp = strchr(name, '/')? "": pathstr;
	environ = noargs;

	do {
		cp = execat(cp, name, fname);
	retry:
		execv(fname,noargs);
		switch(errno) {
		case ETXTBSY:
			if (++etxtbsy > 5)
				return(-1);
			sleep(etxtbsy);
			goto retry;
		case EACCES:
			eacces++;
			break;
		case ENOMEM:
		case E2BIG:
			return(-1);
		}
	} while (cp);
	if (eacces)
		errno = EACCES;
	return(-1);
}

static char *
execat(s1, s2, si)
register char *s1, *s2;
char *si;
{
	register char *s;

	s = si;
	while (*s1 && *s1 != ':')
		*s++ = *s1++;
	if (si != s)
		*s++ = '/';
	while (*s2)
		*s++ = *s2++;
	*s = '\0';
	return(*s1? ++s1: 0);
}
