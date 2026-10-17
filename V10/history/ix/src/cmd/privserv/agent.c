#include <errno.h>
#include <signal.h>
#include <ipc.h>
#include <sys/filio.h>

char *privs = "/cs/priv";

extern void exit();
main(ac, av)
register char **av;
{
	char buf[1024];
	int nflag=0;
	int n, fd;

	while(ac>1 && av[1][0] == '-') {
		if(strcmp(av[1], "-n")==0) {
			nflag++;
		}
		if(strcmp(av[1], "-f")==0) {
			if(ac <= 2) {
				fprint(2, "bad -f option");
				exit(1);
			}
			privs = av[2];
			ac--, av++;
		}
		ac--, av++;
		if(ac < 1) {
			fprint(2, "arg count");
			exit(1);
		}
	}
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	fd = ipcopen(privs, "light");
	if(fd==-1) {
		fprint(2, "%s:%s\n", privs, errstr);
		exit(1);
	}
	if(nflag)
		fprint(fd,"n");
	fprint(fd,"t");
	for(n=0; n<4; n++) {
		if(ioctl(fd, FIOSNDFD, &n) == -1) {
			if(errno == EBADF) continue;
			fprint(2,"can't pass fd %d\n", n);
			exit(1);
		}
	}

	ac--;
	av++;

	if(ac<1) {
		fprint(fd, "s", 1);
	} else {
		fprint(fd, "r", 1);
		for(n=0; n<ac; n++)
			fprint(fd, "%d\n%s\n", strlen(av[n]), av[n]);
		fprint(fd, "-1\n");
	}
	while((n=read(fd,buf,1))>0)
		write(1,buf,n);
	write(1,"",0);
	exit(0);
}
