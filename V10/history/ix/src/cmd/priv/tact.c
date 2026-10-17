#include "priv.h"
#include <sys/filio.h>
#include <sys/pex.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

/* tests and actions for priv */

char logname[9];
struct label plab, pceil;

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
exerciser(ql)
register Qlst *ql;
{
	for(;ql;ql=ql->nextq)
		if(exer(ql->qchain))
			return 1;
	return 0;
}
exer(q)
register Func *q;
{
	if(debug){
		fprintf(stderr,"ACCESS: \n");
		funpr(stderr,q,",");
		fprintf(stderr,"\n");
	}
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
			error("unknown predicate %s", q->name);
		}
	}
	return 1;
}

checkid(q)
register Func *q;
{
	if(debug) {
		fprintf(stderr,"checkid: ");
		funpr(stderr,q,",");
		fprintf(stderr,"\n");
	}
	if(logname[0]==0)
		getlogname(logname);
	if(q->val.re==0)
		q->val.re = egrepcomp(q->av[0]);
	if(q->val.re==0)
		error("dddd!!!! %s",q->av[0]);
	return(dfamatch(q->val.re, logname));
}

checkpw(q)
register Func *q;
{
	register int i;
	if(debug) {
		fprintf(stderr,"checkpw: ");
		funpr(stderr,q,",");
		fprintf(stderr,"\n");
	}
	for(i=0; q->av[i]; i++)
		if(pwquery(0, q->av[i], "pex")>0)
			return 1;
	return 0;
}

checksrc(q)
register Func *q;
{
	char source[SSRCSIZ+1];
	if(debug) {
		fprintf(stderr,"checksrc: ");
		funpr(stderr,q,",");
		fprintf(stderr,"\n");
	}
	source[SSRCSIZ] = 0;
	if(ioctl(0, FIOGSRC, (void*)source) != 0)
		return 0;
	if(q->val.re==0)
		q->val.re = egrepcomp(q->av[0]);
	return(dfamatch(q->val.re, source));
}

extern char *xv[], **xvp, **xvtop;
#define XPUSH(w) if(xvp<xvtop)*xvp++=w;else error("xvp overflow")
#define CPUSH(c) if(cp<&buf[BUFSIZ-1])*cp++=c;else error("buf overflow")

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
			if(n<uac) {
				for(t=uav[n];c = *t;t++) {
					CPUSH(c);
				}
				x = 1;
			}
			if(*s == '*') {
				if(*++s)
					error("junk after *");
				if(n<uac) {
					x = 0;
					CPUSH(0);
					XPUSH(strdup(buf));
					cp = buf;
					while(++n < uac) {
						XPUSH(uav[n]);
					}
				}
			}
		}
		if(x) {
			CPUSH(0);
			XPUSH(strdup(buf));
			cp = buf;
		}
	}
	XPUSH(0);
	n = xvp-xv;
	dav = (char**)calloc((unsigned)n+1, sizeof(char*));
	for(i=0; i<n; i++)
		dav[i] = xv[i];
	return dav;
}


execute(x)
register Need *x;
{
	register Func *q;
	register int result = 0;
	int oksrc = 0;
	savelab();
	for(q=x->payback; q; q=q->next) {
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
			error("unknown action %s",q->name);
		}
	}
	return result;
}

savelab()
{
	if(getplab(&plab, &pceil) == 0) 
		return;
	perror("");
	error("getplab!");
}

setpriv(q)
register Func *q;
{
	int priv = atopriv(q->dav[0]);
	plab.lb_u = q->val.priv = priv;
	if(priv < 0)
		error("improper priv spec %s", q->dav[0]);
}

setceil(q)
register Func *q;
{
	register struct label *lp;
	if(q->val.lp == 0)
		lp = q->val.lp = myatolab(q->dav[0]);
	if(lp == 0 || lp->lb_flag!=L_BITS)
		error("improper ceiling label %s",q->dav[0]);
	pceil = *lp;
	pceil.lb_t = pceil.lb_u = 0;
}

char *emptyenv[] = { 0 };
/* deamon = -1 no fork; 0 fork and wait; 1 fork no wait */
command(q, daemon)
register Func *q;
{
	int status;
	SIG_TYP sigint, sigquit;
	if(q->ac == 0)
		return 0;
	if(nflag) {
		int i;
		fprintf(stderr,"setplab(%s", labtoa(&plab));
		fprintf(stderr,", %s)\n", labtoa(&pceil));
		fprintf(stderr,"exec(%s", q->dav[0]);
		for(i=1;q->dav[i];i++)fprintf(stderr," %s", q->dav[i]);
		fprintf(stderr,")\n");
		return 0;
	} 
	if(daemon >= 0) {
		switch(fork()) {
		case 0:
			break;
		case -1:
			perror("can't fork");
			exit(1);
		default:
			if(daemon)
				return 0;
			sigint = signal(SIGINT,SIG_IGN);
			sigquit = signal(SIGQUIT,SIG_IGN);
			wait(&status);
			signal(SIGINT,sigint);
			signal(SIGQUIT,sigquit);
			return status;
		}
	}
	if(setplab(&plab, &pceil) != 0) {
		perror("can't grant ceiling or license");
		return 1;
	}
	execve(q->dav[0], q->dav, emptyenv);
	perror(q->dav[0]);
	exit(1);
	return 0;
}

doedit(q)
register Func *q;
{
	if(q->ac != 2)
		error("privedit arg count");
	editnode = q->dav[0];
	parse(q->dav[1]);
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
			error("%s not monotone",xp->nodename);
		monotone(xp);
	}
}

istrusted(s)
char *s;
{
	struct label flab;
	if(getflab(s,&flab) == -1) {
		perror(s);
		exit(1);
	}
	return flab.lb_t!=0 || flab.lb_u!=0;
}

vetsrc(oksrc)
{
	int infile = fileno(stdin);
	struct stat instat, erstat;
	SIG_TYP sigint;
	Func f;

	if(oksrc || nflag)
		return 1;

	sigint = signal(SIGINT, SIG_IGN);
	if(pexinput() == -1) {
		unpex(infile, -1);
		error("can't establish trusted path");
	}
	f.name = "priv";
	f.ac = uac;
	f.av = uav;
	onefunpr(stderr, &f);
	fprintf(stderr,"? ");
	if(fgetc(stdin) != 'y'
	|| fgetc(stdin) != '\n'
	|| unpex(infile, -1)) {
		unpex(infile, -1);
		error("request retracted");
	}
	signal(SIGINT, sigint);
	return 1;
}

pexinput()
{
	char src[SSRCSIZ];
	struct pexclude pexbuf;
	int infile = fileno(stdin);
	int erfile = fileno(stderr);
	if(ioctl(infile,FIOGSRC,src) == -1
	|| pex(infile, 15, &pexbuf) != 0
	|| pexbuf.farpid == 0 
	|| pexbuf.farpid>0 && pexbuf.farcap==0
	|| pexbuf.farpid == -1 && src[0] == 0
	|| ioctl(erfile, FIOQX, &pexbuf) != 0	/* stdin==stderr? */
	|| pexbuf.farpid == 0
	|| pexbuf.farpid>0 && pexbuf.farcap==0)
		return -1;
	else
		return 0;
}
