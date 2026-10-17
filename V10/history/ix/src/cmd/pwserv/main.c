#include "pwserv.h"

int logfd = -1;
char logfile[] = "/usr/adm/pwlog";
char mtpt[] = "/cs/pw";
int cleanup();
int cleanup2();
int xxx();
char user[33];

int debt = 0;

main() {
	int n, f, fd, nfd, st;
	ipcinfo *ip;

	if(fork()!=0) exit(0);

	onexit(cleanup2);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGTERM, cleanup);
	close(0);
	close(1);
	close(2);
	close(3);

	logfd = creat(logfile, 05644);

	st = 10;
	n = 0;
	for(;;) {
		logit("pid %d", getpid());
		snarf();
		fd = ipccreat(mtpt,  "light");
		if(fd<0) {
			logit( "bad announce %s", errstr);
			sleep(st);
			if(st<300)st += st/3;
			continue;
		}
		else st = 10;
		chmod(mtpt, 0666);
		logit( "good announce");
		for(;;n++) {
			int esave;
			fd_set rdfds;

			snarf();
			FD_ZERO(rdfds);
			FD_SET(fd, rdfds);
			if(select(fd+1, &rdfds, (fd_set*)0, 1000*1000)<1)
				continue;
			signal(SIGALRM, xxx);
			alarm(5);
			ip = ipclisten(fd);
			esave = errno;
			signal(SIGALRM, SIG_IGN);
			alarm(0);
			if(ip == 0) {
				logit("listen errno=%d %s", esave, errstr);
				break;
			}
			if(*ip->machine) {
				logit("listen!%s %s!%s %d.%d", 
					ip->name, ip->machine, ip->user, 
					ip->uid, ip->gid);
				ipcreject(ip, EPERM, "xenophobia");
				continue;
			}
			nfd = ipcaccept(ip);
			f = fork();
			if(f==0) {
				close(fd);
				serve(ip, nfd);
			}
			else if(f==-1) {
				close(nfd);
				logit(" fork errno=%d", errno);
				break;
			}
			else
				close(nfd);
			debt++;
			if(n%5 == 0 || debt>2)
				twait(1);
		}
		close(fd);
		(void)funmount(mtpt);
		twait(1);
	}
}

int zap;
tiktok() {
	zap = 1;
}

twait(t) {
	int n = 0;
	zap = 0;
	signal(SIGALRM, tiktok);
	alarm(t);
	while(zap == 0 && wait((int*)0) != -1)
		n++;
	signal(SIGALRM, SIG_IGN);
	alarm(0);
	debt -= n;
}


/*
 * send client a fd and write answer on it, so he sees my x.cap.
 */

serve(ip, nfd)
ipcinfo *ip;
{
	struct passfd x;
	int p[2], n;
	char *src, *result;

	if(nfd<0) _exit(0);
	pipe(p);
	ioctl(nfd, FIOSNDFD, &p[1]);
	close(nfd);
	close(p[1]);
	nfd = p[0];
	if(ioctl(nfd, FIORCVFD, (iarg)&x)!=-1 
	    && (n=read(nfd, user,32))>0) {

		user[n] = 0;
		src = getstsrc(x.fd);
		if(src==0)
			src = "";
		if(ok(x.fd, user, ip->name, src)) {
			write(nfd, "ok\n", 3);
			logit( "ok u%d <%.8s> <%s> %o %s<%s>", 
				x.uid, x.logname, src,  x.cap, ip->name, user);
		} else {
			logit( "bad u%d <%.8s> <%s> %o %s<%c...>",
				x.uid, x.logname, src,  x.cap, ip->name, *user);
		}
	} else 
		logit( "dud %s,%s u%d g%d", 
			ip->name, ip->user, ip->uid, ip->gid);
	_exit(0);
}

xxx() {
	signal(SIGALRM, xxx);
	alarm(5);
}

cleanup() {
	logit("sigterm");
	exit(1);
}

cleanup2() {
	logit("death");
}

snarf() {
	int fd;
	static long opwfile;
	struct stat sb;


	fd = open("/etc/pwfile", 0);
	if(fd != -1) {
		fstat(fd, &sb);
		if(sb.st_mtime != opwfile) {
			logit( "snarf(pwfile)");
			pwslurp(fd);
			opwfile = sb.st_mtime;
		}
		close(fd);
	} else
		logit("bad snarf(pwfile)");

}

/*PRINTFLIKE1*/
logit(format, a, b, c, d, e, f, g, h, i, j, k)
char *format;
{
	char buf[200];
	long t;

	time(&t);
	sprint(buf, format,a,b,c,d,e,f,g,h,i,j,k);
	fprint(logfd, "%.12s %s\n", ctime(&t)+4, buf);
}
