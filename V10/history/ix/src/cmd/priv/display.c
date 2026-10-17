#include "priv.h"

display(file)
register FILE *file;
{
	setbuf(file,malloc(BUFSIZ));
	if(rightroot) {
		commpr(file,rightroot);
		fprintf(file,"DEFINE	");
		funpr(file,rightroot, ",\n\t");
		fprintf(file,";\n\n");
	}

	needpr(file,needy);
	qrwalk(file,rootnode);
	fflush(file);
}
needpr(file,x)
register FILE *file;
register Need *x;
{
	if(x==0)return;
	needpr(file,x->nexty);
	
	commpr(file,x->request);	
	funpr(file,x->request, (char*)0);
	fprintf(file,"\n\tNEEDS\t");
	funpr(file,x->price, ",");
	fprintf(file,"\n\tDOES\t");
	funpr(file,x->payback, ",\n\t\t");
	fprintf(file,";\n");
}
qrwalk(file,np)
register FILE *file;
register Node *np;
{
	if(np==0) return;
	qrwalk(file,np->sib);
	fprintf(file,"\n");
	qpr(file,np->qlist,np->nodename);
	commpr(file,np->rchain);
	fprintf(file,"RIGHTS %s\t", np->nodename);
	funpr(file,np->rchain, ",\n\t\t");
	fprintf(file,";\n");
	qrwalk(file,np->child);
}
qpr(file,q,s)
register FILE *file;
register Qlst *q;
char *s;
{
	for(;q;q=q->nextq) {
		commpr(file,q->qchain);
		fprintf(file,"ACCESS %s\t", s);
		funpr(file,q->qchain, ",\n\t\t");
		fprintf(file,";\n");
	}
}
funpr(file,f,sep)
register FILE *file;
register Func *f;
register char *sep;
{
	if(f == 0) return;
	if(f->comment) {
		commpr(file,f);
		fprintf(file,"\t\t");
	}
	onefunpr(file,f);
	if(sep && f->next) {
		fprintf(file,"%s", sep);
		funpr(file,f->next, sep);
	}
}
onefunpr(file,f)
register FILE *file;
register Func *f;
{
	register int i;

	fprintf(file,"%s", f->name);
	if(f->ac > 0) {
		fprintf(file,"(");
		for(i=0; i<f->ac; i++)
			argpr(file,f->av[i], i==0?"":" ");
		fprintf(file,")");
	}
}
commpr(file,f)
register FILE *file;
register Func *f;
{
	if(f==0 || f->comment==0)
		return;
	fprintf(file,"#%s\n",f->comment);
	f->comment = 0;
} 

argpr(file,arg,sep)
register FILE *file;
register char *arg, *sep;
{
	char *quote = strpbrk(arg,",(); \t")?"\"":"";
	fprintf(file,"%s%s%s%s",sep,quote,arg,quote);
}


/* write on log file; guarantee a single record */
logit(comment,ac,av)
char *comment, **av;
{
	int i;
	int l = strlen(comment) + 2;
	char *buf;
	int logfile;
	if(nflag)
		return;
	for(i=0; i<ac; i++)
		l += strlen(av[i]) + 1;
	buf = malloc(l);
	strcpy(buf,comment);
	for(i=0; i<ac; i++) {
		strcat(buf," ");
		strcat(buf,av[i]);
	}
	strcat(buf,"\n");
	logfile = open("/dev/log/privlog",1);
	write(logfile,buf,l-1);
	close(logfile);
	free(buf);
}
