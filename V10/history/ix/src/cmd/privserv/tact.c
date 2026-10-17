#include "priv.h"

extern ioctl();
extern setpgrp();
extern setruid();
extern setgid();
extern setlogname();
extern setuid();
extern setplab();

/* tests and actions for priv */


/* x = formula-in-privs-file, y=submitted-instance-av */

reqmatch(f,y)
Func *f;
register char **y;
{
	struct dfa *x;
	int i;

	for(i=0; i<f->ac; i++) {
		if((x=f->rav[i]) == 0)
			x = f->rav[i] = egrepcomp(f->av[i]);
		if(*y == 0 && x == dfanil) return 1;
		else if(*y==0 && dfamatch(x, "") == 0) return 0;
		else if(*y!=0 && dfamatch(x, *y++) == 0) return 0;
	}
	return 1;
}
sameargs(f,g)
Func *f, *g;
{
	int i;
	if(f->ac != g->ac)
		return 0;
	for(i=0; i<f->ac; i++)
		if(strcmp(f->av[i],g->av[i]) != 0)
			return 0;
	return 1;
}
exerciser(tn)
register Node *tn;
{
	register Qlst *ql;

	if(tn->may=='y')
		return 1;
	else if(tn->may=='n')
		return 0;
	else
	for(ql=tn->qlist;ql;ql=ql->nextq)
		if(exer(ql->qchain)) {
			tn->may = 'y';
			return 1;
		}
	tn->may = 'n';
	return 0;
}
exer(q)
register Func *q;
{
	if(q==0)
		return 0;
	for(;q;q=q->next) {
		switch(q->type) {
		case F_PRED|F_ID:
			if(checkid(q)) continue;
			return 0;
		case F_PRED|F_SRC:
			if(checksrc(q)) continue;
			return 0;
		case F_PRED|F_PW:
			if(checkpw(q)) continue;
			return 0;
		default:
			bomb("internal error 8: %s\n", q->name);
		}
	}
	return 1;
}

checkid(q)
register Func *q;
{
	if(q->val.re==0)
		q->val.re = egrepcomp(q->av[0]);
	if(q->val.re==0)
		bomb("internal error 6: %s\n",q->av[0]);
	return(dfamatch(q->val.re, uenv.logname));
}

checkpw(q)
register Func *q;
{
	register int i;
	for(i=0; q->av[i]; i++) {
		if(pwquery(ttyfd, q->av[i], "pex")>0)
			return 1;
	}
	logpr("bad pw\n");
	return 0;
}

checksrc(q)
register Func *q;
{
	if(q->val.re==0)
		q->val.re = egrepcomp(q->av[0]);
	if(dfamatch(q->val.re, uenv.ttysrc)) return 1;
	logpr("bad src\n");
	return 0;
}

#define BUFSIZ 4096

extern char *xv[], **xvp, **xvtop;
#define XPUSH(w) if(xvp<xvtop)*xvp++=w;else error("xvp overflow\n")
#define CPUSH(c) if(cp<&buf[BUFSIZ-1])*cp++=c;else error("buf overflow\n")
#define CPUSH0	*cp = 0

dset(q)
register Func *q;
{
	if(q->dav == 0)
		q->dav = dollar(q->av);
}

char **
dollar(av)
char **av;
{
	int x, c, n, i;
	char *cp, buf[BUFSIZ];
	char **dav, *s, *t;


	cp = buf;
	xvp = xv;

	while(s = *av++) {
		x = *s ? 0 : 1;
		while(c = *s++) {
			if(c!='$' || !isdigit(*s)) {
				CPUSH(c);
				x = 1;
				continue;
			}
			for(n=0; isdigit(c = *s); s++)
				n = n*10 + c - '0';
			if(n<uenv.ac) {
				for(t=uenv.av[n];c = *t;t++) {
					CPUSH(c);
				}
				x = 1;
			}
			if(*s == '*') {
				if(*++s)
					error("junk after *\n");
				if(n<uenv.ac) {
					x = 0;
					CPUSH0;
					XPUSH(strdup(buf));
					cp = buf;
					while(++n < uenv.ac) {
						XPUSH(uenv.av[n]);
					}
				}
			}
		}
		if(x) {
			CPUSH0;
			XPUSH(strdup(buf));
			cp = buf;
		}
	}
	XPUSH(0);
	n = xvp-xv;
	dav = (char**)calloc((unsigned)n+1, sizeof(char*));
	for(i=0; i<n; i++) {
		dav[i] = xv[i];
	}
	return dav;
}


execute(x)
register Need *x;
{
	register Func *q;
	register int result = 0;
	int oksrc = 0;
	for(q=x->payback; q; q=q->next) {
		dset(q);
		switch(q->type) {
		case F_ACT|F_CEIL:
			setceil(q);
			continue;
		case F_ACT|F_PRIV:
			setpriv(q);
			continue;
		case F_ACT|F_EXEC:
			oksrc = vetsrc(oksrc);
			result = command(q, -(q->next==0));
			continue;
		case F_ACT|F_DAEM:
			oksrc = vetsrc(oksrc);
			result = command(q, 1);
			continue;
		case F_ACT|F_EDIT:
			oksrc = vetsrc(oksrc);
			result = doedit(q);
			continue;
		case F_ACT|F_ANYS:
			oksrc = 1;
			continue;
		default:
			bomb("internal error 7: %s\n",q->name);
		}
	}
	return result;
}

setpriv(q)
register Func *q;
{
	int priv;
	priv = atopriv(q->dav[0]);
	uenv.plab.lb_u = q->val.priv = priv;
	if(priv < 0)
		error("improper priv spec %s\n", q->dav[0]);
}

setceil(q)
register Func *q;
{
	register struct label *lp;
	if(q->val.lp == 0)
		lp = q->val.lp = myatolab(q->dav[0]);
	if(lp == 0 || lp->lb_flag!=L_BITS)
		error("improper ceiling label %s\n",q->dav[0]);
	uenv.pceil = *lp;
	uenv.pceil.lb_t = uenv.pceil.lb_u = 0;
}

char *emptyenv[] = { 0 };
/* deamon = -1 no fork; 0 fork and wait; 1 fork no wait */
command(q, daemon)
register Func *q;
{
	int i, status;

	if(q->ac == 0)
		return 0;
	if(nflag) {
		struct label p, c;
		getplab(&p, &c);
		Fprint(2,"proclab(%s", labtoa(&p));
		Fprint(2,", %s)\n", labtoa(&c));
		Fprint(2,"setplab(%s", labtoa(&uenv.plab));
		Fprint(2,", %s)\n", labtoa(&uenv.pceil));
		Fprint(2,"exec(%s", q->dav[0]);
		for(i=1;q->dav[i];i++)Fprint(2," %s", q->dav[i]);
		Fprint(2,")\n");
		Fflush(2);
		return 0;
	}
	if(daemon >= 0) {
		switch(fork()) {
		case 0:
			break;
		case -1:
			logpr("bad fork %E\n");
			perror("can't fork");
			exit(1);
		default:
			if(daemon)
				return 0;
			wait(&status);
			return status;
		}
	}
	if(
	     /*
	      * 2 ioctl's for /bin/ps's sake
	      */
	     callsys(ioctl, "spgrp 0", ttyfd, TIOCSPGRP, 0) || 
	     callsys(ioctl, "spgrp g", ttyfd, TIOCSPGRP, &uenv.pgrp) || 
	     callsys(setpgrp, "spgrp", 0,uenv.pgrp) || 
	     
	     callsys(setruid, "ruid", uenv.uid) || 
	     callsys(setgid, "gid", uenv.gid) || 
	     callsys(setlogname, "lname", uenv.logname) || 
	     callsys(setuid, "uid", uenv.uid) || 
	     callsys(setplab, "plab", &uenv.plab, &uenv.pceil)
	)
	      return 1;
	
	/* sigs part of env? */
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	execve(q->dav[0], q->dav, emptyenv);
	logpr("bad exec %E %s\n", q->dav[0]);
	perror(q->dav[0]);
	exit(1);
	return 0;
}

callsys(f,name,a1,a2,a3)
int (*f)();
char *name;
{
	if((*f)(a1,a2,a3)==0) return 0;
	logpr("callsys %s %E\n", name);
	perror(name);
	return 1;
}

doedit(q)
register Func *q;
{
	if(q->ac != 2)
		error("privedit arg count\n");
	editnode = q->dav[0];
	parse(q->dav[1],error);
	monotone(rootnode);
	rewrite();
	return 0;
}

monotone(np)
register Node *np;
{
	register Node *xp;

	if(np==0)return;
	for(xp=np->child;xp;xp=xp->sib) {
		if(!above(np->rchain,xp->rchain))
			error("%s not monotone\n",xp->nodename);
		monotone(xp);
	}
}

istrusted(s)
char *s;
{
	struct label flab;
	if(getflab(s,&flab) == -1) {
		logpr("istrusted: %s %E\n",s);
		exit(1);
	}
	return flab.lb_t!=0 || flab.lb_u!=0;
}

vetsrc(oksrc)
{
	char yes[2];
	Func f;

	if(oksrc || nflag)
		return 1;

	if(pexinput() == -1) {
		unpex(ttyfd, -1);
		logpr("bad pex\n");
		error("can't establish trusted path\n");
	}
	f.name = "priv";
	f.ac = uenv.ac;
	f.av = uenv.av;
	onefunpr(ttyfd, &f);
	Fprint(ttyfd, "? ");
	Fflush(ttyfd);
	if(read(ttyfd, yes, 2)!=2
	    || yes[0]!='y'
	    || yes[1]!='\n'
	    || unpex(ttyfd, -1)) {
		unpex(ttyfd, -1);
		logpr("retracted\n");
		error("request retracted\n");
	}
	return 1;
}

pexinput()
{
	struct pexclude pexbuf;

	if( pex(ttyfd, 15, &pexbuf) != 0
	    || pexbuf.farpid == 0
	    || pexbuf.farpid>0 && pexbuf.farcap==0
	    || pexbuf.farpid == -1 && uenv.ttysrc[0] == 0)
		return -1;
	else
		return 0;
}
