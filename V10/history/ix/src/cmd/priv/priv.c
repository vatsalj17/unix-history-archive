#include "priv.h"
char *privs = "/etc/privs";
Func *rightroot;
Node *nodeall, *rootnode;
Need *needy;
char *editnode;
int nflag;
char **uav;
int uac;

main(ac, av)
register char **av;
{
	register Need *granted;
	register Need *x;

	while(ac>1 && av[1][0] == '-') {
		if(strcmp(av[1], "-d")==0)
			debug++;
		if(strcmp(av[1], "-n")==0) {
			nflag++;
			setbuf(stdout,(char*)0);
		} 
		if(strcmp(av[1], "-f")==0) {
			if(ac <= 2)
				error("bad -f option");
			privs = av[2];
			ac--, av++;
		}
		ac--, av++;
		if(ac < 1)
			error("arg count");
	}
	if(!istrusted(privs)) {
		savelab();
		if(plab.lb_t|plab.lb_u) {
			fprintf(stderr,
				"%s untrusted; relinquish licenses\n",privs);
			plab.lb_u = 0;
			setplab(&plab,&pceil);
		}
	}

	rootnode = mknode("/");
	editnode = NULL;

	ac--;
	av++;
	uav = av;
	uac = ac;
	parse(privs);

	if(ac<1) {
		display(stdout);
		monotone(rootnode);
		exit(0);
	}
	granted = 0;
	for(x=needy; x; x=x->nexty) {
		register Node *tn;
		if(reqmatch(x->request, av))  {
			evalprice(x->price);
			for(tn=nodeall;tn;tn=tn->nextx) {
				if(x->price == 0
				|| try(x->price,tn->rchain)
				&& exerciser(tn->qlist)){
					granted = dupreq(x,granted);
					break;
				}
			}
		}
	}
	if(!granted || nflag) {
		fprintf(stderr,"\n");
		funpr(stderr,func("REQUEST",ac,av),"");
		fprintf(stderr," denied\n");
		if(!nflag) {
			logit("priv: denied:",ac,av);
			return 1;
		}
	}
	logit("priv: granted:",ac,av);
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
	if(debug) {
		fprintf(stderr,"covered: ");
		onefunpr(stderr,f);
		fprintf(stderr," < ");
		if(g) funpr(stderr,g,",");
		fprintf(stderr,"\n");
	}
	for( ; g; g=g->next) {
		if(strcmp(f->name,g->name) !=0 ) continue;
		if(f->ac != g->ac)error("aaaa!!!! %s", f->name);
		if(f->type!=g->type)error("bbbbb!!!! %s", f->name);
		if(f->ac == 0) return 1;
		switch(f->type) {
		case F_RGHT|F_RE:
			if(dfamatch(g->val.re, f->val.string))
				return 1;
			continue;
		case F_RGHT|F_PRIV2:
			if(privLE(f->val.priv, g->val.priv))
				return 1;
			continue;
		case F_RGHT|F_LAB:
			if(labLE(f->val.lp, g->val.lp))
				return 1;
			continue;
		default:
			error("zounds! %d", f->type);
		}
	}
	return 0;
}

evalprice(f)
register Func *f;
{
	for(;f;f=f->next) {
		if(f->ac>1)
			error("gadzooks %s", f->name);
		eval(f, 0);
	}
}

eval(f, flag)
register Func *f;
{
	register char *s;
	if(f->ac == 0) return;
	s = f->dav[0];
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
		fprintf(stderr,"%s(%s) type %d?\n",
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
				error("strange oddness %s", x->name);
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
	if(debug) {
		char s[10], t[10];
		strcpy(s,privtoa(x));
		strcpy(t,privtoa(y));
		fprintf(stderr,"privLE(%s,%s)=%d\n",s,t,v);
	}
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
	onefunpr(stderr,f);
	onefunpr(stderr,g);
	error(" ambiguous request");
	return 0;
}
