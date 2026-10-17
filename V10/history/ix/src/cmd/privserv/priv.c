#include "priv.h"

Func *rightroot;
Node *nodeall, *rootnode;
Need *needy;
char *editnode;

request(ac, av)
char **av;
{
	register Need *granted;
	register Need *x;
	register Node *tn;

	/* clear cache of exerciser decisions */
	for(tn=nodeall;tn;tn=tn->nextx)
		tn->may = 0;
	granted = 0;

	for(x=needy; x; x=x->nexty) {
		if(reqmatch(x->request, av))  {
			evalprice(x->price);
			for(tn=nodeall;tn;tn=tn->nextx) {
				if(x->price == 0
				|| try(x->price,tn->rchain)
				&& exerciser(tn)){
					granted = dupreq(x,granted);
					break;
				}
			}
		}
	}
	if(granted ==0 || nflag != 0) {
		/* Fprint(2,"\n"); */
		funpr(2,func("REQUEST",ac,av),"");
		Fprint(2,granted?" granted\n": " denied\n");
		Fflush(2);
		if(!nflag) {
			logpr("denied: %V\n",av);
			return 1;
		}
	}
	logpr("granted: %V\n",av);
	return execute(granted);
}

try(f, r)
register Func *f, *r;
{
	for(;f;f=f->next)
		if(!covered(f,r)) return 0;
	return 1;
}

/* is the value of f dominated by the value of g */
covered(f,g)
register Func *f, *g;
{
	for( ; g; g=g->next) {
		if(strcmp(f->name,g->name) !=0 ) continue;
		if(f->ac != g->ac)bomb("internal error 1: %s\n", f->name);
		if(f->type!=g->type)bomb("internal error 2: %s\n", f->name);
		if(f->ac == 0) {
			goto yes;
		}
		switch(f->type) {
		case F_RGHT|F_RE:
			if(dfamatch(g->val.re, f->val.string))
				goto yes;
			continue;
		case F_RGHT|F_PRIV2:
			if(privLE(f->val.priv, g->val.priv))
				goto yes;
			continue;
		case F_RGHT|F_LAB:
			if(labLE(f->val.lp, g->val.lp))
				goto yes;
			continue;
		default:
			bomb("internal error 3: %d\n", f->type);
		}
	}
	return 0;
yes:
	return 1;
}

evalprice(f)
register Func *f;
{
	for(;f;f=f->next) {
		if(f->ac>1)
			bomb("internal error 4: %s\n", f->name);
		eval(f, 0);
	}
}

/*
 * flag=1 means general pattern, called from trightck (at 'compile' time).
 * flag=0 means particular instanc, called from evalprice (at 'run' time).
 */
eval(f, flag)
register Func *f;
{
	register char *s;

	if(f->ac == 0 || f->valid != 0) return;
	f->valid = 1;
	if(flag)
		s = f->av[0];
	else {
		dset(f);
		s = f->dav[0];
	}
	switch(f->type) {
	case F_RGHT|F_RE:
		if(flag==0) f->val.string = s;
		else f->val.re = egrepcomp(s);
		break;
	case F_RGHT|F_PRIV2:
		f->val.priv = atopriv(s);
		break;
	case F_RGHT|F_LAB:
		f->val.lp = myatolab(s);
		break;
	default:
		logpr("%s(%s) type %d?\n",
			f->name, s, f->type);
		exit(1);
	}
}

/* is every right in lo dominated by some in hi? */
above(fhi,flo)
register Func *fhi, *flo;
{
	register Func *x;
	register int ok;
	for(;flo;flo=flo->next) {
		ok = 0;
		for(x=fhi;x;x=x->next) {
			if(strcmp(x->name,flo->name))continue;
			switch(x->type){
			case F_RGHT|F_RE:
				if(dfale(flo->val.re, x->val.re))
					ok++;
				break;
			case F_RGHT|F_PRIV2:
				if (privLE(flo->val.priv, x->val.priv))
					ok++;
				break;
			case F_RGHT|F_LAB:
				if (labLE(flo->val.lp, x->val.lp))
					ok++;
				break;
			case F_RGHT:
				ok++;
				break;
			default:
				bomb("internal error 5: %s\n", x->name);
			}
			if(ok)break;
		}
		if(!ok)return 0;
	}
	return 1;
}

privLE(x,y)
register x, y;
{
	int v = !(x & ~y);
	return v;
}

Need*		/* pick the most specific request */
dupreq(x,y)	/* by strict language inclusion */
Need *x, *y;
{
	int which = 0;	/* -1: x is better, 1: y is better */
	int i, fleg, glef;
	struct dfa *fr, *gr;
	static struct dfa *sigmastar;
	Func *f, *g;
	if(sigmastar==0) sigmastar = egrepcomp(".*");
	if(y==0) return x;
	if(x==0) return y;
	f = x->request;
	g = y->request;
	if(f->ac > g->ac) which = -1;
	if(g->ac > f->ac) which = 1;
	for(i=0; i<max(f->ac,g->ac); i++) {
		if(i<f->ac && f->rav[i] == 0)
			f->rav[i] = egrepcomp(f->av[i]);
		if(i<g->ac && g->rav[i] == 0)
			g->rav[i] = egrepcomp(g->av[i]);
		fr = i<f->ac? f->rav[i]: sigmastar;
		gr = i<g->ac? g->rav[i]: sigmastar;
		fleg = dfale(fr,gr);
		glef = dfale(gr,fr);
		if(fleg && glef) continue;
		else if(fleg && which<=0) which = -1;
		else if(glef && which>=0) which = 1;
		else goto bad;
	}
	if(which==-1) return x;
	if(which==1) return y;
bad:
	onefunpr(2,f);
	onefunpr(2,g);
	error(" ambiguous request\n");
	return 0;
}
