#include "tdef.h"
#include "t.h"
#include "tw.h"
/*
nroff10.c

Device interfaces
*/

extern int lss;
extern char obuf[];
extern char *obufp;
extern int xfont;
extern int esc;
extern int lead;
extern int *dip;
extern int oline[];
extern int *olinep;
extern int ulfont;
extern int esct;
extern int sps;
extern int ics;
extern int Hinc, Vinc;
extern int ttysave;
extern int ttys[3];
extern char termtab[];
extern int ptid;
extern int waitf;
extern int pipeflg;
extern int eqflg;
extern int xxx;
int bdmode;
int plotmode;

ptinit(){
	register i;
	register char **p, *q;
	char *x[8];

	if((i = open(termtab,0)) < 0){
		prstr("Cannot open ");
		prstr(termtab);
		prstr("\n");
		exit(-1);
	}
	read(i,x,sizeof x);
	read(i,&t.bset,q = 4*(&t.zzz - &t.bset));		/***/
	x[2] =- q;
	q = setbrk(x[2]);
	seek(i,t.twinit+040,0);		/***/
	i = read(i,q,x[2]);
	q =- t.twinit;
	for(p = &t.twinit; p < &t.zzz; p++){
		if(*p)*p =+ q;else *p = &t.zzz;
	}
	sps = EM;
	ics = EM*2;
	if(t.bset || t.breset){
		ttys[2] =& ~t.breset;
		ttys[2] =| t.bset;
		stty(1,ttys);
	}
	oputs(t.twinit);
	if(eqflg)t.Adj = t.Hor;
}
twdone(){
	oputs(t.twrest);
	if(pipeflg){
		close(ptid);
		wait(&waitf);
	}
	ttys[2] = ttysave;
	stty(1,ttys);
}
ptout(i)
int i;
{
	*olinep++ = i;
	if(olinep >= &oline[LNSIZE])olinep--;
	if((i&CMASK) != '\n')return;
	olinep--;
	lead =+ dip->blss + lss - t.Newline;
	dip->blss = 0;
	esct = esc = 0;
	if(olinep>oline){
		move();
		ptout1();
		oputs(t.twnl);
	}else{
		lead =+ t.Newline;
		move();
	}
	lead =+ dip->alss;
	dip->alss = 0;
	olinep = oline;
}
ptout1()
{
	register i, k;
	register char *codep;
	int *q, w, j, phyw;
/***/	register nbold;

	for(q=oline; q<olinep; q++){
	if((i = *q) & MOT){
		j = i & ~MOTV;
		if(i & NMOT)j = -j;
		if(i & VMOT)lead =+ j;
		else esc =+ j;
		continue;
	}
	if((k = (i & CMASK)) <= 040){
		switch(k){
			case ' ': /*space*/
				esc =+ t.Char;
				break;
		}
		continue;
	}
	if(i&074000)xfont = (i>>9) & 03;
	nbold = 0;
/***/	if(*t.bdon){
		if(!bdmode && (xfont == 2)){
			oputs(t.bdon);
			bdmode++;
		}
		if(bdmode && (xfont != 2)){
			oputs(t.bdoff);
			bdmode = 0;
		}
/***/	} else if (xfont == 2)
/***/		nbold = 1;
/***/ dobold:
	codep = t.codetab[k-32];
	w = t.Char * (*codep++ & 0177);
	phyw = w;
	if(i&ZBIT)w = 0;
	esct =+ w;
	if(*codep && (esc || lead))move();
	if(xfont == ulfont){
		for(k=w/t.Char;k>0;k--)oput('_');
		for(k=w/t.Char;k>0;k--)oput('\b');
	}
	while(*codep != 0){
		if(*codep & 0200){
			codep = plot(codep);
			oputs(t.plotoff);
			oput(' ');
		}else{
			if(plotmode)oputs(t.plotoff);
			oput(*codep++);
		}
	}
/***/	if (nbold) {
/***/		nbold--;
/***/		for(k=phyw/t.Char;k>0;k--) oput('\b');
/***/		k = i & CMASK;
/***/		goto dobold;
/***/	}
	if(!w)for(k=phyw/t.Char;k>0;k--)oput('\b');
	}
}
plot(x)
char *x;
{
	register int i;
	register char *j, *k;

	if(!plotmode)oputs(t.ploton);
	k = x;
	if((*k & 0377) == 0200)k++;
	for(; *k; k++){
		if(*k & 0200){
			if(*k & 0100){
				if(*k & 040)j = t.up; else j = t.down;
			}else{
				if(*k & 040)j = t.left; else j = t.right;
			}
			if(!(i = *k & 037))return(++k);
			while(i--)oputs(j);
		}else oput(*k);
	}
	return(k);
}
move(){
	register k;
	register char *i, *j;
	char *p, *q;

/***/	if(esct =+ esc)i = "\0"; else i = t.twnl;
	j = t.hlf;
	p = t.right;
	q = t.down;
	if(lead){
		if(lead < 0){
			lead = -lead;
			if(!esc)i = t.flr; else i = "\0";
			j = t.hlr;
			q = t.up;
		}
/***/		if(*i){
			k = lead/t.Newline;
			lead = lead%t.Newline;
			while(k--)oputs(i);
		}
/***/		if(*j){
			k = lead/t.Halfline;
			lead = lead%t.Halfline;
			while(k--)oputs(j);
		}
	}
	if(esc){
		if(esc < 0){
			esc = -esc;
			j = "\b";
			p = t.left;
		}else j = " ";
		k = esc/t.Char;
		esc = esc%t.Char;
		while(k--)oputs(j);
	}
/***/	if((*t.ploton) && (esc || lead)){
		if(!plotmode)oputs(t.ploton);
		esc =/ t.Hor;
		lead =/ t.Vert;
		while(esc--)oputs(p);
		while(lead--)oputs(q);
		oputs(t.plotoff);
	}
	esc = lead = 0;
}
ptlead(){move();}
dostop(){
	int junk;

	flusho();
	read(2,&junk,1);
}
