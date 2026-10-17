#include "nd.h"

int logfd = -1;
int ttyfd = 1;
char *logf;
char *mtpt = "/cs/notary";
char *mtptreg;
char *notary = "/usr/notary";

main(ac,av)
char **av;
{
	int i, st = 0;
	
	pid = getpid();
	srand((int)time((long*)0)^pid);
	
	for(i=1; i<ac; i++){
		if(strcmp(av[i],"-m")==0) mtpt = av[++i];
		else if(strcmp(av[i],"-d")==0) notary = av[++i];
		else {
			fprint(2,"usage: notaryd [-m mtpt] [-d playpen]\n");
			exit(1);
		}
	}
	db = glue3(notary, "/", "notary");
	odb = glue3(notary, "/", "onotary");
	dbf = glue3(notary, "/", "notary.F");
	logf = glue3(notary, "/", "log");
	mtptreg = glue3(mtpt, "!", "register");

	chdir(notary);
	logfd = open(logf, 1);

	if(logfd == -1) {
		logit("quitting: cannot open %s", logf);
		exit(1);
	}

	if(ipcopen(mtpt,"light")!=-1) {
		logit("quitting: %s already mounted", mtpt);
		exit(1);
	}

	if((cbtfd = open(dbf, 0))==-1) {
		logit("quitting: cannot open %s", dbf);
		exit(1);
	}
	snarf(1);
	detach();
	logit("running");
	for(;;){
		register int fd;

		fd = ipccreat(mtpt,  "light");
		if(fd<0) {
			logit( "bad announce %s", errstr);
			if(st<600)
				st += 1+st/2;
			sleep(st);
		} else {
			st = 0;
			chmod(mtpt, 0666);
			logit( "good announce fd=%d",fd);
			listen(fd);
		}
	}
}

listen(fd)
int fd;
{
	fd_set rdfds;
	register int client;
	
	for(;;) {
		snarf(0);
		FD_ZERO(rdfds);
		FD_SET(fd, rdfds);
		while(wait3((int*)0,1,(char*)0)>0)
			;
		if(select(fd+1, &rdfds, (fd_set*)0, 1000*1000)<1)
			continue;
		arm(30,xxx);
		ip = ipclisten(fd);
		if(disarm()) 
			logit("listen timeout");
		if(ip == 0) {
			logit("listen errno=%d %s", errno, errstr);
			break;
		}
		if(*ip->machine) {
			logit("off-machine");
			ipcreject(ip, EPERM, "xenophobia");
			continue;
		}
		arm(30,xxx);
		client = ipcaccept(ip);
		if(disarm())
			logit("accept timeout");
		if(client != -1) {
			serve(client);
			close(client);
		} else
			logit("bad accept %s", errstr);
		ip = 0;
	}
	close(fd);
}

detach()
{
	register int i;

	for(i=0; i<4; i++) close(i);
	ttyfd = -1;
	if(fork()!=0)
		exit(0);
	pid = getpid();
	setpgrp(0,pid);
}


char *
glue3(a,b,c)
char *a, *b, *c;
{
	char *s;
	s = malloc(strlen(a)+strlen(b)+strlen(c)+1);
	strcpy(s,a);
	strcat(s,b);
	strcat(s,c);
	return s;
}

/*PRINTFLIKE1*/
logit(format, a, b, c, d, e, f, g, h, i, j, k)
char *format;
{
	int incommunicado;
	char buf[200];
	long t;

	time(&t);
	sprint(buf, format,a,b,c,d,e,f,g,h,i,j,k);
	incommunicado = 1;
	if(logfd != -1) {
		incommunicado = 0;
		lseek(logfd,0L,2);
		lp2(logfd, &t, buf);
		lseek(logfd,0L,2);
	}
	if(ttyfd != -1) {
		incommunicado = 0;
		lp2(ttyfd, &t, buf);
	}
	if(incommunicado)
		exit(0);
}

lp2(fd, tp, buf)
long *tp;
char *buf;
{
	if(ip == 0)
		fprint(fd, "%.12s %d %s\n", 
		ctime(tp)+4, pid, buf);
	else
		fprint(fd, "%.12s %d %s %s!%s %s\n", 
			ctime(tp)+4, pid, 
			ip->name, ip->machine, ip->user, buf);
}

int tick;
yyy()
{
	arm(0,SIG_IGN);
	longjmp(env,1);
}
xxx()
{
	arm(0,SIG_IGN);
	tick++;
}
disarm()
{
	int x = tick;
	arm(0,SIG_IGN);
	return x;
}
arm(n,f)
int (*f)();
{
	tick = 0;
	signal(SIGALRM, f);
	alarm(n);
}

