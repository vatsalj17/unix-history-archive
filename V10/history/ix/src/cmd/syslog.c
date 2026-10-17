#include <errno.h>
#include <ctype.h>
#include <libc.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/label.h>
#define KERNEL
#include <sys/log.h>

extern char *labtoa();
extern char *getstsrc();
char **gav;
extern struct label labelno, labeltop;
struct stat sb;
char lbuf[1024];
int logfd;

/* requires T_LOG to exercise syslog(2) */
/* requires T_NOCHK for diagnostic fstat */

main(ac,av)
char **av;
{
	char *pmask();
	int x;
	
	nochk(1,0);
	nochk(2,0);
	logset(ac, av);
	
	if(ac<2 || ac >4)error("usage: syslog arg [arg]", "");
	gav = av;
	if(strcmp(av[1], "on")==0) {
		(void)xx(LOGON,  "Fm", av+2);
	}
	else if(strcmp(av[1], "off")==0) {
		(void)xx(LOGOFF,  "0m", av+2);
	}
	else if(strcmp(av[1], "get")==0) {
		x = xx(LOGGET, "5", av+2);
		print("%s\n", pmask(x));
	}
	else if(strcmp(av[1], "set")==0) {
		(void)xx(LOGSET,  "5k", av+2);
	}
	else if(strcmp(av[1], "fget")==0) {
		x = xx(LOGFGET,  "f", av+2);
		print("%d\n", x);
	}
	else if(strcmp(av[1], "fset")==0) {
		(void)xx(LOGFSET,  "f4", av+2);
	}
	else if(strcmp(av[1], "pget")==0) {
		x = xx(LOGPGET,  "p", av+2);
		print("%s\n", pmask(x));
	}
	else if(strcmp(av[1], "pset")==0) {
		(void)xx(LOGPSET,  "pk", av+2);
	}
	else {
		error("unknown arg %s", av[1]);
	}
	exit(0);
}

char lbuf2[1024];
error(f, a)
char *f, *a;
{
	sprint(lbuf2, f, a);
	fprint(2,"%s\n", lbuf2);
	fprint(logfd, "ERROR %s %s\n", lbuf, lbuf2);
	exit(1);
}

#define f(x) (x<ac)?av[x]:""

logset(ac, av)
char **av;
{
	int uid, euid;
	char lname[10];
	char tname[100];
	char sname[100];
	struct label plab, clab;
	char *realuid();
	long t;

	logfd = open("/dev/log/log00", 1);
	getplab(&plab, &clab);
	getlogname(lname);
	strncpy(tname, ttyname(0), sizeof(tname));
	strncpy(sname, getstsrc(0), sizeof(sname));
	uid = getuid();
	euid = geteuid();
	t = time((long*)0);

	sprint(lbuf, "syslog(%s,%s,%s,%s) %.20s %s %d%s '%s' '%s'", 
		f(0), f(1), f(2), f(3),
		4+ctime(&t), lname, uid, realuid(uid, euid), tname, sname);
	labcat(lbuf, " p=", &plab);
	labcat(lbuf, " c=", &clab);
}

callsyslog(a1, a2, a3)
{
	int rv;

	rv = syslog(a1, a2, a3);
	if(rv == -1) 
		fprint(logfd, "ERR=%d %s\n", errno, lbuf);
	else
		fprint(logfd, "%s\n", lbuf);
	return rv;
}

char *
realuid(uid, euid)
{
	static char buf[10];
	if(uid == euid) return "";
	sprint(buf,"[euid=%d]", euid); 
	return buf;
}

labcat(s, t, p)
char *s, *t;
struct label *p;
{
	strcat(s, t);
	t = labtoa(p);
	if(p->lb_u == 0 && p->lb_t == 0) {
		t += 14;
		if(p->lb_fix == F_LOOSE) {
			t++;
			if(p->lb_flag == L_BITS) t += 2;
		}
	}
	strcat(s, t);
}


/*
 * format codes:
 *	F file name -> file descriptor open for writing
 *	f file name -> file descriptor open for reading
 *	m logging device name, or minor dev number.
 *	5 poison mask index: 0...4
 *	4 poison level: 0...3
 *	k mask: combo of letters
 *	p pid
 */

xx(code, format, xv)
char *format;
char **xv;
{
	int x, args[2], *ap;
	struct label lab;

	ap = args;
	ap[0] = ap[1] = 0;
	for(;*format;format++) {
		if(*xv == 0)
			error("missing argument", "");
		switch(*format) {
		case 'F':
			if(**xv != '/')
				error("file name %s not absolute", *xv);
			x = open(*xv, 1);
			if(x == -1) {
				perror(*xv);
				exit(1);
			}
			if(fgetflab(x, &lab) == -1)
				fprint(2,"can't get label of %s\n", *xv);
			else if(!labEQ(&lab,&labelno) && !labEQ(&lab,&labeltop))
				fprint(2, "covert channel warning: %s\n", *xv);
			*ap = x;
			break;
		case 'f':
			if(**xv != '/')
				error("file name %s not absolute", *xv);
			x = open(*xv, 0);
			if(x == -1) {
				perror(*xv);
				exit(1);
			}
			*ap = x;
			break;
		case 'm':
			if(isdigit(**xv))
				x = atoi(*xv);
			else if(**xv != '/')
				error("file name %s not absolute", *xv);
			else if(stat(*xv, &sb)==-1) {
				perror(*xv);
				exit(1);
			} else
				x = minor(sb.st_rdev);
			*ap = x;
			break;
		case '5':
		case '4':
			*ap = atoi(*xv);
			if(*ap<0 || *ap>= *format-'0')
				error("arg %s out of range", *xv);
			break;
		case 'k':
			*ap = parsemask(*xv);
			break;
		case 'p':
			*ap = atoi(*xv);
			break;
		case '0':
			*ap = 0;
			xv--;
			break;
		default:
			*ap = 0;
		}
		xv++;
		ap++;
	}
	if(*xv)
		error("extraneous arg %s", *xv);
	return callsyslog(code, args[0], args[1]);
}
struct tab {
	char lc, uc;
	int val;
} tab[] = {
	{'n','N', LN}, {'s','S', LS}, {'u','U', LU},
	{'i','I', LI}, {'d','D', LD}, {'p','P', LP},
	{'l','L', LL}, {'a','A', LA}, {'x','X', LX},
	{'e','E', LE}, {'t','T', LT}, 0
};
parsemask(s)
char *s;
{
	struct tab *t;
	int x;
	for(x=0;*s;s++) {
		for(t=tab;t->lc;t++)
			if(t->lc == *s || t->uc == *s) {
				x |= t->val; 
				break;
			}
		if(t->lc == 0) error("unknown log mask bit '%.1s'", s);
	}
	return x;
}
char *
pmask(x) 
{
	struct tab *t;
	static char mbuf[33];
	char *p = mbuf;
	if(x==0)return "-";
	for(t=tab;t->lc; t++)
		if( x & t->val) *p++ = t->uc;
	*p = 0;
	return mbuf;
}
