#include	<errno.h>
#include	<sys/types.h>
#include	<sys/file.h>

#define	HOSTNAMESIZE	32
#ifndef	hostnamelen
#define	hostnamelen	hnamelen
#endif
extern	errno;
static	char	hostname[HOSTNAMESIZE];
static	int	hostnamelen	= HOSTNAMESIZE;

gethostname(p, len)
register char	*p;
{
	int fd;
	register i;
	register char *q = hostname;

	if ((fd = open("/etc/localhostname", FATT_RDONLY)) == -1) {
		errno	= EFAULT;
		return(-1);
	}
	hostnamelen = read(fd, &hostname, sizeof hostname);
	if (hostnamelen < 0) {
		errno	= EFAULT;
		return(-1);
	}
	else
		for (i = 0; i < (len < hostnamelen ?  len : hostnamelen); i++)
			if (*q == '\n') {
				*p	= '\0';
				break;
			}
			else
				*p++	= *q++;
	return(0);
}
