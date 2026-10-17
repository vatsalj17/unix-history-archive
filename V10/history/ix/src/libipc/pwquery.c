#include <signal.h>
#include <ipc.h>
#include <sys/filio.h>


/*
 * trustable because checks x.cap
 */
pwquery(ufd, name, class)
char *name;
char *class;
{
	SIG_TYP fint, fquit;
	char buf[100];
	int x, fd;
	struct passfd y;

	strcpy(buf, "/cs/pw!");
	if(class && *class)
		strcat(buf, class);
	fd = ipcopen(buf, "light");
	if(fd<0) 
		return -1;
	x = ioctl(fd, FIORCVFD, &y);
	close(fd);
	if(x < 0) {
		return -1;
	}
	if(y.cap == 0) {
		close(y.fd);
		return -1;
	}

	fint = signal(SIGINT, SIG_IGN);
	fquit = signal(SIGQUIT, SIG_IGN);
	fd = y.fd;

	x = strlen(name);

	if(ioctl(fd, FIOSNDFD, &ufd)!=-1 && write(fd, name, x)==x) {
		if(read(fd, buf, 3)==3 && strncmp(buf, "ok\n", 3)==0)
			x = 1;
		else
			x = 0;
	} else x = -1;
	
	close(fd);

	signal(SIGINT, fint);
	signal(SIGQUIT, fquit);
	return x;
}
