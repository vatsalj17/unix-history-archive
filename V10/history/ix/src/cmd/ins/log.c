#include "ins.h"

char logfile[] = "/usr/spool/ins/log";
char lockfile[] = "/usr/spool/ins/lock";

int vflag;
int logfd;

static char *argstring;
setarg(ac, av)
char **av;
{
	int i;
	cpbuf[0] = 0;
	for(i=0; i<ac; i++) {
		strcat(cpbuf, " '");
		strcat(cpbuf, av[i]);
		strcat(cpbuf, "'");
	}
	argstring = strdup(cpbuf+1);
}

lock(flag) {
	static lockfd;

	lockfd = open(lockfile, flag);
	if(lockfd == -1 && errno == ECONC) {
		fprint(2, "ins: concurrent active ins, try later\n");
		exit(1);
	} else if(lockfd==-1)
		perror(lockfile);
	ioctl(lockfd, FIOCLEX, (void*)0);
}
int Econv();
static logpid;
setlog(vf){
	long t;
	char buf[10];

	if(access(lockfile,0)==-1 && errno==ENOENT) {
		close(creat(lockfile, 03664));
		chmod(lockfile, 03664);	/* CREAT bug */
	}
	if(nflag) {
		logfd = -1;
		lock(0);
		return;
	}
	vflag = vf;
	lock(1);
	logfd = open(logfile, 1);
	if(logfd == -1 && errno == ENOENT)
		logfd = creat(logfile, 05664);
	if(logfd == -1) {
		fprint(2, "WARNING: this ins session not logged\n");
		return;
	}
	ioctl(logfd, FIOCLEX, (void*)0);
	lseek(logfd, 0L, 2);		/* SHOULD be append only */
	fmtinstall('E', Econv);
	logpid = getpid();
	getlogname(buf);
	buf[8]=0;
	time(&t);
	log("\nins", "%s %s\n", buf, argstring); 
	log("date", "%s", ctime(&t)); 
}
Econv()
{
	int f1;
	char ebuf[10];

	if(errno!=0) {
		sprint(ebuf,"e=%d", errno);
		f1 = strlen(ebuf);
		strconv(ebuf, f1,f1);
	}
	return 0;
}
/*PRINTFLIKE2*/
log(type, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
char *
type, *format, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10;
{
	int x;

	x = errno;
	sprint(cpbuf+sprint(cpbuf, "%s.%d ", type, logpid), format,
		a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	if(logfd>1)
		fprint(logfd, "%s", cpbuf);
	if(vflag)
		print("%s", cpbuf);
	errno = x;
}
