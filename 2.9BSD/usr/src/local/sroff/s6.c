#include "sdef.h"
#include "d.h"
#include "v.h"
#include "tw.h"
/*
sroff6.c

width functions, sizes and fonts
*/

extern int eschar;
extern int widthp;
extern int ohc;
extern int xfont;
extern int setwdf;
extern char trtab[];
extern int chbits;
extern int nonumb;
extern int noscale;
extern int font;
extern int font1;
extern int ulfont;
extern int space;
extern int sps;
extern int nlflg;
extern int nform;
extern int dfact;
extern int dfactd;
extern int lss;
extern int lss1;
extern int vflag;
extern int ch0;
extern int level;
extern int ch;
extern int xxx;
extern int bdtab[];
extern int sbold;
extern int smnt;
extern int ul;
extern int cu;
extern int fontab[PFONT][0200-040];
extern int fontabS[];
extern char *codetab[];
extern int Em;

width(c)
int c;
{
	register i,j,k;

	j = c;
	k = 0;
	if(j & MOT){
		if(j & VMOT)goto rtn;
		k = j & ~MOTV;
		if(j & NMOT)k = -k;
		goto rtn;
	}
	if((i = (j & CMASK)) == 010){
		k = -widthp;
		goto rtn;
	}
	if(i == PRESC)i = eschar;
	if((i == ohc) ||
	   (i >= MAXCHAR))goto rtn;
	if(j & ZBIT)goto rtn;
	i = trtab[i] & BMASK;
	if(i < 040)goto rtn;
		/* stan, if EBIT is on pass character without counting it */
	if (!(j & EBIT)) 
		k = getcw((j & ~CMASK) | i);
	widthp = k;
rtn:
	return(k);
}
getcw(i)
int i;
{
	register ft, k;
	register struct fonts *fp;

	fp = &fonts[getfont(i)];
	ft = fp->phys;
	k = i & CMASK;

	if (k>0177)
		k = fontabS[k-0200] & WDMASK;
	else if (ft & PROPOR)
		k = fontab[ft&~PROPOR][k-32] & WDMASK;
	else k = fp->Char;
	return(k);
}
setch(){
	register i,*j,k;
	extern int chtab[];

	if((i = getrq()) == 0)return(0);
	for(j=chtab;*j != i;j++)if(*(j++) == 0)return(0);
	k = *(++j) | chbits;
	return(k);
}
findft(i)
int i;
{
	register k;

#if NFONTS > 8
	k = (i & BMASK) - '0';
	if (i & ~BMASK)
		k += (((i>>8) & BMASK) - '0') * 10;
#else
	k = i - '0';
#endif
	if((k >= 1) && (k <= NFONTS))return(--k);
	for(k=0; k < NFONTS; k++)
		if (fonts[k].lab == i) return(k);
	return(-1);
}
mchbits(){
	if (font == ulfont)
		chbits = 0|ULMODE;	/* use font 0 */
	else chbits = (font << FONTSHFT);
	space = ' ' | chbits;
	if (ul) chbits |= ULMODE;
	if (cu) space |= ULMODE;
	sps = width(space);
	Em = fonts[font].Em;
}
setps(){
	register i,j;

	if((((i=getch() & CMASK) == '+')  || (i == '-')) &&
	  (((j=(ch = getch() & CMASK) - '0') >= 0) && (j <= 9))){
		ch = 0;
		return;
	}
	if((i -= '0') == 0){
		return;
	}
	if((i > 0) && (i <= 9)){
		if((i <= 3) &&
		  ((j=(ch = getch() & CMASK) - '0') >= 0) && (j <= 9)){
			ch = 0;
		}
	}
}
caseft(){
	skip();
	setfont(1);
}
setfont(a)
int a;
{
	register i,j;

	if(a)i = getrq();
		else i = getsn();
	if(!i || (i == 'P')){
		j = font1;
		goto s0;
	}
	if((j = findft(i))  == -1)return;
s0:
	font1 = font;
	font = j;
	mchbits();
}
setwd(){
	register i, base, wid;
	int delim, em, k;
	int savlevel, savhp, savfont, savfont1;

	base = v.st = v.sb = wid = v.ct = 0;
	if((delim = getch() & CMASK) & MOT)return;
	savhp = v.hp;
	savlevel = level;
	v.hp = level = 0;
	savfont = font;
	savfont1 = font1;
	setwdf++;
	while((((i = getch()) & CMASK) != delim) && !nlflg){
		wid += width(i);
		if(!(i & MOT)){
			em = 2*t.Halfline;
		}else if(i & VMOT){
			k = i & ~MOTV;
			if(i & NMOT)k = -k;
			base -= k;
			em = 0;
		}else continue;
		if(base < v.sb)v.sb = base;
		if((k=base + em) > v.st)v.st = k;
	}
	nform = 0;
	setn1(wid);
	v.hp = savhp;
	level = savlevel;
	font = savfont;
	font1 = savfont1;
	mchbits();
	setwdf = 0;
}
vmot(){
	dfact = lss;
	vflag++;
	return(mot());
}
hmot(){
	dfact = Em;
	return(mot());
}
mot(){
	register i, j;

	getch(); /*eat delim*/
	if(i = atoi())
		i = makem(i);
	getch();
	vflag = 0;
	dfact = 1;
	return(i);
}
sethl(k)
int k;
{
	register i;

	i = Em/2;
	if(k == 'u')i = -i;
	else if(k == 'r')i = -Em;
	vflag++;
	i = makem(i);
	vflag = 0;
	return(i);
}
makem(i)
int i;
{
	register j;

	if((j = i) < 0)j = -j;
	j = (j & ~MOTV) | MOT;
	if(i < 0)j |= NMOT;
	if(vflag)j |= VMOT;
	return(j);
}
casefp(){
	register i, j;

	skip();
	if(((i = (getch() & CMASK) - '0' -1) < 0) || (i >NFONTS))return;
	if(skip() || !(j = getrq()))return;
	fonts[i].lab = j;
}
casevs(){
	register i;

	skip();
	vflag++;
	dfact = INCH; /*default scaling is points!*/
	dfactd = 72;
	i = inumb(&lss);
	if(nonumb)i = lss1;
	if(i < VERT)i = VERT;
	lss1 = lss;
	lss = i;
}
xlss(){
	register i, j;

	getch();
	dfact = lss;
	i = atoi();
	dfact = 1;
	getch();
	if((j = i) < 0)j = -j;
	ch0 = ((j & 03700)<<3) | HX;
	if(i < 0)ch0 |= 040000;
	return(((j & 077)<<9) | LX);
}
casefz(){}
caseps(){}
caselg(){}
casecs(){}
casebd(){
	register i, j, k;

	if(skip() || !(i = getrq()) ||
	  ((j = findft(i))  == -1))
		return;
	if (j == smnt) {
		if(skip() || !(i = getrq()) ||
		    ((k = findft(i))  == -1)) {
			bdtab[smnt] = 0;
			return;
		}
		sbold = k;
	}
	skip();
	noscale++;
	bdtab[j] = atoi();
	noscale = 0;
}
casess(){}
getlg(i)
int i;
{
	return(i);
}
