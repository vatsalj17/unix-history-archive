#include <sys/types.h>
#include <sys/filio.h>

extern char *strncpy ();
extern char *strncat ();

setstsrc(fd, name)
char *name;
{
	char buf[SSRCSIZ+1];
	strncpy(buf, name?name:"", sizeof(buf)-1);
	buf[sizeof(buf)-1] = 0;
	return ioctl(fd, FIOSSRC, buf);
}

set2stsrc(fd, s1, s2)
char *s1;
char *s2;
{
	char buf[SSRCSIZ+1];
	strncpy(buf, s1?s1:"", sizeof(buf)-1);
	buf[sizeof(buf)-1] = 0;
	strncat(buf, s2?s2:"", sizeof(buf)-1-strlen(buf));
	return ioctl(fd, FIOSSRC, buf);
}

char *
getstsrc(fd) 
{
	static char buf[SSRCSIZ+1];
	if(ioctl(fd, FIOGSRC, buf) == -1)
		return 0;
	else
		return buf;
}

