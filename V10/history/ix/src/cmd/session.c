#include <signal.h>
#include <libc.h>
#include <fio.h>
#include <sys/filio.h>
#include <sys/ttyio.h>
#include <sys/types.h>
#include <sys/label.h>
#include <sys/stat.h>
#include <sys/pex.h>

/*
 * Subsession at a different level
 * give user subshell at changed level
 * change & restore terminal label
 *	-x 		do not fork, do not restore term label
 *	-l lab		new label [default, bottom]
 *	-u user		use this username for authentication
 *	-c file a1... execute 'file' with indicated argv list
 */

char logfile[] = "/dev/log/sessionlog";
char pwfile[] = "/etc/pwfile";		/* name pw lic lab */
char *floor = "/etc/floor";
char *shv[2] = {"/bin/sh", 0};
char *noshv[2] = {"/etc/nosh", 0};

extern char **environ;
char *env[] = { (char *)0};	/* if going down use this */

int mypid;
char *tname;
char **origav;

extern struct label labelno;
extern struct label labelbot;
extern struct label labeltop;
struct label labelfloor;

struct label oldp, oldc;		/* initial process labels */
struct label newp, newc;		/* proposed process labels */
struct label authc;			/* authorized ceiling */
struct label oldt;			/* old label of terminal */
struct label newt;			/* new label of terminal */

char sname[SSRCSIZ+1];
char sname2[SSRCSIZ+1];
struct label *atolab();
char username[9];

struct label arglab();
struct label labMIN();
struct label labMAX();

char ustr[] = "usage: /bin/session [-x] [-l lab] [-u user] [-c file arg...]\n";

/*
 * Record current state
 * Decide what levels to ask for, collect authorization
 * Use AI to emend user's desires
 * Check if authorization is sufficient
 * Change labels, go.
 */
main(ac, av)
char **av;
{
	char **ap = 0;
	int nofork = 0, asked = 0, c;
	int easylab, easyceil;
	int retval;
	extern int optind;
	extern char *optarg;
	struct label temp1, temp2;

	origav = av;
	nochk(2,0);
/*
	if(av[0][0] != '/')
		goto err;
*/
	ioctl(0, FIOGSRC, sname);
	tname = ttyname(0); tname = tname?strdup(tname):"";
	mypid = getpid();
	getplab(&oldp, &oldc);
	getlogname(username);
	username[8] = 0;

	if(getflab(floor, &labelfloor) == -1) {
		fprint(2, "Can't find floor; assume floor=top\n");
		labelfloor = labeltop;
	}

	newp = oldp;
	newc = oldc;
	authc = oldc;

	fgetflab(0, &oldt);


	while((c = getopt(ac, av, "xl:u:C:c")) != -1)
		switch(c) {
		case 'x':	
			nofork++;	
			break;
		case 'l':
			newp = arglab();
			break;
		case 'C':
			newc = arglab();
			break;
		case 'u':
			auth(optarg, 1);
			asked++;
			break;
		case 'c':
			ap = &av[optind]; 
			goto setup;
		default:
			goto err;
		}	if(optind != ac) {
err:
		fprint(2, ustr);
		exit(1);
	}
setup:
	newp.lb_u = 0;
	temp1 = labMIN(&oldp, &labelfloor);
	temp2 = labMIN(&newp, &labelfloor);
	easylab = labLE(&temp1, &temp2);
	newc = labMAX(&newp, &newc);
	easyceil = labLE(&newc, &oldc);

	if(!easyceil || !easylab) {
		if(!asked)
			(void) auth(username, 0);
		newc = labMIN(&newc, &authc);
		if(!labLE(&newp, &newc))
			sorry("insuf ceil");
	}

	newt = newp; 
	newt.lb_t = 0;
	newt.lb_u = 0;
	/* newt.lb_fix = F_RIGID; */
	

	if(!labLE(&oldp, &newp)) environ = env;
	if(ap == 0)
		ap = labLE(&labelfloor, &newp)? shv : noshv;
	if(!ok(ap))
		sorry("no confirmation");
	mung(&newt);
	retval = run(nofork, ap);
	mung(&oldt);
	ioctl(0, FIOSSRC, sname);

	exit(retval);
}

mung(lp)
struct label *lp;
{
	struct label l;
	if(fsetflab(0, &labelno) == -1||fsetflab(0, lp) == -1) {
		perror("setflab term");
		exit(1);
	}
}

run(flag, av)
char **av;
{
	SIG_TYP fint, fquit;
	int pid;

	fint = signal(SIGINT, SIG_IGN);
	fquit = signal(SIGQUIT, SIG_IGN);

	if(flag) 
		pid = 0;
	else
		pid = fork();

	mypid = getpid();
	if(pid == -1) {
		perror("fork");
		return 1;
	}
	else if(pid == 0) {
		signal(SIGINT, fint);
		signal(SIGQUIT, fquit);
		if(setplab(&newp, &newc) == -1) {
			perror("setplab");
			sorry("setlab stdin");
		}
		logit("ok", av[0]);
		setuid(getuid());
		execv(av[0], av);
		perror(av[0]);
		logit("failed", av[0]);
		return 1;
	}
	while(wait((int*)0) != -1)
		;
	signal(SIGINT, fint);
	signal(SIGQUIT, fquit);
	return 0;
}

/*
 * Verify password listed for name (first valid line for 'name').
 * The password is bypassed if session was invoked with license
 * T_SETLIC, on the assumption that the license was granted by
 * priv(1).
 */
auth(name, flag)
char *name;
{
	struct label *lp, paranoid;
	char *p[4];
	register int fd;
	register char *t, *s;

	fd = open(pwfile, 0);
	if(fd == -1) {
		goto garble;
	}
	if(flag || (oldp.lb_u&T_SETLIC)==0) {
		switch(pwquery(0, name, "pex")) {
		case -1:
			goto garble;
		default:
		case 0:
			sorry("incorrect password");
		case 1:
			if(flag)
				setpwcache(name);
			break;
		}
	}
	
	if(fgetflab(fd, &paranoid) == -1
	    /* || paranoid.lb_flag != L_NO */
	    || (paranoid.lb_t == 0 && paranoid.lb_u == 0)) {
		goto garble;
	}

	setfields(":");
	Finit(fd, (char*)0);
	while(s=Frdline(fd)) {
		if(t = strchr(s, '#'))
			*t = 0;
		if(getfields(s, p, 5) == 5) {
			if(strcmp(name, p[0]) == 0) {
				lp = atolab(p[4]);
				if(lp == 0) 
					goto garble;
				authc = *lp;
				close(fd);
				return 1;
			}
		}
	}
	close(fd);
	return;
garble:
	sorry("system error in authentication\n");
}

ok(ap)
char **ap;
{
	int i;
	char *s, c;
	if(pex(0, 10, (struct pexclude *)0) != 0 
	    && pex(0, -1, (struct pexclude *)0) != 0)
		return 0;
	for(i=0; s=origav[i]; i++)
		Fprint(0, i?" %s":"%s", s);
	Fprint(0, " (EXEC");
	for(i=0; s= ap[i]; i++)
		Fprint(0, " %s", s);
	Fprint(0, ")? ");
	Fflush(0);

	i = 0;
	if(read(0, &c, 1) == 1 && c == 'y' || c == 'Y')
		i++;
	unpex(0, -1);
	return i;
}

setpwcache(name)
char *name;
{
	if(strcmp(name, username)!=0)
		return;
	if(strchr(sname, ':')!=0)
		return;
	if(strlen(sname)+strlen(name)+1 > SSRCSIZ)
		return;
	strcpy(sname2, sname);
	strcat(sname2, ":");
	strcat(sname2, name);
	if(ioctl(0, FIOSSRC, sname2) == -1)
		fprint(2, "session: warning FIOSSRC\n");
}

sorry(s)
char *s;
{
	fprint(2, "Sorry.\n");
	logit("denied", s);
	exit(1);
}


logit(s, t)
char *s, *t;
{
	long tim;
	int i;
	static int fd = -2;

	if(fd == -2) {
		fd = open(logfile, 1);
		if(fd != -1) {
			ioctl(fd, FIOCLEX, (struct sgttyb *)0);
			Finit(fd, (char*)0);
		}
	}
	if(fd < 0) return;
	time(&tim);

	Fprint(fd, "%.12s %d %s %s %s:%s %s", 
		ctime(&tim)+4, mypid, sname, tname, username, s, t);
	if(origav)
	for(i=0; s=origav[i]; i++)
		Fprint(fd, " <%s>", s);
	Fprint(fd, "\n");
	Fflush(fd);
}

struct label
arglab()
{
	struct label *lp = atolab(optarg); 
	if(lp == 0) {
		fprint(2, "malformed label %s\n", optarg);
		exit(1);
	}
	lp->lb_t = 0;
	lp->lb_u = 0;
	return *lp;
}
