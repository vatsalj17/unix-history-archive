#include <errno.h>
#include <sys/types.h>
#include <sys/dir.h>	/* for DIRSIZ */

char *strcpy(), *malloc();
extern errno;
/*
 * Create file with unique name.
 * Optionally return file descriptor.
 * Works with blind directories.
 */
char *
mkunique(name, mode, afd)
	char *name;
	int *afd;
{
	static long num;
	register char *s, *t;
	register fd, i, suffix;
	int pid = getpid()*1000;
	int len = strlen(name);

	if((s = malloc(DIRSIZ+1+len)) == 0)
		return 0;
	for(;;) {
		num = (num+1)%1000;
		suffix = pid + num;
		t = strcpy(s, name) + len + 8;
		*t = 0;
		for(i=8; --i>=0; suffix/=10)
			*--t = suffix%10 + '0';
		if(access(s, 0) == -1)
			break;
	}
	while(t>s && t[-1]!='/')
		t--;
	fd = creat(s, mode);
	if(fd == -1)
		return 0;
	if(afd)
		*afd = fd;
	else 
		(void)close(fd);
	return s;
}

char *
tmpnam()
{
	return mkunique("/tmp/",0644,(int*)0);
}

mkanon()
{
	register int fd;
	register char *s = mkunique("/tmp/.", 0600, (int*)0);

	fd = open(s, 2);
	if(fd != -1)
		unlink(s);
	return fd;
}
