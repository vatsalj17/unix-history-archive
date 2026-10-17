#
/*
 * login [ name ]
 */

struct {
	char	name[8];
	char	tty;
	char	ifill;
	int	time[2];
	int	ufill;
} utmp;

struct {
	int	speeds;
	char	erase, kill;
	int	tflags;
} ttyb;

struct {
	int	junk[5];
	int	size;
	int	more[8];
	long int acc;
	long int mod;
} statb;

struct {
	char	logname[8],
		logdir[22],
		logtty;
} u;

char	*ttyx;

#define	ECHO	010

main(argc, argv)
char **argv;
{
	char pbuf[128];
	char	nsh[64], cbu;
	extern g();
	register char *namep, *np, *bp;
	char *cp;
	char pwbuf[9], *lnamep;
	int t, sflags, f, c, uid, gid, badname;
	int sflg, iflg, dflg;
	long int ml;

	signal(3, 1);
	signal(2, 1);
	nice(0);
	ttyx = "/dev/ttyx";
	if (((utmp.tty=ttyn(0)) == 'x') || (utmp.tty=='\0')) {
		write(1, "Sorry.\n", 7);
		exit();
	}
	ttyx[8] = u.logtty  = utmp.tty;
    loop:
	badname = 0;
	namep = utmp.name;
	lnamep = u.logname;
	if (argc>1) {
		np = argv[1];
		while (namep<utmp.name+8 && *np)
			if(*np==' ')
				break;
			else
				*namep++  = *lnamep++ = *np++;
		argc = 0;
	} else {
		write(1, "Name: ", 7);
		signal(14,g);
		alarm(300);
		while ((c = getchar()) != '\n') {
			if (c <= 0)
				 exit();
			if (namep < utmp.name+8)
				if(c==' ')
					break;
				else
					*namep++ = *lnamep++ = c;
		}
		alarm(0);
	}
	dflg = 0;
	if (lnamep[-1] == '/') {
		--lnamep;
		--namep;
		dflg++;
	}
	*lnamep = '\0';
	while (namep < utmp.name+8)
		*namep++ = ' ';
	if (getpwentry(utmp.name, pbuf)) {
		badname++;
		goto askpw;
	}
	np = colon(pbuf);
	if (*np!=':') {
	   askpw:
		gtty(0, &ttyb);
		sflags = ttyb.tflags;
		ttyb.tflags =& ~ ECHO;
		stty(0, &ttyb);
		write(1, "Password: ", 10);
		namep = pwbuf;
		signal(14,g);
		alarm(300);
		while ((c=getchar()) != '\n') {
			if (c <= 0)
				exit();
			if (namep<pwbuf+8)
				*namep++ = c;
		}
		alarm(0);
		*namep++ = '\0';
		ttyb.tflags = sflags;
		stty(0, &ttyb);
		write(1, "\n", 1);
		if(badname)
			goto bad;
		namep = crypt(pwbuf);
		while (*namep++ == *np++);
		if (*--namep!='\0' || *--np!=':')
			goto bad;
	}
	np = colon(np);
	uid = 0;
	while (*np != ':')
		uid = uid*10 + *np++ - '0';
	np++;
	np = colon(np);
	namep = np;
	np = colon(np);
	if (chdir(namep)<0) {
		write(1, "No directory\n", 13);
		goto loop;
	}
	{
		register char *cp1 = namep, *cp2 = u.logdir;

		while(*cp1)
			*cp2++ = *cp1++;
	}
	logpost(&u);
	time(utmp.time);
	if ((f = open("/etc/utmp", 1)) >= 0) {
		t = utmp.tty;
		seek(f, (t-'0')*16, 0);
		write(f, &utmp, 16);
		close(f);
	}
	if ((f = open("/usr/adm/wtmp", 1)) >= 0) {
		seek(f, 0, 2);
		write(f, &utmp, 16);
		close(f);
	}
	sflg = iflg = 0;
	if (stat(".mail",&statb) >= 0) {
		ml = statb.acc;
		sflg = statb.size;
		if (dflg) {
			if ((dflg=open(".mail",0)) >= 0) {
				read(dflg,cbu,1);
				close(dflg);
			}
			if (stat("/etc/motd",&statb) >= 0)
				if (ml >= statb.mod)
					iflg++;
		}
	}
	if (!iflg  && ((f = open("/etc/motd", 0)) >= 0)) {
		while((t=read(f,pwbuf,9)) > 0)
			write(1,pwbuf,t);
		close(f);
	}
	if (sflg)
		write(1, "You have mail.\n", 15);
	chown(ttyx, uid);
	setgid(1);
	setuid(uid);
	if (*np == '\0')
		execl("/bin/sh","-",0);
	else {
	/*	get name to call it  */
		bp = cp = np;
		while (*bp != '\0')
			if (*bp++ == '/') cp = bp;
		while (*--bp == '/') *bp = '\0';
		bp = nsh;
		*bp++ = '-';
		while (*bp++ = *cp++);
		execl(np,nsh,0);
		execl("/bin/sh","sh",np,0);
	}
	write(1, "No shell.\n", 9);
	exit();
bad:
	write(1, "Login incorrect.\n", 17);
	goto loop;
}

getpwentry(name, buf)
char *name, *buf;
{
	extern fin;
	int fi, r, c;
	register char *gnp, *rnp;

	fi = fin;
	r = 1;
	if((fin = open("/etc/passwd", 0)) < 0)
		goto ret;
loop:
	gnp = name;
	rnp = buf;
	while((c=getchar()) != '\n') {
		if(c <= 0)
			goto ret;
		*rnp++ = c;
	}
	*rnp++ = '\0';
	rnp = buf;
	while (*gnp++ == *rnp++);
	if ((*--gnp!=' ' && gnp<name+8) || *--rnp!=':')
		goto loop;
	r = 0;
ret:
	close(fin);
	fin = 0;
	(&fin)[1] = 0;
	(&fin)[2] = 0;
	return(r);
}

colon(p)
char *p;
{
	register char *rp;

	rp = p;
	while (*rp != ':') {
		if (*rp++ == '\0') {
			write(1, "Bad /etc/passwd\n", 16);
			exit();
		}
	}
	*rp++ = '\0';
	return(rp);
}
g() {}
