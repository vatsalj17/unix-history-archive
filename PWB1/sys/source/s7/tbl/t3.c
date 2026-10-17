 /* t3.c: interpret commands affecting whole table */
# include "t..c"
int linsize;
struct optstr {char *optnam; int *optadd;} options [] {
	"expand", &expflg,
	"EXPAND", &expflg,
	"center", &ctrflg,
	"CENTER", &ctrflg,
	"box", &boxflg,
	"BOX", &boxflg,
	"allbox", &allflg,
	"ALLBOX", &allflg,
	"doublebox", &dboxflg,
	"DOUBLEBOX", &dboxflg,
	"frame", &boxflg,
	"FRAME", &boxflg,
	"doubleframe", &dboxflg,
	"DOUBLEFRAME", &dboxflg,
	"tab", &tab,
	"TAB", &tab,
	"linesize", &linsize,
	"LINESIZE", &linsize,
	0,0};
getcomm()
{
char line[200], *cp, nb[25], *t;
struct optstr *lp;
int c, ci;
for(lp= options; lp->optnam; lp++)
	*(lp->optadd) = 0;
texname = texstr[texct=0];
tab = '\t';
printf(".nr %d \\n(.s\n", LSIZE);
gets1(line);
/* see if this is a command line */
if (index(line,';') < 0)
	{
	backrest(line);
	return;
	}
for(cp=line; (c = *cp) != ';'; cp++)
	{
	if (!letter(c)) continue;
	for(lp= options; lp->optadd; lp++)
		{
		if (prefix(lp->optnam, cp))
			{
			*(lp->optadd) = 1;
			while (letter(*cp)) cp++;
			if (lp->optadd == &tab)
				{
				while (*cp==' ')cp++;
				if (*cp == '('){
					*(lp->optadd) = *++cp;
					cp++;
					}
				}
			if (lp->optadd == &linsize)
				{
				while (*cp==' ')cp++;
				if (*cp== '('){
					for(t=nb; (ci= *++cp) != ')';)
						*t++ = ci;
					*t = 0;
					printf(".nr %d %s\n",LSIZE,nb);
					}
				}
			break;
			}
		}
	if (letter(*cp)) error("Illegal option");
	cp--;
	}
cp++;
backrest(cp);
return;
}
backrest(cp)
	char *cp;
{
char *s;
for(s=cp; *s; s++);
un1getc('\n');
while (s>cp)
	un1getc(*--s);
return;
}
