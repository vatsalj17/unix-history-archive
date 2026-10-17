#include "priv.h"

char *privs = "/etc/privs";
char *mtpt = "/cs/priv";
char *lgfile = "/usr/adm/privlog";
char *server = "/etc/privserv";

int ppid;
int nflag;
int ttyfd = -1;

int death();
int reaper();
int timebomb();
int nz;

struct uenv uenv;
extern struct label labelno;

main(ac, av)
register char **av;
{
	int naplen = 4, i;

	if(fork()!=0)
		return 0;
	ppid = getpid();

	for(i=1; i<ac; i++) {
		if(strcmp(av[i], "-p")==0) {
			if(i==ac-1)
				bomb("bad -p option\n");
			server = av[++i];
		}
		else if(strcmp(av[i], "-m")==0) {
			if(i==ac-1)
				bomb("bad -m option\n");
			mtpt = av[++i];
		}
		else if(strcmp(av[i], "-l")==0) {
			if(i==ac-1)
				bomb("bad -l option\n");
			lgfile = av[++i];
		}
		else if(strcmp(av[i], "-f")==0) {
			if(i==ac-1)
				bomb("bad -f option\n");
			privs = av[++i];
		}
		else bomb("bad args\n");
	}

	signal(SIGTERM, death);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	mkdir("/tmp/blackhole");
	chdir("/tmp/blackhole");
	setflab("/tmp/blackhole", &labelno);

	loginit(lgfile);
	logpr("start %s %s %s\n", server, privs, mtpt);

	rootnode = mknode("/");

	(void)refresh(1);
	for(;;) {
		fd_set x;
		ipcinfo *ip;
		int fd;

		timebomb();
		fd = ipccreat(mtpt, "light");
		errno = defuse();
		if(fd < 0) {
			logpr("%s %s\n", errstr, mtpt);
			if(naplen<120) naplen += 3;
			sleep(naplen);
			continue;
		}
		naplen = 4;
		chmod(mtpt, 0666);
		logpr("mount %s\n", mtpt);
		ioctl(fd, FIOCLEX, 0);
		for(;;) {
			signal(SIGCHLD, reaper);
			exorcise();	/* any zombies? */
			if(refresh(0)) {
				close(fd);
				unlink(mtpt);
				sleep(4);
				exorcise();
				execv(server, av);
				logpr("death: %E cannot re-exec %s\n", server);
				exit(1);
			}

			nz = 0;
			FD_ZERO(x);
			FD_SET(fd,x);
			if(select(fd+1,&x,(fd_set*)0,1000*1000)<1)
				continue;
			timebomb();
			ip = ipclisten(fd);
			errno = defuse();
			if(ip == 0) {
				if(nz>0)continue;
				break;
			}
			serve(ip);
		}
		logpr("listen %E %s\n", errstr);
		close(fd);
		exorcise();
		sleep(1);
	}
}

serve(ip)
ipcinfo *ip;
{
	char *getarg();
	struct passfd passfd;
	char c;
	int n, fd, pid;

	fd = ipcaccept(ip);
	ioctl(fd, FIOCLEX, 0);

	if(fd == -1) {
		logpr("bad accept\n");
		return;
	}

	pid = fork();

	if(pid==-1) {
		close(fd);
		logpr("fork fail\n");
		return;
	} else if(pid>0) {
		birth(fd, pid);
		return;
	}
	signal(SIGCHLD, SIG_DFL);
	close(0);
	close(1);
	close(2);
	close(3);

	/*
	 * fd talks to client's agent
	 */
	for(;;) {
		if(read(fd, &c, 1) != 1) break;
		switch(c) {
		default:
			logpr("ill cmd '%c'\n", c);
			exit(1);
			break;
		/*
		 * client using -n
		 */
		case 'n':
			nflag++;
			break;
		/*
		 * get client's environment, incl. his tty stream
		 */
		case 't':
			(void)getfd(fd, 0, &passfd);
			(void)getfd(fd, 1, &passfd);
			(void)getfd(fd, 2, &passfd);
			ttyfd = getfd(fd, 3, &passfd);
			strncpy(uenv.logname, passfd.logname, 
				sizeof(passfd.logname));
			uenv.cap = passfd.cap;
			uenv.uid = passfd.uid;
			uenv.gid = passfd.gid;
			fgetflab(ttyfd, &uenv.plab);
			uenv.plab.lb_t = uenv.plab.lb_u = 0;
			uenv.plab.lb_fix = F_LOOSE;
			uenv.pceil = uenv.plab;
			ioctl(ttyfd, TIOCGPGRP, &uenv.pgrp);
			ioctl(ttyfd, FIOGSRC, uenv.ttysrc);
			ubufset();
			break;
		/*
		 * trivial request = just show privs
		 */
		case 's':
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
			setpgrp(0,uenv.pgrp);
			display(1);
			monotone(rootnode);
			exit(0);
			break;
		/*
		 * request with args
		 */
		case 'r':
			n = 0;
			uenv.av = (char**)0;
			for(uenv.ac=0;;uenv.ac++) {
				if(uenv.ac>=n) {
					n += n/2+ 20;
					uenv.av = (char**)realloc((char*)uenv.av,
						n*sizeof(char*));
				}
				if((uenv.av[uenv.ac] = getarg(fd))==0)
					break;
			}
			exit(request(uenv.ac, uenv.av));
			break;
		}
	}
	exit(0);
}

getfd(fd, k, pf)
struct passfd *pf;
{
	if(ioctl(fd, FIORCVFD, pf) == -1) return -1;
	if(pf->fd != k) {
		dup2(pf->fd, k);
		close(pf->fd);
	}
	return k;
}

timebomb()
{
	signal(SIGALRM, timebomb);
	alarm(5);
}

defuse()
{
	int x = errno;
	signal(SIGALRM, SIG_IGN);
	alarm(0);
	return x;
}

death()
{
	logpr("death\n");
	exit(1);
}

reaper()
{
	nz++;
}


/*
 * parse priv file; check to see if its changed.
 * allow generous sleep times to let edits finish:
 * the main thread is (a) in no hurry, and (b) can do
 * nothing legal & useful til privs file is quiescent
 */
refresh(flag)
{
	int rv = 0, fd;
	struct label plab, pceil;
	struct stat st;
	static long t = 0;

	fd = open(privs, 0);
	if(fd == -1) {
		logpr("death: bad open %s %E\n", privs);
		exit(1);
	}
	while(pex(fd,15,(struct pexclude*)0)!=0) {
		logpr("pex %s failed\n", privs);
		sleep(3);
	}
	if(fstat(fd, &st)==-1) {
		logpr("death: bad stat %s %E\n", privs);
		exit(1);
	}
	if(st.st_ctime == t) goto out;
	else if(flag==0) {
		rv = 1;
		goto out;
	}
	logpr("refresh %s\n", privs);
	t = st.st_ctime;
	parse(privs, bomb);
	if(!istrusted(privs)) {
		if(getplab(&plab, &pceil)!=0) {
			logpr("death: plab %E\n");
			exit(1);
		}
		if(plab.lb_t|plab.lb_u) {
			logpr("%s untrusted; drop lic.\n",privs);
			plab.lb_u = 0;
			setplab(&plab,&pceil);
		}
		loginit((char*)0);
	}
out:
	unpex(fd,0);
	close(fd);
	return rv;
}

/*
 * reap zombies
 */
exorcise()
{
	int pid;

	for(;;) {
		pid = wait3((int*)0,1,(struct vtimes*)0);
		if(pid>0)
			rip(pid);
		else
			break;
	}
	nz = 0;
}

int pidtab[128], fdtop = 0;

birth(fd, pid)
{
	if(fd>=128 || fd<0) {
		logpr("pid %d fd %d\n", pid, fd);
		return;
	}
	if(pidtab[fd]!=0) {
		logpr("pid %d already at fd %d\n", pidtab[fd], fd);
	}
	pidtab[fd] = pid;
	if(fd>fdtop)
		fdtop = fd;
}

rip(pid)
{
	int n, fd;

	n = 0;
	for(fd=0; fd<=fdtop; fd++) {
		if(pidtab[fd]==pid) {
			pidtab[fd] = 0;
			close(fd);
			logpr("rip %d\n", pid);
			n++;
		}
	}
	if(n!=1)logpr("pid %d had %d instances!\n", pid, n);
	while(fdtop>0 && pidtab[fdtop]==0) fdtop--;
}



/*
 * parse an arg word, produced at other end with
 * print("%d\n%s\n", strlen(s), s);
 *
 * arg list terminated with -1\n
 */
char *
getarg(fd)
{
	char *s, c;
	int state, n;

	n = 0;

	state = 0;
	for(;;) {
		if(read(fd,&c,1)!=1)break;
		if(state==0 && c=='-') {
			state=1;
			continue;
		}
		if(state==0 && isdigit(c)) state=2;
		if(state==0)continue;
		if(state==1 && c=='1') {
			state =3;
			continue;
		}
		if(state==2 && isdigit(c)) {
			n = 10*n + c-'0';
			continue;
		}
		if(state==2 && c=='\n')
			break;
		if(state==3 && c=='\n')
			return 0;
		return 0;
	}
	s = calloc(1,(unsigned)n+1);
	read(fd, s, n);
	return s;
}

