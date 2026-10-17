#include "priv.h"

display(file)
{
	errno = 0;
	if(rightroot) {
		commpr(file,rightroot);
		Fprint(file,"DEFINE	");
		funpr(file,rightroot, ",\n\t");
		Fprint(file,";\n\n");
	}

	needpr(file,needy);
	qrwalk(file,rootnode);
	return Fflush(file)||errno;
}

needpr(file,x)
register Need *x;
{
	if(x==0)return;
	needpr(file,x->nexty);

	commpr(file,x->request);
	funpr(file,x->request, (char*)0);
	Fprint(file,"\n\tNEEDS\t");
	funpr(file,x->price, ",");
	Fprint(file,"\n\tDOES\t");
	funpr(file,x->payback, ",\n\t\t");
	Fprint(file,";\n");
}

qrwalk(file,np)
register Node *np;
{
	if(np==0) return;
	qrwalk(file,np->sib);
	Fprint(file,"\n");
	qpr(file,np->qlist,np->nodename);
	commpr(file,np->rchain);
	Fprint(file,"RIGHTS %s\t", np->nodename);
	funpr(file,np->rchain, ",\n\t\t");
	Fprint(file,";\n");
	qrwalk(file,np->child);
}

qpr(file,q,s)
register Qlst *q;
char *s;
{
	for(;q;q=q->nextq) {
		commpr(file,q->qchain);
		Fprint(file,"ACCESS %s\t", s);
		funpr(file,q->qchain, ",\n\t\t");
		Fprint(file,";\n");
	}
}

funpr(file,f,sep)
register Func *f;
register char *sep;
{
	if(f == 0) return;
	if(f->comment) {
		commpr(file,f);
		Fprint(file,"\t\t");
	}
	onefunpr(file,f);
	if(sep && f->next) {
		Fprint(file,"%s", sep);
		funpr(file,f->next, sep);
	}
}
onefunpr(file,f)
register Func *f;
{
	register int i;

	Fprint(file,"%s", f->name);
	if(f->ac > 0) {
		Fprint(file,"(");
		for(i=0; i<f->ac; i++)
			argpr(file,f->av[i], i==0?"":" ");
		Fprint(file,")");
	}
}
commpr(file,f)
register Func *f;
{
	if(f==0 || f->comment==0)
		return;
	Fprint(file,"#%s\n",f->comment);
	f->comment = 0;
}

argpr(file,arg,sep)
register char *arg, *sep;
{
	char *quote = strpbrk(arg,",(); \t")?"\"":"";
	Fprint(file,"%s%s%s%s",sep,quote,arg,quote);
}


