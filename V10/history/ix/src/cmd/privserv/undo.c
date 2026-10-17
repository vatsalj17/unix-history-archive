#include "priv.h"

/*remove the right (or access predicate) named in f from the chain r
  doesn't distinguish access predicates, e.g. ID(), on the basis
  of argument.
*/
Func *
rmright(r,f)
Func *r, *f;
{
	if(r == 0)
		return 0;
	if(strcmp(r->name,f->name) == 0)
		return rmright(r->next,f);
	r->next = rmright(r->next,f);
	return r;
}

hasright(r,f)
Func *r, *f;
{
	for(; r; r=r->next)
		if(strcmp(r->name,f->name) == 0)
			return 1;
	return 0;
}

/* remove the right named in f from everywhere */

rmdef(f)
Func *f;
{
	Node *n;
	editable("/",1);
	for( ; f; f=f->next) {
		if(getright(f->name,(char*)0,0)==0)
			error("%s is not a right",f->name);
		for(n=nodeall; n; n=n->nextx)
			if(hasright(n->rchain,f))
				error("right %s in use",f->name);
		rightroot = rmright(rightroot,f);
		fixlast(rightroot);
	}
}
Qlst *		/* bug: RMACCESS node F,F */
rma(ql, f)	/* will remove ACCESS node F,G */
Qlst *ql;
Func *f;
{
	Func *q, *g;
	int found = 0;
	int flen = flength(f);
	if(ql == 0)
		return ql;
	if(flen == flength(ql->qchain)) {
		for(g=f; g; g=g->next) {
			for(q=ql->qchain;q;q=q->next)
				if(samefunc(q,g))
					found++;
		}
		if(found == flen)
			return rma(ql->nextq,f);
	}
	ql->nextq = rma(ql->nextq,f);
	ql->lastq = ql->nextq? ql->nextq->lastq: ql;
	return ql;
}
samefunc(f,g)
Func *f, *g;
{
	register i;
	if(strcmp(f->name,g->name)!=0)
		return 0;
	if(f->ac != g->ac)
		return 0;
	for(i=0; i<f->ac; i++)
		if(strcmp(f->av[i],g->av[i])!=0)
			return 0;
	return 1;
}

rmaccess(name, f)
char *name;
Func *f;
{
	Node *n = getnode(name,0);
	editable(name,1);
	if(f == 0)
		n->qlist = 0;
	n->qlist = rma(n->qlist,f);
}

flength(f)
Func *f;
{
	int l = 0;
	for( ; f; f=f->next)
		l++;
	return l;
}

rmrights(name, f)
char *name;
Func *f;
{
	Node *n = getnode(name,0);
	editable(name,1);
	if(f == 0)
		n->rchain = 0;
	for( ; f; f=f->next) {
		if(getright(f->name,(char*)0,0)==0)
			error("%s is not a right",f->name);
		n->rchain = rmright(n->rchain,f);
		fixlast(n->rchain);
	}
}
/* remove node named s and everything inferior */
Node *
rmnode(n, s)
Node *n;
char *s;
{
	Node *p, *c;
	editable(s,1);
	if(n==0)
		return 0;
	n->nextx = rmnode(n->nextx, s);
	if(!nodeLE(n->nodename,s))
		return n;
	p = n->parent;
	if(p == 0)
		error("attempt to delete root %s", s);
	if(p->child == n)
		p->child = n->sib;
	else for(c=p->child; c->sib; c=c->sib) {
		if(c->sib == n) {
			c->sib = n->sib;
			break;
		}
	}
	return n->nextx;
}

rmnodes(f)
Func *f;
{
	for( ; f; f=f->next) {
		char *s = f->name;
		if(f->ac)
			error("arg list for node %s",s);
		if(getnode(f->name,0) == 0)
			error("node %s does not exist",s);
		nodeall = rmnode(nodeall,s);
	}
}

Need *
rmreq(r,n)
Func *r;
Need *n;
{
	if(n==0) return 0;
	n->nexty = rmreq(r,n->nexty);
	return sameargs(r,n->request)? n->nexty: n;
}

rmrequest(r)
Func *r;
{
	editable("/",1);
	needy = rmreq(r,needy);
}

fixlast(f)
Func *f;
{
	Func *g;
	if(f==0) return;
	for(g=f; g->next; g=g->next)
		continue;
	f->last = g;
}

/* is node name s equal or closer to root than t? */

nodeLE(s,t)
char *s, *t;
{
	int l = strlen(t);
	int v = strncmp(s,t,l)==0 && (t[l]=='/'||t[l]==0);
	return v;
}

editable(s,rmflag)
char *s;
{
	if(editnode == 0 && rmflag != 0)
		error("node %s not editable",s);
	if(editnode != 0 && !nodeLE(s,editnode))
		error("node %s not editable",s);
}
