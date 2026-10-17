#include "stdio.h"

#define DLEV 2

int strip();
int an();
int s();
int es();
int ily();
int CCe();
int VCe();
int bility();
int tion();
int ize();
int y_to_e();
int i_to_y();

struct suftab {
	char *suf;
	int (*p1)();
	int n1;
	char *d1;
	char *a1;
	int (*p2)();
	int n2;
	char *d2;
	char *a2;
} suftab[] {
	{"ssen",ily,4,"-y+iness","+ness" },
	{"ssel",ily,4,"-y+i+less","+less" },
	{"se",s,1,"","+s",		es,2,"-y+ies","+es" },
	{"s'",s,2,"","+'s"},
	{"s",s,1,"","+s"},
	{"ytilib",bility,5,"-le+ility",""},
	{"yti",CCe,3,"-e+ity","+ity"},
	{"ylb",y_to_e,1,"-e+y",""},
	{"yl",ily,2,"-y+ily","+ly"},
	{"laci",strip,2,"","+al"},
	{"latnem",strip,2,"","+al"},
	{"lanoi",strip,2,"","+al"},
	{"tnem",strip,4,"","+ment"},
	{"gni",CCe,3,"-e+ing","+ing"},
	{"re",strip,1,"","+r",		i_to_y,2,"-y+ier","+er"},
	{"de",strip,1,"","+d",		i_to_y,2,"-y+ied","+ed"},
	{"citsi",strip,2,"","+ic"},
	{"tse",strip,2,"","+st",	i_to_y,3,"-y+iest","+est"},
	{"cigol",i_to_y,1,"-y+ic",""},
	{"tsigol",i_to_y,2,"-y+ist",""},
	{"tsi",VCe,3,"-e+ist","+ist"},
	{"msi",VCe,3,"-e+ism","+ist"},
	{"noitacif",i_to_y,6,"-y+ication",""},
	{"noitazi",ize,5,"-e+ation",""},
	{"noit",tion,3,"-e+ion","+ion"},
	{"naino",an,3,"","+ian"},
	{"na",an,1,"","+n"},
	{"evit",tion,3,"-e+ive","+ive"},
	{"ezi",CCe,3,"-e+ize","+ize"},
	{"elbaif",i_to_y,4,"-y+iable",""},
	{"pihs",strip,4,"","+ship"},
	{"dooh",strip,4,"","+hood"},
	0
};

char *preftab[] {
	"anti",
	"inter",
	"micro",
	"mis",
	"multi",
	"non",
	"over",
	"re",
	"semi",
	"sub",
	"super",
	"under",
	"un",
	0
};

int numb;
char word[100];
char *deriv[40];

main()
{
	register char *ep, *cp;
	for(;;) {
		for(ep=word;(*ep=getchar())!='\n';ep++)
			if(*ep<=0)
				exit(0);
		numb++;
		for(cp=word;cp<ep;cp++)
			if('a'<=*cp&&*cp<='z')
				goto lcase;
		putsuf(ep,".",0);
		for(cp=word+1;cp<ep;cp++)
			if('A'<=*cp&&*cp<='Z')
				*cp =| 040;
lcase:
		putsuf(ep,".",0);
		suffix(ep,0);
	}
}

suffix(ep,lev)
char *ep;
{
	register struct suftab *t;
	register char *cp, *sp;
	lev =+ DLEV;
	deriv[lev] = deriv[lev-1] = 0;
	for(t= &suftab[0];sp=t->suf;t++) {
		cp = ep;
		while(*sp)
			if(*--cp!=*sp++)
				goto next;
		for(sp=cp; --sp>=word&&!vowel(*sp|040); ) ;
		if(sp<word)
			return;
		(*t->p1)(ep-t->n1,t->d1,t->a1,lev+1);
		if(t->p2!=0) {
			deriv[lev] = deriv[lev+1] = 0;
			(*t->p2)(ep-t->n2,t->d2,t->a2,lev);
		}
		return;
next:		;
	}
}

strip(ep,d,a,lev)
char *ep,*d,*a;
{
	putsuf(ep,a,lev);
	suffix(ep,lev);
}

s(ep,d,a,lev)
char *ep,*d,*a;
{
	if(lev>DLEV+1)
		return;
	strip(ep,d,a,lev);
}

an(ep,d,a,lev)
char *ep,*d,*a;
{
	if((*word&0140)!=0100)	/*must be proper name*/
		return;
	putsuf(ep,a,lev);
}

ize(ep,d,a,lev)
char *ep,*d,*a;
{
	*ep++ = 'e';
	strip(ep,"",d,lev);
}

y_to_e(ep,d,a,lev)
char *ep,*d,*a;
{
	*ep++ = 'e';
	strip(ep,"",d,lev);
}

ily(ep,d,a,lev)
char *ep,*d,*a;
{
	if(ep[-1]=='i')
		i_to_y(ep,d,a,lev);
	else
		strip(ep,d,a,lev);
}

bility(ep,d,a,lev)
char *ep,*d,*a;
{
	*ep++ = 'l';
	y_to_e(ep,d,a,lev);
}

i_to_y(ep,d,a,lev)
char *ep,*d,*a;
{
	if(ep[-1]=='i') {
		ep[-1] = 'y';
		a = d;
	}
	strip(ep,"",a,lev);
}

es(ep,d,a,lev)
char *ep,*d,*a;
{
	if(lev>DLEV)
		return;
	switch(ep[-1]) {
	default:
		return;
	case 'i':
		i_to_y(ep,d,a,lev);
		break;
	case 's':
	case 'h':
	case 'z':
	case 'x':
		strip(ep,d,a,lev);
	}
}

tion(ep,d,a,lev)
char *ep,*d,*a;
{
	switch(ep[-2]) {
	case 'c':
	case 'r':
		putsuf(ep,a,lev);
		break;
	case 'a':
		y_to_e(ep,d,a,lev);
	}
}

/*	possible consonant-consonant-e ending*/
CCe(ep,d,a,lev)
char *ep,*d,*a;
{
	switch(ep[-1]) {
	case 'l':
		if(vowel(ep[-2]))
			break;
		switch(ep[-2]) {
		case 'l':
		case 'r':
		case 'w':
			break;
		default:
			y_to_e(ep,d,a,lev);
			return;
		}
		break;
	case 's':
		if(ep[-2]=='s')
			break;
	case 'c':
	case 'g':
	case 'v':
	case 'z':
		if(vowel(ep[-2]))
			break;
	case 'u':
		y_to_e(ep,d,a,lev);
		if(!(ep[-2]=='n'&&ep[-1]=='g'))
			return;
	}
	VCe(ep,d,a,lev);
}

/*	possible consonant-vowel-consonant-e ending*/
VCe(ep,d,a,lev)
char *ep,*d,*a;
{
	char c;
	c = ep[-1];
	if(!vowel(c) && vowel(ep[-2])) {
		c = *ep;
		*ep++ = 'e';
		putsuf(ep,d,lev);
		suffix(ep,lev);
		ep--;
		*ep = c;
	}
	strip(ep,d,a,lev);
}

char *lookuppref(wp,ep)
char **wp;
char *ep;
{
	register char **sp;
	register char *bp,*cp;
	for(sp=preftab;*sp;sp++) {
		bp = *wp;
		cp = *sp;
		while(*cp)
			if((*bp++|040)!=*cp++)
				goto next;
		for(cp=bp;cp<ep;cp++) 
			if(vowel(*cp)) {
				*wp = bp;
				return(*sp);
			}
next:	;
	}
	return(0);
}

putsuf(ep,a,lev)
char *ep,*a;
{
	register char *cp;
	char *bp;
	register char *pp;
	char space[20];
	deriv[lev] = a;
	putw(word,ep,lev);
	bp = word;
	pp = space;
	deriv[lev+1] = pp;
	while(cp=lookuppref(&bp,ep)) {
		*pp++ = '+';
		while(*pp = *cp++)
			pp++;
		putw(bp,ep,lev+1);
	}
	deriv[lev+1] = deriv[lev+2] = 0;
}

putw(bp,ep,lev)
char *bp,*ep;
{
	int i;
	if(ep-bp<=1)
		return;
	for( ;bp<ep;bp++)
		putchar(*bp);
	putchar('\t');
	i = lev;
	do {
		if(deriv[i])
			printf("%s",deriv[i]);
	} while(--i>0);
	printf("\t%05d\t%d\n",numb,lev);
}

vowel(c)
{
	switch(c) {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	case 'y':
		return(1);
	}
	return(0);
}
