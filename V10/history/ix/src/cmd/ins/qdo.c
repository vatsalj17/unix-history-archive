#include "ins.h"

int copy(), remove(), owner(), mode(), lab(), priv(), mysync();
struct command {
	char code;
	int n;
	int (*f)();
} ctab[] = {
	{'c', 3, copy},
	{'r', 1, remove},
	{'o', 3, owner},
	{'m', 2, mode},
	{'l', 2, lab},
	{'p', 3, priv},
	{'y', 1, mysync},
	{0}
};

int err;
char jtarg[300];

qdo(qfile, csum)
char *qfile, *csum;
{
	struct command *p;
	char *lp, *yp, *av[12];
	int fd;
	
	if(qfile == 0) return 0;
	fd = open(qfile, 0);
	if(fd == -1) {
		perror(qfile);
		return -1;
	}
	ioctl(fd, FIOCLEX, (void*)0);
	yp = xs(qfile, 0, 0);
	if(strcmp(csum, yp)!=0) {
		log("!sum", "%s %s should be %s\n",
			qfile, yp, csum);
		print("job file %s wrong checksum\n", qfile);
		close(fd);
		return -1;
	}

	err = 0;
	jtarg[0] = 0;
	while(lp = Frdline(fd)) {
		for(p=ctab;p->code;p++)
			if(*lp == p->code) {
				if(p->n!=getmfields(lp, av, p->n+1)-1)
					break;
				log("qdo", "%s %s %s %s\n"+3*(3-p->n),
					av[0], av[1], av[2], av[3]);
				errno=0;
				if(err==0 && nflag==0)
					(*(p->f))(av+1);
				break;
			}
	}
	close(fd);
	if(jtarg[0])
		xs(jtarg, 1, 1);
	return err?-1:0;
}



copy(av)
char **av;
{
	char *xp;
	struct stat x;
	int n, m, fin, fout;

	fin = open(av[0], 0);
	if(fin == -1 || fstat(fin, &x) == -1) {
		log("!copy", "nofrom %E %s %s\n", errno, av[0], av[1]);
		perror(av[0]);
		err++;
		return;
	}
	strcpy(jtarg, av[1]);
	fout = creat(av[1], (int)x.st_mode);
	if(fout == -1) {
		log("!copy", "noto %E %s %s\n", av[0], av[1]);
		perror(av[1]);
		err++;
		close(fin);
		return;
	}

	while((n=read(fin,cpbuf,sizeof(cpbuf)))>0) {
		m = write(fout,cpbuf,n);
		if(m == -1) {
			log("!copy", "write %E %s %s\n", av[0], av[1]);
			perror(av[1]);
			err++;
			close(fin);
			close(fout);
			return;
		}
	}
	if(n == -1) {
		log("!copy", "read %E %s %s\n", av[0], av[1]);
		perror(av[0]);
		err++;
		close(fin);
		close(fout);
		return;
	}
	close(fin);
	close(fout);
	xp = xs(av[1], 0, 1);
	if(strcmp(av[2],xp)!=0) {
		log("!copy", "xsbad %s %s is %s should be %s\n", 
			av[0], av[1], xp, av[2]);
		fprint(2, "target %s copied with wrong checksum %s %s\n", 
			av[1], av[2], xp);
		err++;
		return;
	}

	unlink(av[0]);
	log("copy", "%s %s %s\n", av[0], av[1], xp);
}


remove(av)
char **av;
{
	strcpy(jtarg, av[0]);
	if(unlink(av[0]) == -1) {
		log("!rm", "%E %s %d\n", av[0], errno);
		perror(av[0]);
		err++;
	}
	log("rm", "%s\n", av[0]);
}


int uid, gid;

owner(av)
char **av;
{
	struct stat x;

	strcpy(jtarg, av[0]);
	stat(av[0], &x);
	uid = x.st_uid;
	gid = x.st_gid;
	
	uidgid(av[1], av[2]);

	if(chown(av[0], uid, gid) == -1) {
		log("!chown", "%E %s %d %d %d\n", av[0], uid, gid, errno);
		perror(av[1]);
		err++;
	}
	log("chown", "%s %d %d\n", av[0], uid, gid);
}

uidgid(u, g)
char *u, *g;
{
	struct passwd *pwd;
	struct group *grp;
	char *strchr();
	struct passwd *getpwnam();
	struct group *getgrnam();

	if (*u == '\0')
		;
	else if (isnumber(u))
		uid = atoi(u);
	else if ((pwd = getpwnam(u)) != 0)
		uid = pwd->pw_uid;
	else {
		error( "unknown user id: %s", u);
	}
	if (g == 0 || *g == '\0')
		;
	else if (isnumber(g))
		gid = atoi(g);
	else if ((grp = getgrnam(g)) != 0)
		gid = grp->gr_gid;
	else {
		error( "unknown group id: %s", g);
	}
	gid &= 0xffff;
	uid &= 0xffff;
}

isnumber(s)
char *s;
{
	register c;

	if(*s == '-') s++;
	while(c = *s++)
		if(!isdigit(c))
			return(0);
	return(1);
}



mode(av)
char **av;
{
	int m;
	strcpy(jtarg, av[1]);
	sscanf(av[0], "%o", &m);
	if(chmod(av[1], m)== -1) {
		log("!chmod", "%E %s %s\n", av[0], av[1]);
		perror(av[1]);
		err++;
	}
	log("chmod", "%s %s\n", av[0], av[1]);
}


lab(av)
char **av;
{
	struct label *atolab(), *lp;

	lp = atolab(av[1]);
	if(lp==0){
		log("!setlab1", "%s %s\n", av[0], av[1]);
		err++;
		fprint(2,"illegal label '%s'\n", av[1]);
		return;
	}
	strcpy(jtarg, av[0]);
	if(setflab(av[0], lp) == -1) {
		log("!setlab2", "%E %s %s\n", av[0], av[1]);
		perror(av[0]);
		err++;
	}
	log("setlab", "%s %s\n", av[0], av[1]);
}


priv(av)
char **av;
{
	char **xp, *xav[10];
	int pid, status;
	struct label x;
	unsigned short is_t, is_u, want_t, want_u;

	if(getflab(av[0], &x)==-1) {
		log("!setpriv1", "%E %s %s %s\n", av[0], av[1], av[2]);
		perror(av[0]);
		err++;
		return;
	}
	strcpy(jtarg, av[0]);
#ifdef lint
	/* cyntax doesnt like bitfields */
	is_t = 0;
	is_u = 0;
#else
	is_t = x.lb_t;
	is_u = x.lb_u;
#endif
	want_t = (unsigned short)atopriv(av[1]);
	want_u = (unsigned short)atopriv(av[2]);
	if(is_t==want_t && is_u == want_u)
		return;
	if((pid = fork())==0) {
		xp = xav;
		*xp++ = "/etc/priv";
		*xp++ = "/etc/setpriv";
		*xp++ = av[1];
		*xp++ = av[2];
		*xp++ = av[0];
		*xp = 0;
		execv(xav[0], xav);
		execv(xav[1], xav+1);
		log("!setpriv2", "%E %s %s %s\n", av[0], av[1], av[2]);
		exit(1);
	}
	else if(pid>0) {
		if(wait(&status)!=pid) {
			error("stray wait","");
			err++;
		}
		else if(status != 0) {
			log("!setpriv3", "%d %s %s %s\n", 
				status, av[0], av[1], av[2]);
			err++;
		}
	} else {
		error("fork","");
		err++;
	}
	log("setpriv", "%s %s %s\n", av[0], av[1], av[2]);
}


mysync(av)
char **av;
{
	if(access(av[0], 0) == 0) {
		log("defer", "%s\n", av[0]);
		err++;
	}
}


