#
# include "stdio.h"
/* C command */

# define SBSIZE 10000
# define DROP (-2)
# define SAME 0
# define MAXINC 10
char	sbf[SBSIZE];
# define CHSPACE 1000
char	ts[CHSPACE+50];
# define EXPSIZE 500
char *strdex(), *copy(), *calloc(), *token(), *coptok();
char	*tsa ts;
char	*tsp ts;
char *fnames[MAXINC];
# define LINELEN 512
FILE 	*fin;
FILE	*fout;
int	instring;
char	direct[50];
int	nd 1;
char	*dirs[10] {direct, 0};
char	nfil[100];
int	pflag;
int	depth;
int	skipcom;
FILE	*fins[MAXINC];
int	ifno;
char	*lp;
char	*line;
# define NPREDEF 20
char *prespc[NPREDEF];
char **predef prespc;
char *punspc[NPREDEF];
char **prund punspc;
char **predp;
int	lineno[MAXINC];
int	exfail;
struct symtab {
	char	name[8];
	char	*value;
} *symtab, *lookup();
# define symsiz 400
struct symtab *defloc;
struct symtab *udfloc;
struct symtab *incloc;
struct symtab *ifloc;
struct symtab *elsloc;
struct symtab *eifloc;
struct symtab *ifdloc;
struct symtab *ifnloc;
struct symtab *sysloc;
struct symtab *lneloc;
struct symtab *prdloc;
int	trulvl;
int	flslvl;
char	*stringbuf;

mainpp(argc,argv)
	char *argv[];
{
	int i;
# if tgp
	int ifbrk;
# endif
	char ln[LINELEN];
	register int c;
	register char *rlp;
	char *sp;
	struct symtab stab[symsiz];

	fin = stdin;
	fout = stdout;
# if unix
	fnames[ifno=0] = "";
# endif
# if gcos
	fnames[ifno=0] = "s*";
# endif
# if ibm
	fnames[ifno=0] = "";
# endif
	for(i=1; i<argc; i++)
		{
		switch(argv[i][0])
			{
			case '-':
			switch(argv[i][1])
				{
				case 'P':
					pflag++;
				case 'E':
					continue;
				case 'D':
					if (predef>prespc+NPREDEF)
						{
						error("too many -D options, ignoring %s",argv[i]);
						continue;
						}
					*predef++ = argv[i]+2;
					continue;
				case 'U':
					if (prund>punspc+NPREDEF)
						{
						error("too many -U options, ignoring %s",argv[i]);
						continue;
						}
					*prund++ = argv[i]+2;
					continue;
				case 'I':
					if (nd>8)
						error("excessive -I file (%s) ignored",argv[i]);
					else
						dirs[nd++] = argv[i]+2;
					continue;
				case '\0': continue;
				default: 
					error("unknown flag %s", argv[i]);
					continue;
				}
			default:
				if (fin==stdin)
					{
					fin = fopen(argv[i], "r");
					if (fin==NULL)
						{
						error("No source file %s",argv[i]);
						exit(8);
						}
					fnames[ifno]=argv[i];
					strcpy(direct, argv[i]);
					for(sp=direct; *sp; sp++);
					while (sp>direct && *sp != '/') sp--;
# if unix
					if (sp==direct)
						*sp++ = '.';
# endif
					*sp=0; /* direct now has place where source file is */
					}
				else
				if (fout==stdout)
					{
					fout= fopen(argv[i], "w");
					if (fout==NULL)
						{
						error("Can't write %s", argv[i]);
						exit(8);
						}
					}
				else
					error("extraneous name %s", argv[i]);
			}
		}

	fins[ifno]=fin;
	exfail = 0;
		/* after user -I files here are the standard include libraries */
# if unix
	dirs[nd++] = "/usr/include";
# endif
# if gcos
	dirs[nd++] = "cc";
# endif
# if ibm
	dirs[nd++] = "stdio";
# endif
	/* dirs[nd++] = "/compool"; */
	dirs[nd++] = 0;
	symtab = stab;
	for (c=0; c<symsiz; c++) {
		stab[c].name[0] = '\0';
		stab[c].value = 0;
	}
	insym(&defloc, "define");
	insym(&udfloc, "undef");
	insym(&incloc, "include");
	insym(&elsloc, "else");
	insym(&eifloc, "endif");
	insym(&ifdloc, "ifdef");
	insym(&ifnloc, "ifndef");
	insym(&ifloc,  "if");
# if unix
	insym(&sysloc, "unix");
# endif
# if gcos
	insym (&sysloc, "gcos");
# endif
# if ibm
	insym (&sysloc, "ibm");
# endif
	insym(&lneloc, "line");
	predp=predef;
	while (predp>prespc)
		if (sp=strdex(*--predp, '='))
			{
			*sp++=0;
			stsym(*predp, sp);
			}
		else
			insym(&prdloc, *predp);
	predp=prund;
	while (predp>punspc)
		{
		if (sp=strdex(*--predp, '='))
			*sp++=0;
		lookup(*predp, DROP);
		}
	stringbuf = sbf;
	trulvl = 0;
	flslvl = 0;
	line  = ln;
	lineno[0] = 1;
	if (pflag==0) fprintf(fout, "# 1 \"%s\"\n", fnames[ifno]);
	while(getline()) {
		skipcom=0;
		if (ln[0] != '#' && flslvl==0)
			{
# if tgp
			ifbrk= checklen(line);
# endif
			for (rlp = line; c = *rlp++;)
				putc(c, fout);
# if tgp
			if (ifbrk)
				fprintf(fout,"\n# %d",lineno[ifno]);
# endif
			}
		putc('\n', fout);
	}
# if tgp
	checklen(line);
# endif
	for(rlp=line; c = *rlp++;)
			putc(c,fout);
}

getline()
{
	register int c, sc, state;
	struct symtab *np;
	char *namep, *filname, **dirp;
	int filok, inctype;

	lp = line;
	*lp = '\0';
	state = 0;
	if ((c=getch()) == '#')
		state = 1;
	while (c!='\n' && c!='\0') {
		if (letter(c)) {
			namep = lp;
			sch(c);
			while (letnum(c=getch()))
				sch(c);
			sch('\0');
			lp--;
			if (state==6)
				{
				lookup(namep, DROP);
				goto out;
				}
			if (state>3 && state <6) {
				if (flslvl==0 &&(state+!lookup(namep,-1)->name[0])==5)
					trulvl++;
				else
					flslvl++;
		out:
				while (c!='\n' && c!= '\0')
					c = getch();
				return(c);
			}
			if (state==3) /* include */
				if (*namep != '"' && *namep != '<')
					{
					error("Bad include syntax", 0);
					state=1;
					}
			if (state!=2 || flslvl==0)
				{
				pushback(c);
				np = lookup(namep, state);
				c = getch();
				}
			if (state==1) {
				if (np==defloc)
					skipcom = state = 2;
				else if (np==incloc)
					state = 3;
				else if (np==ifnloc)
					state = 4;
				else if (np==ifdloc)
					state = 5;
				else if (np==eifloc) {
					if (flslvl)
						--flslvl;
					else if (trulvl)
						--trulvl;
					else errback("If-less endif",0);
					goto out;
					}
				else if (np==elsloc) {
					if (flslvl)
						--flslvl? ++flslvl : ++trulvl;
					else if (trulvl)
						{++flslvl; --trulvl;}
					else
						errback("If-less else",0);
					goto out;
					}
				else if (np==udfloc) {
					state=6;
					}
				else if (np==ifloc) {
					if (flslvl ==0 && yyparse())
						trulvl++;
					else
						flslvl++;
					return('\n');
					}
				else if (np==lneloc)
					{
					if(pflag==0) fprintf(fout, "# ");
					lp=line;
					for(; c !='\n' && c != '\0'; c=getch())
						if (!pflag)
							sch(c);
					sch('\0');
					return(c);
					}
				else {
					errback("Undefined control",0);
					while (c!='\n' && c!='\0')
						c = getch();
					return(c);
				}
			} else if (state==2) {
				if (flslvl)
					goto out;
				np->value = stringbuf;
				if (c != '\n' && c != 0)
					{
					savch(c);
					while ((c=getch())!='\n' && c!='\0')
						{
						if (c== '\\')
							{
							c = getch();
							if (c=='\n')continue;
							savch('\\');
							}
						savch(c);
						}
					}
				savch('\0');
				return(1);
			}
			continue;
		} else if ((sc=c) == '\'' || sc== '"' || (state==3 && sc== '<')) {
			sch(sc);
			filname = lp;
			inctype = sc=='<';
			if (sc== '<')
				{
			/*
				fprintf(fout==stdout?stderr:stdout, "note: include <> obsolete, use \"\"\n");
			*/
				sc= '>';
				}
			instring++;
			while ((c=getch())!=sc && c!='\n' && c!='\0') {
				sch(c);
				if (c=='\\')
					sch(getch());
			}
			instring = 0;
			if (flslvl)
				goto out;
			if (state==3) {
				if (flslvl)
					goto out;
				*lp = '\0';
				while ((c=getch())!='\n' && c!='\0');
				if (ifno+1 >=MAXINC)
					error("Unreasonable include nesting",0);
				filok=0;
				for(dirp=dirs+inctype; *dirp; dirp++)
					{
					if (filname[0]=='/' || **dirp=='\0')
						strcpy(nfil,filname);
					else
						{
						strcpy(nfil,*dirp);
# ifdef unix
						strcat(nfil, "/");
# endif
# ifdef gcos
						strcat(nfil, "/");
# endif
# ifdef ibm
						strcat(nfil, ".");
# endif
						strcat(nfil, filname);
						}
					if ( (fins[ifno+1]=fopen(nfil, "r"))!=NULL)
						{
						filok=1;
						fin = fins[++ifno];
						break;
						}
					}
				if (filok==0)
					errback("Can't find include file %s", filname);
				else
					{
					if (pflag==0) fprintf(fout, "\n# 1 \"%s\"", filname);
					lineno[ifno]=1;
					fnames[ifno] = copy(filname);
					}
				return(c);
			}
		}
		sch(sc=c);
		c = getch();
		if (isdigit(sc))
			{
			for (;isalpha(c) || isdigit(c); c=getch())
				sch(c);
			}
	}
	sch('\0');
	if (state>1)
		errback("Control syntax",0);
	return(c);
}
insym(sp, namep)
	struct symtab **sp;
	char *namep;
{
	register struct symtab *np;
	*sp = np = lookup(namep, 1);
	np -> value = np -> name;
}

stsym(namep, valp)
char *namep, *valp;
{
	register struct symtab *np;

	np = lookup(namep, 1);
	np->value = valp;
}

error(s, x)
	char *s;
{
	FILE *efout;
	efout = fout==stdout ? stderr : stdout;
	if (fnames[ifno][0])
		fprintf(efout,"%s: %d: ", fnames[ifno], lineno[ifno]);
	fprintf(efout, s, x);
	putc('\n',efout);
	exfail++;
}
errback(s,x)
	char *s;
{
	lineno[ifno]--;
	error(s,x);
	lineno[ifno]++;
}

sch(c)
{
	register char *rlp;

	rlp = lp;
	if (rlp==line+LINELEN-2)
		error("Line overflow", 0);
	*rlp++ = c;
	if (rlp>line+LINELEN-1)
		rlp = line+LINELEN-1;
	lp = rlp;
}

savch(c)
{
	*stringbuf++ = c;
	if (stringbuf-sbf < SBSIZE)
		return;
	error("Too much defining", 0);
	exit(exfail);
}

getch()
{
	register int c, lastst;

	while ((c=getc1())=='/'  && !instring)
		{
		if ((c=getc1())!='*')
			{
			pushback(c);
			return('/');
			}
		if (!skipcom)
			{putc('/',fout); putc('*', fout);}
		lastst=0;
		while ( (c = getc1()) != '\0')
			{
			if (lastst && c=='/')
				{
				if (!skipcom)
					putc('/', fout);
				break;
				}
			if (c=='\n' || !skipcom)
				putc(c, fout);
			lastst = (c=='*');
			}
		if (c=='\0')break;
		}
	return(c);
}
char pushbuff[EXPSIZE];
char *pushp pushbuff;
pushback(c)
	{
	*++pushp = c;
	if (pushp>pushbuff+EXPSIZE) {
		error("too much backup", 0);
		exit(8);
		}
	}

getc1()
{
	register c;

	if (*pushp !=0)
		return(*pushp--);
	depth=0;
	if ((c = getc(fin)) == EOF && ifno>0) {
		fclose(fin);
		fin = fins[--ifno];
		if (pflag==0) fprintf(fout, "\n# %d \"%s\"\n",lineno[ifno], fnames[ifno]);
		c = getc1();
		if (c=='\n') lineno[ifno]--;
	}
	if (c==EOF)
		return(0);
	if (c=='\n' )
		lineno[ifno]++;
	return(c);
}

struct symtab *
lookup(namep, enterf)
char *namep;
{
	register char *np, *snp;
	register struct symtab *sp;
	int i, c, around;
	np = namep;
	snp = np+8;
	around = i = 0;
	while ( (c = *np++ ) && (np-snp)<0)
		{
		i =+ c;
		}
	i =% symsiz;
	sp = &symtab[i];
	while (sp->name[0]) {
		if (sp->name[0] != DROP)
		   {
		   snp = sp->name;
		   np = namep;
		   while (*snp++ == *np)
			   if (*np++ == '\0' || np==namep+8) {
				   if (enterf==DROP)
					   {
					   sp->name[0]= DROP;
					   return(sp);
					   }
				   if (!enterf)
					   subst(namep, sp);
				   return(sp);
			   }
		   }
		if (++sp >= &symtab[symsiz])
			if (around++)
				{
				error("too many defines", 0);
				exit(exfail);
				}
			else
			sp = symtab;
	}
	if (enterf>0) {
		snp = namep;
		for (np = &sp->name[0]; np < &sp->name[8];)
			if (*np++ = *snp)
				snp++;
	}
	return(sp);
}
char revbuff[200], *bp;
backsch(c)
	{
	if (bp-revbuff > 200)
		error("Excessive define looping", bp--);
	*bp++ = c;
	}

subst(np, sp)
char *np;
struct symtab *sp;
{
	register char *vp;
	int macflg;

	lp = np;
	bp = revbuff;
	if (depth++>100)
		{
		error("define recursion loop on %s", np);
		return;
		}
	if ((vp = sp->value) == 0)
		return;
	macflg= (*vp == '(');
	/* arrange that define unix unix still
	has no effect, avoiding rescanning */
	while (blank(*vp))
		vp++;
	if (strcmp(sp->name,vp) == SAME)
		{
		while (*vp)
			sch(*vp++);
		return;
		}
	if (macflg)
		expdef(vp);
	else
		while (*vp)
			backsch(*vp++);
	while (bp>revbuff)
		pushback(*--bp);
}




char *
copy(as)
char as[];
{
	register char *otsp, *s;
	int i;

	otsp = tsp;
	s = as;
	while(*tsp++ = *s++);
	if (tsp >tsa+CHSPACE)
		{
# if unix
		tsp = tsa = i = calloc(CHSPACE+50,sizeof(char));
		if (i== NULL)
# endif
			{
			error("no space for file names", 0);
			exit(8);
			}
		}
	return(otsp);
}


expdef(proto)
  char *proto;
{
char buffer[EXPSIZE], *parg[20], *pval[20], name[20], *cspace, *wp;
char protcop[EXPSIZE], *pr;
int narg, k, c;
pr = protcop;
while (*pr++ = *proto++)
	if (pr>=protcop+EXPSIZE){
		error("define prototype too big", 0);
		exit(8);
		}
proto= protcop;
for (narg=0; (parg[narg] = token(&proto)) != 0; narg++)
	;
/* now scan input */
cspace = buffer;
while ((c=getch()) == ' ');
if (c != '(')
	{
	error("defined function requires arguments", 0);
	return;
	}
pushback(c);
for(k=0; pval[k] = coptok(&cspace, buffer+EXPSIZE); k++);
if (k!=narg)
 {
  error("define argument mismatch");
  return;
 }
while (c= *proto++)
   {
   if (!letter(c))
      backsch(c);
   else
      {
      wp = name;
      *wp++ = c;
      while (letnum(*proto))
        *wp++ = *proto++;
      *wp = 0;
      for (k=0; k<narg; k++)
      if(strcmp(name,parg[k]) == SAME)
        break;
      wp = k <narg ? pval[k] : name;
      while (*wp) backsch(*wp++);
      }
   }
}

char *
token(cpp) char **cpp;
{
char *val;
int stc;
stc = **cpp;
*(*cpp)++ = '\0';
if (stc==')') return(0);
while (**cpp == ' ') (*cpp)++;
for (val = *cpp; (stc= **cpp) != ',' && stc!= ')'; (*cpp)++)
  {
  if (!letnum(stc) || (val == *cpp && !letter(stc)))
    {
    error("define prototype argument error");
    return(0);
    }
  }
return(val);
}

char *
coptok (cpp, clim) char **cpp, *clim;
{
	char *val;
	int stc, stop,paren;
paren = stop = 0;
val = *cpp;
if (getch() == ')')
	return(0);
while (((stc = getch()) != ',' && stc != ')' ) || paren > 0 || stop >0)
	{
	if (stc == '\0')
		{
		error("non terminated macro call", 0);
		val = 0;
		break;
		}
	if (stop == 0 && (stc == '"' || stc == '\''))
		stop = stc;
	else if (stc==stop)
		stop=0;
	if ( stc == '\\')
		{
		stc = getch();
		if (stop>0 || (stc != ',' && stc != '\\'))
			*(*cpp)++ = '\\';
		*(*cpp)++ = stc;
		}
	else
		{
		*(*cpp)++ = stc;
		if (stop==0)
			{
			if (stc == '(')
				paren++;
			if (stc == ')')
				paren--;
			}
		}
	if (*cpp >= clim)
		{
		error("define argument too long",0);
		exit(8);
		}
	}
*(*cpp)++ = 0;
pushback(stc);
return(val);
}
letter(c)
{
if (isalpha(c) || c == '_')
    return (1);
else
    return(0);
}
letnum(c)
{
if (letter(c) || isdigit(c))
  return(1);
else
  return(0);
}


blank(c)
{
return(c==' ' || c== '\t');
}

char *
strdex(s,c)
	char *s;
{
while (*s)
	if (*s==c)
		return(s);
	else
		s++;
return(0);
}
# if tgp
# define MAXOUT 80
checklen(sln)
	char *sln;
{
/* for tgp: scans string sln, and puts in newlines for blanks,
   where it likes, but to make lines less than MAXOUT chars long */

	char *p, *s, *st;
	int stopc, back, ifdone, c;
st=s=sln;
ifdone=p=stopc=back=0;
while (c= *s++)
	{
	if (c == '\\')
		back=2;
	if (back==0)
		{
		if (stopc== c)
			stopc=0;
		else
		if (c == '"' || c == '\'')
			stopc= c;
		}
	if (back>0)back--;
	if (s-st >MAXOUT && p != 0)
		{
		st=p;
		*p= '\n';
		ifdone=1;
		}

	if (stopc==0 && back==0)
		if (c==' ') p=s-1;;
	}
return(ifdone);
}
# endif

























main(argc,argv) char *argv[]; {
	exit(mainpp (argc,argv) );
	}
