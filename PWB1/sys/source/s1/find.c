#include "stdio.h"
#include "stat.h"
#define A_DAY	86400L /* a day full of seconds */
#define EQ(x,y)	(strcmp(x,y)==0)

int	randlast;
char	pathname[200];

struct anode {
	int (*F)();
	struct anode *L, *R;
} node[100];
int nn;  /* number of nodes */
char	*fname;
long	now;
int	Argc,
	Ai,
	Pi;
char	**Argv;

struct statb Statb;

struct	anode	*exp(),
		*e1(),
		*e2(),
		*e3(),
		*mk();

main(argc,argv) char *argv[]; {

	struct anode *exlist;
	int paths;
	register char *cp, *sp = 0;

	time(&now);
	Argc = argc; Argv = argv;
	if(argc<3) {
usage:		fprintf(stderr, "Usage: find path-list predicate-list\n");
		exit(1);
	}
	for(Ai = paths = 1; Ai < (argc-1); ++Ai,++paths)
		if(*Argv[Ai] == '-' || EQ(Argv[Ai], "(") || EQ(Argv[Ai], "!"))
			break;
	if(paths == 1) /* no path-list */
		goto usage;
	if(!(exlist = exp())) { /* parse and compile the arguments */
		fprintf(stderr, "Parsing error\n");
		exit(1);
	}
	if(Ai<argc) {
		fprintf(stderr, "Missing conjunction\n");
		exit(1);
	}
	for(Pi = 1; Pi < paths; ++Pi) {
		strcpy(pathname, Argv[Pi]);
		fname = pathname;
		for(cp = pathname; *cp; ++cp)
			if(*cp == '/')
				sp = cp + 1;
		fname = sp? sp: pathname;
		descend(pathname, exlist); /* to find files that match  */
	}
	exit(0);
}

/* compile time functions:  priority is  exp()<e1()<e2()<e3()  */

struct anode *exp() { /* parse -o ... */
	int or();
	int p1;
	char *na;
	p1 = e1() /* get left operand */ ;
	if(EQ(na=nxtarg(),"-o")) {
		randlast--;
		return(mk(&or,p1,exp()));
	}
	else if(Ai <= Argc) --Ai;
	return(p1);
}
struct anode *e1() { /* parse -a */
	int and();
	int p1;
	char *na;
	p1 = e2();
	if(EQ(na=nxtarg(),"-a")) {
		randlast--;
		return(mk(&and,p1,e1()));
	}
	else if(Ai <= Argc) --Ai;
	return(p1);
}
struct anode *e2() { /* parse not (!) */
	int not();
	char *na;
	if(randlast) {
		fprintf(stderr, "Operand follows operand\n");
		exit(1);
	}
	randlast++;
	if(EQ(na=nxtarg(),"!"))
		return(mk(&not,e3(),0));
	else if(Ai <= Argc) --Ai;
	return(e3());
}
struct anode *e3() { /* parse parens and predicates */
	int exeq(), ok(), glob(),  mtime(), atime(), user(),
		group(), size(), perm(), links(), print(),
		type(), ino();
	int p1, i;
	char *a, *b, s;
	a = nxtarg();
	if(EQ(a,"(")) {
		randlast--;
		p1 = exp();
		a = nxtarg();
		if(EQ(a,")")!=0) goto err;
		return(p1);
	}
	else if(EQ(a,"-print")) {
		return(mk(&print,0,0));
	}
	b = nxtarg();
	s = *b;
	if(s=='+') b++;
	if(EQ(a,"-name"))
		return(mk(&glob,b,0));
	else if(EQ(a,"-mtime"))
		return(mk(&mtime,atoi(b),s));
	else if(EQ(a,"-atime"))
		return(mk(&atime,atoi(b),s));
	else if(EQ(a,"-user")) {
		if((i=getunum("/etc/passwd", b)) == -1) {
			fprintf(stderr, "Cannot find -user name\n");
			exit(1);
		}
		return(mk(&user,i,s));
	}
	else if(EQ(a,"-unum"))
		return(mk(&user,atoi(b),s));
	else if(EQ(a,"-inum"))
		return(mk(&ino,atoi(b),s));
	else if(EQ(a,"-group")) {
		if((i=getunum("/etc/group", b)) == -1) {
			fprintf(stderr, "Cannot find -group name\n");
			exit(1);
		}
		return(mk(&group,i,s));
	} else if(EQ(a,"-size"))
		return(mk(&size,atoi(b),s));
	else if(EQ(a,"-links"))
		return(mk(&links,atoi(b),s));
	else if(EQ(a,"-perm")) {
		for(i=0; *b ; ++b) {
			if(*b=='-') continue;
			i =<< 3;
			i = i + (*b - '0');
		}
		return(mk(&perm,i,s));
	}
	else if(EQ(a,"-type")) {
		i = s=='d' ? IFDIR :
		    s=='b' ? IFBLK :
		    s=='c' ? IFCHR :
		    000000;
		return(mk(&type,i,0));
	}
	else if (EQ(a,"-exec")) {
		i = Ai - 1;
		while(EQ(nxtarg(),";")!=0);
		return(mk(&exeq,i,0));
	}
	else if (EQ(a,"-ok")) {
		i = Ai - 1;
		while(EQ(nxtarg(),";")!=0);
		return(mk(&ok,i,0));
	}
err:	fprintf(stderr, "bad option < %s >\n", a);
	exit(1);
}
struct anode *mk(f,l,r) struct anode *l,*r; { /*make an expression node*/
	node[nn].F = f;
	node[nn].L = l;
	node[nn].R = r;
	return(&(node[nn++]));
}

nxtarg() { /* get next arg from command line */
	static strikes = 0;

	if(strikes==3) {
		fprintf(stderr, "Incomplete statement\n");
		exit(1);
	}
	if(Ai>=Argc) {
		strikes++;
		Ai = Argc + 1;
		return("");
	}
	return(Argv[Ai++]);
}

/* execution time functions */
and(p) register struct anode *p; {
	return(((*p->L->F)(p->L)) && ((*p->R->F)(p->R))?1:0);
}
or(p) register struct anode *p; {
	 return(((*p->L->F)(p->L)) || ((*p->R->F)(p->R))?1:0);
}
not(p) register struct anode *p; {
	return( !((*p->L->F)(p->L)));
}
glob(p) register struct { int f; char *pat; } *p;  {
	return(gmatch(fname,p->pat));
}
print() {
	puts(pathname);
	return(1);
}
mtime(p) register struct { int f, t, s; } *p;  {
	return(scomp((int)((now - Statb.i_mtime) / A_DAY), p->t, p->s));
}
atime(p) register struct { int f, t, s; } *p;  {
	return(scomp((int)((now - Statb.i_atime) / A_DAY), p->t, p->s));
}
user(p) register struct { int f, u, s; } *p;  {
	return(scomp(Statb.i_uid&0377,p->u,p->s));
}
ino(p) register struct { int f, u, s; } *p; {
	return(scomp(Statb.i_ino,p->u,p->s));
}
group(p) register struct { int f, u; } *p;  {
	return(p->u == Statb.i_gid);
}
links(p) register struct { int f, link, s; } *p;  {
	return(scomp(Statb.i_nlink,p->link,p->s));
}
size(p) register struct { int f, sz, s; } *p;  {
	return(scomp(nblock(Statb.i_size0,Statb.i_size1),p->sz,p->s));
}
perm(p) register struct { int f, per, s; } *p;  {
	register i;
	i = (p->s=='-') ? p->per : 07777; /* '-' means only arg bits */
	return((Statb.i_mode & i & 07777) == p->per);
}
type(p) register struct { int f, per, s; } *p; {
	return((Statb.i_mode&IFMT)==p->per);
}
exeq(p) register struct { int f, com; } *p; {
	fflush(stdout); /* to flush possible `-print' */
	return(doex(p->com));
}
ok(p) struct { int f, com; } *p; {
	char c;  int yes;
	yes = 0;
	fflush(stdout);
	fprintf(stderr, "< %s ... %s ... > ?   ", Argv[p->com], pathname);
	fflush(stderr);
	if((c=getchar())=='y') yes = 1;
	while(c!='\n') c = getchar();
	if(yes) return(doex(p->com));
	return(0);
}

/* support functions */
scomp(a, b, s) /* funny signed compare */
register a, b;
register char s;
{
	if(s == '+')
		return(a > b);
	if(s == '-')
		return(a < (b * -1));
	return(a == b);
}

doex(com) {
	register np;
	register char *na;
	static char *nargv[50];
	static ccode;

	ccode = np = 0;
	while(com < Argc) {
		na = Argv[com++];
		if(EQ(na,";")) break;
		if(EQ(na,"{}")) nargv[np++] = pathname;
		else nargv[np++] = na;
	}
	nargv[np] = 0;
	if (np==0) return(9);
	if(fork()) /*parent*/ wait(&ccode);
	else { /*child*/
		pexec(nargv[0], nargv);
		exit(1);
	}
	return(ccode ? 0:1);
}

getunum(f, s) char *f, *s; { /* find user/group name and return number */
	register i;
	register char *sp, c;
	char str[20];
	FILE *pin;

	i = -1;
	pin = fopen(f, "r");
	c = '\n'; /* prime with a CR */
	do {
		if(c=='\n') {
			sp = str;
			while((*sp = getc(pin)) != ':')
				if(*sp++ == -1) goto RET;
			*sp = '\0';
			if(EQ(str,s)) {
				while((c=getc(pin)) != ':')
					if(c == -1) goto RET;
				sp = str;
				while((*sp = getc(pin)) != ':') sp++;
				*sp = '\0';
				i = atoi(str);
				goto RET;
			}
		}
	} while((c = getc(pin)) != EOF);
 RET:
	fclose(pin);
	return(i);
}

descend(name, exlist)
struct anode *exlist;
char *name;
{
	int	dir = 0, /* open directory */
		offset,
		dsize,
		entries,
		dirsize;
	struct dir_entry {
		int	dinode;
		char	dname[14];
	} dentry[32];
	register struct dir_entry	*dp;
	register char *c1, *c2;
	int i;
	char *endofname;

	if(stat(name,&Statb)<0) {
		fprintf(stderr, "--bad status < %s >\n", name);
		return(0);
	}
	(*exlist->F)(exlist);
	if((Statb.i_mode&IFMT)!=IFDIR)
		return(1);

	for(c1 = name; *c1; ++c1);
	endofname = c1;
	dirsize = Statb.i_size1;
	for(offset=0 ; offset < dirsize ; offset =+ 512) { /* each block */
		dsize = 512<(dirsize-offset)? 512: (dirsize-offset);
		if(!dir) {
			if((dir=open(name,0))<0) {
				fprintf(stderr, "--cannot open < %s >\n",
					name);
				return(0);
			}
			if(offset) seek(dir,offset,0);
			if(read(dir,&dentry,dsize)<0) {
				fprintf(stderr, "--cannot read < %s >\n",
					name);
				close(dir);
				return(0);
			}
			if(dir > 10) {
				close(dir);
				dir = 0;
			}
		} else 
			if(read(dir,&dentry,dsize)<0) {
				fprintf(stderr, "--cannot read < %s >\n",
					name);
				close(dir);
				return(0);
			}
		for(dp=dentry, entries=dsize>>4; entries; --entries, ++dp) { /* each directory entry */
			if(dp->dinode==0
			|| (dp->dname[0]=='.' && dp->dname[1]=='\0')
			|| (dp->dname[0]=='.' && dp->dname[1]=='.' && dp->dname[2]=='\0'))
				continue;
			if (dp->dinode == -1)
				continue;
			c1 = endofname;
			*c1++ = '/';
			c2 = dp->dname;
			for(i=0; i<14; ++i)
				if(*c2)
					*c1++ = *c2++;
				else
					break;
			*c1 = '\0';
			if(c1 == endofname) /* ?? */
				return 0;
			fname = endofname+1;
			if(descend(name, exlist)==0)
				fprintf(stderr, "--? < %s >\n", name);
		}
	}
	if(dir)
		close(dir);
	return(1);
}

gmatch(s, p) /* string match as in glob */
register char *s, *p; {
	if (*s=='.' && *p!='.') return(0);
	return amatch(s, p);
}

amatch(s, p)
register char *s, *p;
{
	register cc;
	int scc, k;
	int c, lc;

	scc = *s;
	lc = 077777;
	switch (c = *p) {

	case '[':
		k = 0;
		while (cc = *++p) {
			switch (cc) {

			case ']':
				if (k)
					return(amatch(++s, ++p));
				else
					return(0);

			case '-':
				k =| lc <= scc & scc <= (cc=p[1]);
			}
			if (scc==(lc=cc)) k++;
		}
		return(0);

	case '?':
	caseq:
		if(scc) return(amatch(++s, ++p));
		return(0);
	case '*':
		return(umatch(s, ++p));
	case 0:
		return(!scc);
	}
	if (c==scc) goto caseq;
	return(0);
}

umatch(s, p)
register char *s, *p;
{
	if(*p==0) return(1);
	while(*s)
		if (amatch(s++,p)) return(1);
	return(0);
}

/* nblock() borrowed from "ls -s" */
nblock(size0, siz)
char *size0, *siz;
{
	register int n;

	n = ldiv(size0, siz, 512);
	if (siz&0777)
		n++;
	if (n>8)
		n =+ (n+255)/256;
	return(n);
}
