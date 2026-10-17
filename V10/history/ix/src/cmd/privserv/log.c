#include "priv.h"

extern int ppid;
extern int nflag;

extern int lineno;	/* from gram.y */
extern char *fn;

static char ubuf[1024];
static int lfd = -1;

int Vconv();
int Econv();

loginit(file)
char *file;
{
	if(file==0) {
		close(lfd);
		lfd = -1;
	} else {
		lfd = open(file, 1);
		ioctl(lfd,FIOCLEX,0);
		fmtinstall('V', Vconv);
		fmtinstall('E', Econv);
		if(ubuf[0]==0)sprint(ubuf, "%d ", ppid);
	}
}


/*
 * write on log file;
 * guarantee a single record
 */

/*PRINTFLIKE1*/
logpr(fmt, a)
char *fmt;
{
	long t;
	char *donprint(), logbuf[4096], *lp;

	if(nflag||lfd==-1)
		return;
	lp = logbuf;
	time(&t);
	lp += sprint(lp,"%16.15s %s", 4+ctime(&t), ubuf);
	lp = donprint(lp,logbuf+4096,fmt,&a);
	lseek(lfd, 0L, 2);			/* superfluous */
	write(lfd, logbuf, lp-logbuf);
}

/*
 * %V format item: print an argv array
 */
Vconv(pav, f1, f2, f3)
char ***pav;
{
	char **av = *pav, *s;

	while(s = *av++) {
		strconv(" ", 0, -1);
		strconv(s, 0, -1);
	}
	return sizeof(char**);
}
/*
 * %E: print errno
 */
Econv(pav, f1, f2, f3)
char ***pav;
{
	char buf[10];

	sprint(buf,"err=%d", errno);
	strconv(buf, 0, -1);
	return 0;
}



ubufset()
{
	sprint(ubuf, "%d.%d %s(%d,%d,%s) ",
		ppid, getpid(), uenv.logname, uenv.uid, uenv.gid, uenv.ttysrc);
}

/*
 * write on log file, and
 * report to user, and
 * die
 */
	/*PRINTFLIKE1*/
void
bomb(s, t, u, v)
register char *s, *t, *u, *v;
{
	logpr(s, t, u, v);
	error(s, t, u, v);
}

/*
 * report to user, and
 * die
 */
	/*PRINTFLIKE1*/
void
error(s, t, u, v)
register char *s, *t, *u, *v;
{
	if(lineno>0)
		fprint(2,"error, file %s, line %d: ",
			fn, lineno);
	else if(fn!=0)
		fprint(2,"error, file %s: ", fn);
	else
		fprint(2,"error: ");
	fprint(2,s,t,u,v);
	exit(1);
}
