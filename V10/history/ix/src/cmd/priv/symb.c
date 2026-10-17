#include "priv.h"

Node *
mknode(s)
register char *s;
{
	register Node *np;

	np = (Node*)calloc(1, sizeof(Node));
	np->nodename = strdup(s);
	np->nextx = nodeall;
	nodeall = np;
	return np;
}

/*
 * look up authorization node by name
 *  if flag nonzero create it if needed
 */
Node *
getnode(s, flag)
register char *s;
{
	register Node *dp, *np;
	register char *t;
	if(*s == 0 || strcmp(s, "/")== 0) return rootnode;
	if(*s != '/')
			error("nodename %s doesn't begin with /", s);
	t = strrchr(s, '/');
	if(t==0)
		return 0;
	*t = 0;
	dp = getnode(s, flag);
	*t = '/';

	if(dp == 0) {
		error("no parent for '%s'", s);
		return 0;
	}
	for(np=dp->child; np; np=np->sib)
		if(strcmp(s, np->nodename)==0)
			return np;
	if(flag) {
		np = mknode(s);
		np->sib = dp->child;
		np->parent = dp;
		dp->child = np;
	} else
		error("no node '%s'", s);
	return np;
}

makeright(f)
register Func *f;
{
	for(;f;f=f->next) {
		if(f->ac>1)error("right %s too many args", f->name);
		if(getright(f->name,f->av[0],1))
			error("multiple definition of %s", f->name);
	}
}

/* look up right named s with param t.  create if flag!=0. */

Func *
getright(s, t, flag)
register char *s;
char *t;
{
	register Func *f;
	for(f=rightroot; f; f=f->next) {
		if(strcmp(f->name,s)==0) 
			return f;
	}
	if(flag) {
		f = func(s, t?1:0, &t);
		f->type = F_RGHT;
		if(t == 0)
			;
		else if(strcmp(t, "RE")==0)
			f->type |= F_RE;
		else if(strcmp(t, "LAB")==0)
			f->type |= F_LAB;
		else if(strcmp(t, "PRIV")==0)
			f->type |= F_PRIV2;
		else {
printf("%s %s %d\n", s, t, flag);
			error("bad right type %s", t);
		}
		rightroot = funlist(rightroot, f);
	}
	return 0;
}
int setceil();
int setpriv();
int command();
int checkid();
int checkpw();
int checksrc();
Func builtins[] = {
	{"CEILING",	F_ACT|F_CEIL, 	 1 },
	{"PRIV",	F_ACT|F_PRIV,	 1 },
	{"EXEC",	F_ACT|F_EXEC,	-1 },
	{"DAEMON",	F_ACT|F_DAEM,	-1 },
	{"PRIVEDIT",	F_ACT|F_EDIT,	 2 },
	{"ANYSRC",	F_ACT|F_ANYS,	 0 },

	{"ID",		F_PRED|F_ID,	 1 },
	{"SRC",		F_PRED|F_SRC,	 1 },
	{"PW",		F_PRED|F_PW,	-1 },
	{0},
};


Func *
func(name, ac, av)
register char *name;
register char **av;
{
	register int i;
	register Func *f;

	f = (Func*)calloc(1,sizeof(Func));
	f->last = f;
	f->name = name;
	f->ac = ac;
	f->av = (char**)calloc((unsigned)ac+1, sizeof(char*));
	for(i=0; i<ac; i++)
		f->av[i] = av[i];
	if(strcmp(name, "REQUEST")==0) {
		f->rav = (struct dfa **)calloc((unsigned)ac+1, sizeof(struct dfa*));
	} else
		f->dav = dollar(f->av);
	f->comment = savec;
	savec = 0;
	return f;
}

struct label *
myatolab(s)
register char *s;
{
	register struct label *lp, *np;

	lp = atolab(s);
	if(lp != 0) {
		np = (struct label *)calloc(1, sizeof(struct label));
		*np = *lp;
		lp = np;
	} else
		error("illegal label '%s'", s);
	return lp;
}

bick(f, type)
register Func *f;
enum classtype type;
{
	register Func *g;
	
	for(;f;f=f->next) {
		g = getbuilt(f->name);
		if(g==0 || (g->type&type)==0)
			error("unknown or inappropriate builtin %s", f->name);
		if(g->ac > 0 && g->ac != f->ac)
			error("wrong number of args in builtin %s", f->name);
		f->type = g->type;
		f->f = g->f;
	}
}

trightck(f)
register Func *f;
{
	register Func *g;

	for(;f;f=f->next) {
		g = getright(f->name,f->av[0],0);
		if(g==0)
			error("unknown right %s", f->name);
		if(g->ac > 0 && g->ac != f->ac)
			error("wrong number of args in right %s", f->name);
		f->type = g->type;
		eval(f, 1);
	}
}

Func *
getbuilt(s)
register char *s;
{
	register Func *f;

	for(f=builtins; f->name; f++)
		if(strcmp(f->name, s)==0) return f;
	return 0;
}
Need *
insreq(x,n)
Need *x, *n;
{
	if(n==0)
		return x;
	if(cmpreq(x->request,n->request)>0) {
		x->nexty = n;
		return x;
	}
	n->nexty = insreq(x,n->nexty);
	return n;
}
cmpreq(x,y)
Func *x, *y;
{
	int c;
	int i = 0;
	while(i<x->ac && i<y->ac) {
		c = strcmp(x->av[i],y->av[i]);
		if(c) return c;
		i++;
	}
	return x->ac > y->ac? 1: x->ac < y->ac? -1: 0;
}

needright(c1, c2, c3)
register Func *c1, *c2, *c3;
{
	register Need *n;
	register Func *x, *f;

	for(n=needy; n; n=n->nexty)
		if(sameargs(n->request,c1)) {
			onefunpr(stderr,c1);
			error("duplicate request");
		}
	for(x=c2; x; x=x->next) {
		f = getright(x->name, x->av[0], 0);
		if(f == 0) error("unknown right %s", x->name);
		if(f->ac != x->ac) error("num args %s", x->name);
		x->type = f->type;
	}
	bick(c3, F_ACT);
	n = (Need*)calloc(1, sizeof(Need));
	n->request = c1;
	n->price = c2;
	n->payback = c3;
	needy = insreq(n, needy);
}
Func *
funlist(p, f)
register Func *p, *f;
{
	register Func *q;

	if(p == 0)
		return f;
	q = p->last;
	p->last = q->next = f->last;
	return p;
}
Qlst *
mkq(f)
Func *f;
{
	Qlst *newq = (Qlst*)calloc(1,sizeof(Qlst));
	newq->lastq = newq;
	newq->qchain = f;
	return newq;
}
Qlst *
qlist(p, qitem)
register Qlst *p, *qitem;
{
	register Qlst *q;
	if(p == 0)
		return qitem;
	q = p->lastq;
	p->lastq = q->nextq = qitem->lastq;
	return p;
}
makeaclist(name, f)
register char *name;
register Func *f;
{
	register Node *np = getnode(name, 1);
	bick(f, F_PRED);
	editable(name,0);
	np->qlist = qlist(np->qlist, mkq(f));
}
makerights(name, f)
register char *name;
register Func *f;
{
	register Node *np = getnode(name, 1);
	trightck(f);
	editable(name,0);
	np->rchain = funlist(np->rchain, f);
}
