#
#ifndef unsw_orig

#endif
/*

	C debugger

*/

#include <param.h>
#include <user.h>
#ifndef unsw_orig
#include "cdb.h"
#endif

#ifdef unsw_orig
#define	DSP	0
#define	ISP	1
#define	NBKP	10
#define	SYMSIZ	12*400
#define	BADJST	01

int	fcore;
int	errno;
int	fsym;
int	symoff;
char	*lp;
int	errflg;
int	symlen;
int	symct;
int	symcor;
int	symbuf[SYMSIZ];
int	*symptr;
struct {
	int	loc;
	int	ins;
	int	count;
	int	flag;
} bkptl[NBKP];
int	lastbp;
char	symbol[8];
int	symflg;
int	symval;
char	tsym[8];
char	fsymbol[10];
char	ssymbol[8];
int	ssymflg;
int	ssymval;
int	signo;
char	line[128];
int	regbuf[512];
#endif
char	**uregs	&regbuf[512];
#ifdef unsw_orig
char	*rtsize;
int	loccsv;
int	locsr5;
#define	RUSER	1
#define	RIUSER	2
#define	WUSER	4
#define	RUREGS	3
#define	WUREGS	6
#define	SETTRC	0
#define	CONTIN	7
#define	EXIT	8

#define	ps	-1
#define	pc	-2
#define	sp	-6
#define	r5	-9
#define	r4	-10
#define	r3	-11
#define	r2	-12
#define	r1	-5
#define	r0	-3
#endif
struct reglist {
	char	*rname;
	int	roffs;
} reglist[] {
	"ps", ps,
	"pc", pc,
	"sp", sp,
	"r5", r5,
	"r4", r4,
	"r3", r3,
	"r2", r2,
	"r1", r1,
	"r0", r0,
};
#ifdef unsw_orig

struct sfregs {
	int	junk[2];
	int	fpsr;
	float	sfr[6];
};

struct lfregs {
	int	junk[2];
	int	fpsr;
	double	lfr[6];
};

#endif
int	frnames[] { 0, 3, 4, 5, 1, 2 };
#ifdef unsw_orig
int	dot;
int	tdot;
#endif
int	dotinc 2;
int	lastcom '/';
int	lastype	'o';
#ifdef unsw_orig
int	modifier;
#endif
char	*symfil	"a.out";
char	*corfil	"core";
#ifndef unsw_orig
char	ttnm[]	"/dev/tty ";
int	arg	-1;
int	ssgrp	-1;
#endif
#ifdef unsw_orig
int	callist[50];
int	entpt[50];
int	callev;
int	pid;
int	adrflg;
int	idsep;
#endif
char	*signals[] {
	"",
	"Hangup",
	"Interrupt",
	"Quit",
	"Illegal instruction",
	"Trace/BTP",
	"IOT",
	"EMT",
	"Floating exception",
	"Killed",
	"Bus error",
	"Memory fault",
	"Bad system call",
	"",
	"",
	"",
};

#ifndef unsw_orig
main(argc,argv)
#endif
#ifdef unsw_orig
main(argc, argv)
#endif
char **argv;
{
#ifndef unsw_orig
	char ttyn();
#endif
	int onintr();

	if (argc>1)
		symfil = argv[1];
	if (argc>2)
		corfil = argv[2];
	fcore = open(corfil, 0);
#ifndef unsw_orig
	if ((fsym = open(symfil, 2)) < 0) {
#endif
#ifdef unsw_orig
	if ((fsym = open(symfil, 0)) < 0) {
#endif
		printf("%s not found\n", symfil);
		return;
	}
#ifndef unsw_orig
	fcore = fcore<0? fsym:fcore;
#endif
	read(fsym, regbuf, 020);
	if (regbuf[0]==0411)			/* I/D separated */
		idsep++;
	else if (regbuf[0]!=0410 && regbuf[0]!=0407) {	/* magic */
		printf("Bad format: %s\n", symfil);
		return;
	}
	symoff = regbuf[1] + regbuf[2];
	symlen = regbuf[4];
	if (regbuf[7] != 1)
		symoff =<< 1;
	symoff =+ 020;
	seek(fsym, symoff, 0);
	symcor = read(fsym, symbuf, sizeof symbuf);
	if (symcor>0)
		symoff =+ symcor;
	symcor =>> 1;
	read(fcore, regbuf, 1024);
	signo = regbuf[0].u_arg[0]&017;
	regbuf->u_tsize =<< 6;
	regbuf->u_dsize =<< 6;
	regbuf->u_ssize =<< 6;
	rtsize = (regbuf->u_tsize+017777) & ~017777;
	if (symlook("csv\0\0\0\0"))
		loccsv = ssymval;
	if (symlook("savr5\0\0\0"))
		locsr5 = ssymval;
#ifndef unsw_orig
	ttnm[8] = ttyn(0);
#endif
	setstack();
	signal(SIGINS, onintr);
	setexit();
	signal(SIGINT, onintr);
loop:
	if (errflg) {
		printf("?\n");
		errflg = 0;
	}
	lp = line;
	while ((*lp = getchar()) != '\n')
		if (*lp++ == '\0') {
#ifndef unsw_orig
			if (autof) {
				offauto();
				reset();
			} else {
				if (pid)
					ptrace(EXIT, pid, 0, 0);
				return;
			}
#endif
#ifdef unsw_orig
			if (pid)
				ptrace(EXIT, pid, 0, 0);
			return;
#endif
		}
	lp = line;
	command();
	goto loop;
}

command()
{
#ifndef unsw_orig
	register n,q;
#endif
#ifdef unsw_orig
	register n;
#endif

#ifndef unsw_orig
	if (*lp=='#') {
		litflg++;
		lp++;
	}
	q = lastcom;
	if (lastcom=='!') lastcom = '/';
#endif
	adrflg = expr();
	if (errflg)
		return;
	n = getcnt();
	if (lastcom=='$')
		lastcom = '/';
	if (*lp == '\n') {
		if (!adrflg)
			dot =+ dotinc;
	} else
		lastcom = *lp++;
	modifier = 0;
#ifndef unsw_orig
	if (lastcom=='!') {
		expr();
		if (errflg)
			return;
		pokeval = ssymval;
	}

	/* change made at CAC for backwards typeout 9-76 */

	if (lastcom == '^') {
		if (!adrflg) {
			switch(q) {

			case '/':
			case '?':
			case '\'':
			case '\\':
			case '&':
				lastcom = q;
				dot =- dotinc;
				break;

			default:
				errflg++;
				return;
			}
		} else {
			errflg++;
			return;
		}
	}
#endif
	if (*lp != '\n')
		modifier = *lp++;
#ifndef unsw_orig
	if (*lp != '\n')
		arg = *lp++ - '0';
	if (lastcom=='%') {
		switch(modifier) {

		case 'r':
			runcom();
			return;

		case 'a':
			autocom();
			return;

		case 'g':
			gcom(arg);
			arg = -1;
			return;

		case 's':
			while(n--) {
				runcom();
				arg = -1;
			}
			return;

		case 'p':
			grprnt(arg);
			arg = -1;
			return;
		}
#endif
#ifdef unsw_orig
	if (lastcom=='%' && modifier=='r') {
		runcom();
		return;
#endif
	}
	if (*lp != '\n') {
		errflg++;
		return;
	}
#ifndef unsw_orig
	if (adrflg && lastcom != '!')
#endif
#ifdef unsw_orig
	if (adrflg)
#endif
		dot = tdot;
	while(n) {
		scommand(n);
		if (errflg)
			return;
		if (--n)
			dot =+ dotinc;
	}
}

scommand(n)
{
	register w, c;
	double fw;
	struct { int i[4]; };
	int onintr();
#ifndef unsw_orig
	char *a,*b;
#endif

	switch(lastcom) {

	case '/':
		w = cget(dot, DSP);
		if (modifier)
			lastype = modifier;
		switch(lastype) {

		case 'o':
			printf("%.1o\n", w);
			dotinc = 2;
			return;

		case 'i':
			printf("%d\n", w);
			dotinc = 2;
			return;

		case 'f':
			fw = 0;
			fw.i[0] = w;
			fw.i[1] = cget(dot+2, DSP);
			printf("%e\n", fw);
			dotinc = 4;
			return;

		case 'd':
			fw.i[0] = w;
			fw.i[1] = cget(dot+2, DSP);
			fw.i[2] = cget(dot+4, DSP);
			fw.i[3] = cget(dot+6, DSP);
			printf("%e\n", fw);
			dotinc = 8;
			return;
		}
		errflg++;
		return;

	case '\\':
		printf("%.1o\n", cget(dot, DSP)&0377);
		dotinc = 1;
		return;

	case '=':
		printf("%.1o\n", dot);
		return;

	case '\'':
		printc(cget(dot, DSP) & 0377);
		if (n<=1)
			putchar('\n');
		dotinc = 1;
		return;

	case '"':
		w = cget(dot, DSP);
		while(c = cget(w++, DSP)&0377)
			printc(c);
#ifndef unsw_orig
		printf("\n");
#endif
#ifdef unsw_orig
		putchar('\n');
#endif
		return;

#ifndef unsw_orig
	case '*':
		dot = cget(dot,DSP);
		lastcom = '/';
		scommand(1);
		return;

#endif
	case '&':
		psymoff(cget(dot, DSP), 0100000);
		printf("\n");
		return;

	case '$':
		printf("%s\n", signals[signo]);
		printtrace();
		return;

	case '?':
		printins(0);
		printf("\n");
		return;

	case '%':
		runcom();
		signal(SIGINT, onintr);
		return;

#ifndef unsw_orig
	case '!':
		a = dot;
		b = regbuf->u_tsize;
		cput(dot,(a < b) && idsep?ISP:DSP,pokeval);
		return;

#endif
	}
	errflg++;
}

getcnt()
{
	register t1, t2;

	if (*lp != ',')
		return(1);
	lp++;
	t1 = tdot;
	if (expr() == 0) {
		tdot = t1;
		return(1);
	}
	t2 = tdot;
	tdot = t1;
	return(t2);
}

cget(n, space)
{
	register w;

	w = get(n, space);
	if (errflg)
		reset();
	return(w);
}

#ifndef unsw_orig
cput(n,space,w)
{
	put(n,space,w);
	if (errflg)
		reset();
	return;
}

#endif
printc(c)
{
	if (c<' ' || c>'~')
		printf("\\%o", c);
	else
#ifndef unsw_orig
		putchar(c);
#endif
#ifdef unsw_orig
		printf("%c", c);
#endif
}

expr()
{
	int i, t1, t2, donef, lastop, b;

	tdot = 0;
	adrflg = 0;
	lastop = '+';
	ssymval = 0;
	donef = 0;
loop:
#ifndef unsw_orig
	if (*lp==':') {
		lp++;
		fsymbol[0] = '~';
	} else
		fsymbol[0] = 0;
#endif
#ifdef unsw_orig
	fsymbol[0] = 0;
#endif
	if (symchar(0)) {
		adrflg++;
#ifndef unsw_orig
		symcollect(fsymbol[0]=='~'?0:'_');
#endif
#ifdef unsw_orig
		symcollect('_');
#endif
		if (*lp++==':' && symchar(0)) {
			for (i=0; i<8; i++)
				fsymbol[i] = tsym[i];
			fsymbol[0] = '~';
			symcollect(0);
		} else 
			lp--;
		if (symlook(tsym) == 0) {
			errflg++;
#ifndef unsw_orig
		if (modifier=='g')
			return(0);
		else
#endif
			reset();
		}
		goto loop;
	}
	if (*lp>='0' && *lp<='9') {
		adrflg++;
		ssymval = 0;
		if (*lp == '0')
			b = 8;
		else
			b = 10;
		while (*lp>='0' && *lp<='9') {
			ssymval =* b;
			ssymval =+ *lp++ -'0';
		}
		goto loop;
	}
#ifndef unsw_orig
	if (*lp=='.') {
		lp++;
		tdot = dot;
	}
#endif
	switch (*lp) {

	default:
		donef++;

	case '+':
	case '-':
		switch(lastop) {

		case '+':
			tdot =+ ssymval;
			goto op;

		case '-':
			tdot =- ssymval;

		op:
			if (donef)
				return(adrflg);
			else
				lastop = *lp++;
		}
		goto loop;

	case ' ':
	case '\t':
		lp++;
		goto loop;

	case '[':
		lp++;
		t1 = ssymval;
		t2 = tdot;
		if (expr() == 0)
			tdot = 0;
		ssymval = cget(t1 + (tdot<<1), DSP);
		tdot = t2;
		if (*lp == ']')
			lp++;
		goto loop;
	}
}

symcollect(c)
{
	register char *p;

	p = tsym;
#ifndef unsw_orig
	if (c && !litflg)
#endif
#ifdef unsw_orig
	if (c)
#endif
		*p++ = c;
	while (symchar(1)) {
		if (p < &tsym[8])
			*p++ = *lp;
		lp++;
	}
	while (p < &tsym[8])
		*p++ = 0;
#ifndef unsw_orig
	litflg = 0;
#endif
}

symchar(dig)
{
	if (*lp>='a'&&*lp<='z' || *lp=='_')
		return(1);
	if (dig && *lp>='0' && *lp<='9')
		return(1);
	return(0);
}

#ifndef unsw_orig
autocom()
{
	char	*ptr;

	while (!symchar(1) && *lp != '\n')
		lp++;
	ptr = lp;
	ptr--;
	while(*ptr++ != '\n');
	*--ptr = '\0';
	close(0);
	if (open(lp,0) < 0) {
		errflg++;
		open(ttnm,0);
		return;
	}
	autof++;
	reset();
}

offauto()
{
	close(0);
	open(ttnm,0);
	autof = 0;
}

#endif
error()
{
	errflg++;
	reset();
}

printtrace()
{
	int tpc, tr5, narg, argp, i;

	if (modifier=='r') {
		printregs();
		return;
	}
	if (modifier=='f' || modifier=='d') {
		printfregs();
		return;
	}
	tpc = uregs[pc];
	tr5 = uregs[r5];
	if (locsr5)
	{
		if (narg = get(locsr5, DSP)) {
			tr5 = narg;
		}
	}
	callev = 0;
	while (errflg == 0) {
		narg = findroutine(tpc, tr5);
		printf("%2d: %.8s(", callev, ssymbol);
		if (--narg >= 0)
			printf("%.1o", get(tr5+4, DSP));
		argp = tr5+4;
		while(--narg >= 0)
			printf(",%.1o", get(argp =+ 2, DSP));
		printf(")\n");
		tpc = get(tr5+2, DSP);
		if (callev < 50) {
			entpt[callev] = ssymval;
			callist[callev++] = tr5;
		}
		if ((tr5 = get(tr5, DSP)) == 0)
			break;
	}
}

setstack()
{
	register int tpc, tr5, i;

	tpc = uregs[pc];
	tr5 = uregs[r5];
	if (locsr5)
		if (i = get(locsr5, DSP)) {
			tr5 = i;
		}
	callev = 0;
	while (errflg == 0) {
		findroutine(tpc, tr5);
		tpc = get(tr5+2, DSP);
		if (callev >= 50)
			break;
		entpt[callev] = ssymval;
		callist[callev++] = tr5;
		if ((tr5 = get(tr5, DSP)) == 0)
			break;
	}
	errflg = 0;
}

printfregs()
{
	register i;
	double f;

	printf("fpsr	%.1o\n", regbuf[0].fpsr);
	for (i=0; i<6; i++) {
		if (regbuf[0].fpsr&0200)	/* long mode */
			f = regbuf[0].lfr[frnames[i]];
		else
			f = regbuf[0].sfr[frnames[i]];
		printf("fr%d	%e\n", i, f);
	}
}

printregs()
{
	register struct reglist *p;
	register char *v, *d;

	for (p=reglist; p < &reglist[9]; p++) {
		printf("%s	%.1o", p->rname, v=uregs[p->roffs]);
		d = vallook(v);
		if (d < 010000) {
			printf("	%.8s", ssymbol);
			if (d)
				printf("+%.1o", d);
		}
		printf("\n");
	}
}

findroutine(rpc, rr5)
{
	register callpt, inst, narg;

	callpt = get(rr5+2, DSP);
	if ((inst=get(callpt-4, ISP)) == 04737)	/* jsr pc,*$... */
		narg = 1;
	else if ((inst&~077)==04700)		/* jsr pc,... */
		narg = 0;
	else {
		errflg++;
		return(0);
	}
	inst = vallook((inst==04767?callpt:0) + get(callpt-2, ISP));
	if (inst) {
		ssymbol[0] = '?';
		ssymbol[1] = 0;
		ssymval = 0;
	}
	inst = get(callpt, ISP);
	if (inst == 05726)		/* tst (sp)+ */
		return(narg+1);
	if (inst == 022626)		/* cmp (sp)+,(sp)+ */
		return(narg+2);
	if (inst == 062706)		/* add $n,sp */
		return(narg+get(callpt+2, ISP)/2);
	return(narg);
}

runcom()
{
	int stat;
	register w, i;

	switch(modifier) {


	/* delete breakpoint */
	case 'd':
		if (adrflg==0)
			error();
		for (w=0; w<NBKP; w++) {
			i = bkptl[w].loc;
			if (bkptl[w].flag & BADJST)
				i =- 4;
#ifndef unsw_orig
			if ((dot==i) || (ssflg && !i)) {
#endif
#ifdef unsw_orig
			if (dot==i) {
#endif
				if (lastbp==bkptl[w].loc) {
					ptrace(WUREGS,pid,2*(512+ps),uregs[ps]&~020);
					lastbp = 0;
				} else {
					ptrace(WUSER,pid,bkptl[w].loc,bkptl[w].ins);
				}
#ifndef unsw_orig
				if (!recur) {
					bkptl[w].loc = 0;
					bkptl[w].flag = 0;
				}
#endif
#ifdef unsw_orig
				bkptl[w].loc = 0;
				bkptl[w].flag = 0;
#endif
				return;
			}
		}
		error();

	/* set breakpoint */
	case 'b':
		if (adrflg==0)
			error();
		for (w=0; w<NBKP; w++) {
			i = bkptl[w].loc;
			if (bkptl[w].flag&BADJST)
				i =- 4;
			if (i==dot)
				return;
		}
		for (w=0; w<NBKP; w++)
			if (bkptl[w].loc==0) {
				bkptl[w].loc = dot;
#ifndef unsw_orig
				bkptl[w].flag =| arg<<8;
				arg = -1;
#endif
				return;
			}
		error();

	/* run program */
	case 'r':
		lastbp = 0;
#ifndef unsw_orig
		close(fsym);
		close(fcore);
#endif
		if (pid) {
			ptrace(EXIT, pid, 0, 0);
			pid = 0;
		}
		if ((pid = fork())==0) {
#ifndef unsw_orig
			close(0);
			close(1);
			open(ttnm,0);
			open(ttnm,1);
#endif
			ptrace(SETTRC, 0, 0, 0);
			signal(SIGINT, 0);
			signal(SIGINS, 0);
			doexec();
			printf("Can't execute %s\n", symfil);
			exit(0);
		}
#ifndef unsw_orig
		fsym = open(symfil,0);
#endif
		bpwait(0);
		setbp(1);
		ptrace(WUREGS, pid, 2*(512+ps), 0170000);

	case 'c':
		if (pid==0)
			error();
#ifndef unsw_orig
		setbp(ssflg);
		if (ssflg) {
			ptrace(WUREGS, pid, 2*(512+ps), uregs[ps]&~020);
			ssflg = 0;
		}
#endif
#ifdef unsw_orig
		setbp(0);
#endif
		if (lastbp) {
			w = lastbp;
			ptrace(CONTIN, pid, 0, 0);
			bpwait(1);
			ptrace(WUSER, pid, w, 03);
			ptrace(WUREGS, pid, 2*(512+ps), uregs[ps]&~020);
			lastbp = 0;
		}
		ptrace(CONTIN, pid, 0, 0);
		bpwait(1);
		w = uregs[pc]-2;
		for (i=0; i<NBKP; i++)
			if (bkptl[i].loc == w)
				break;
		if (i >= NBKP) {
			printf("%s\n", signals[signo]);
			return;
		}
		lastbp = w;
		ptrace(WUSER, pid, w, bkptl[i].ins);
		ptrace(WUREGS, pid, 2*(512+pc), w);
		ptrace(WUREGS, pid, 2*(512+ps), uregs[ps]|020);
		printf("Breakpoint: ");
		psymoff(w, 0777);
		printf("\n");
#ifndef unsw_orig
		if ((i = bkptl[i].flag>>8) >= 0)
			grprnt(i);
#endif
		return;
#ifndef unsw_orig

	case 's':
		if (pid==0)
			error();
		if(!ssflg) {
			ssflg++;
			modifier = 'd';
			recur++;
			adrflg++;
			runcom();
			modifier = 's';
			ptrace(WUREGS, pid, 2*(512+ps), uregs[ps]|020);
			recur = 0;
			lastbp = 0;
		}
		if (lastcom=='%') {
			lastcom = 0;
			ssgrp = arg;
		}
		ptrace(CONTIN, pid, 0, 0);
		bpwait(0);
		if (ssgrp>=0)
			grprnt(ssgrp);
		dot = uregs[pc];
		printf("--> ");
		psymoff(dot, 0100000);
		printf("/");
		printins(0);
		printf("\n");
		return;
#endif
	}
	error();
}

doexec()
{
	extern _exectrap;
	char *argl[32];
	register char *p, **ap;
	register c;

	_exectrap++;
	ap = argl;
	*ap++ = symfil;
	p = lp;
	do {
		while (*p==' ')
			p++;
		if (*p=='\n' || *p=='\0')
			break;
#ifndef unsw_orig
		if (*p=='<' || *p=='>') {
			p = redir(p);
			continue;
		}
#endif
		*ap++ = p;
		while (*p!=' ' && *p!='\n')
			p++;
		c = *p;
		*p++ = '\0';
	} while (c != '\n');
	*ap++ = 0;
	execv(symfil, argl);
}

#ifndef unsw_orig
redir(ptr)
char 	*ptr;
{
	register char	*p1;
	register int	filep;

	filep = *ptr++=='<'?0:1;
	while (*ptr==' ' || *ptr=='\t')
		ptr++;
	p1 = ptr;
	while (*p1!=' ' && *p1!='\n' && *p1!='\0')
		p1++;
	*p1++ = '\0';
	close(filep);
	if (filep) {
		if (creat(ptr,0666)<0)
			open(ttnm,filep);
	} else {
		if (open(ptr,filep)<0)
			open(ttnm,filep);
	}
	return(p1);
}

#endif
setbp(runflag)
{
	register w, i1, l;
	int i2;

	for (w=0; w<NBKP; w++) {
		l = bkptl[w].loc;
		if (l && (runflag||bkptl[w].ins==0)) {
			i1 = ptrace(RUSER, pid, l, 0);
			if (i1==04567) {	/* jsr r5,... */
				i2 = ptrace(RUSER, pid, l+2, 0);
				if (loccsv == i2+l+4) { /* jsr r5,csv */
					l =+ 4;
					bkptl[w].loc = l;
					bkptl[w].flag =| BADJST;
					i1 = ptrace(RUSER, pid, l, 0);
				}
			}
			bkptl[w].ins = i1;
			ptrace(WUSER, pid, l, 03);
			if (errno) {
				printf("Can't set breakpoint ");
				psymoff(bkptl[w].loc);
				printf("\n");
			}
		}
	}
}

bpwait(f)
{
	extern int onintr();
	register w;
	int stat;

    loop:
	signal(SIGINT, 1);
	while ((w = wait(&stat))!=pid && w != -1);
	signal(SIGINT, onintr);
	if (w == -1) {
		ptrace(EXIT, pid, 0, 0);
		pid = 0;
		printf("Wait error\n");
		reset();
	}
	if ((stat & 0377) != 0177) {
		if (signo = stat&0177)
			printf("%s\n", signals[signo]);
		printf("Process terminated.\n");
		if (pid == w) {
			pid = 0;
			reset();
		}
		goto loop;
	}
	signo = stat>>8;
	collinfo();
	if (signo!=SIGTRC) {
		printf("%s\n", signals[signo]);
		reset();
	}
}

collinfo()
{
	register i;

	for (i=0; i<9; i++)
		uregs[reglist[i].roffs] =
		    ptrace(RUREGS, pid, 2*(512+reglist[i].roffs), 0);
	setstack();
}

symlook(symstr)
char *symstr;
{
	register i;
	register symv;

	symset();
	if (fsymbol[0]==0) {
		while(symget()) {
			if (eqstr(symbol, symstr)) {
				savsym();
				return(1);
			}
		}
		return(0);
	}
	while (symget()) {
		/* wait for function symbol */
		if (symbol[0]!='~' || !eqstr(symbol, fsymbol))
			continue;
#ifndef unsw_orig
		pokeval = symv = symval;
		fcnstr = symptr;
#endif
#ifdef unsw_orig
		symv = symval;
#endif
		while (symget()&& symbol[0]!='~' &&symflg!=037)
#ifndef unsw_orig
			if (eqstr(symbol, symstr)) {
				is_auto++;
				return(recur?1:localsym(symv));
			}
#endif
#ifdef unsw_orig
			if (eqstr(symbol, symstr))
				return(localsym(symv));
#endif
		return(0);
	}
}

localsym(s)
{
	register i, xr5;

	/* label, static */
	if (symflg>=2 && symflg<=4) {
		ssymval = symval;
		return(1);
	}
	/* auto, arg */
	if (symflg==1) {
		for (i=0; i<callev; i++)
			if (entpt[i]==s) {
				ssymval = symval+callist[i];
				return(1);
			}
		return(0);
	}
	/* register */
	if (symflg==20) {
		for (i=0; i<callev; i++)
			if (entpt[i]==s) {
				if (i==0) {
					return(0); /* temp, no reg lvalue */
				}
				ssymval = callist[i-1] - 10 + 2*symval;
				return(1);
			}
		return(0);
	}
	return(0);
}

eqstr(as1, as2)
int *as1, *as2;
{
	register char *s1, *s2, *es1;

	s1 = as1;
	s2 = as2;
	for (es1 = s1+8; s1 < es1; )
		if (*s1++ != *s2++)
			return(0);
	return(1);
}

vallook(value)
char *value;
{
	register char *diff;

	diff = 0177777;
	symset();
	while (symget())
		if (symflg&040 && value-symval<=diff) {
			if (symflg==1 && value!=symval)
				continue;
			savsym('_');
			diff = value-symval;
		}
	return(diff);
}

get(aaddr, space)
char *aaddr;
{
	int w;
	register int w1;
	register char *addr;

	addr = aaddr;
	if (pid) {		/* tracing on? */
		w = ptrace(space==DSP?RUSER:RIUSER, pid, addr, 0);
		if (addr&01) {
			w1 = ptrace(space==DSP?RUSER:RIUSER, pid, addr+1, 0);
			w = (w>>8)&0377 | (w1<<8);
		}
		errflg = errno;
		return(w);
	}
	w = 0;
	if (idsep==0&&addr<regbuf->u_tsize || idsep&&space==ISP) {
		seek(fsym, addr+020, 0);
		if (read(fsym, &w, 2) != 2)
			errflg++;
		return(w);
	}
	if( idsep!=0 && addr < regbuf->u_dsize ) {
		/* address in data space */
	} else if( idsep==0 && addr < regbuf->u_dsize+rtsize ) {
		/* address in data space, relocate addr
		   remembering that rtsize is zero for 407 a.outs */
		addr =- rtsize;	/* only effective for 410 a.outs */
	} else if (-addr < regbuf->u_ssize) {
		/* address in stack space */
		addr =+ regbuf->u_dsize + regbuf->u_ssize;
	} else
		errflg++;
	seek(fcore, addr+1024, 0);
	if (read(fcore, &w, 2) < 2)
		errflg++;
	return(w);
}

#ifndef unsw_orig
put(aaddr, space, val)
char *aaddr;
{
	int w;
	register char *addr;

	w = val;
	addr = aaddr;
	if (pid) {		/* tracing on? */
		ptrace(space==DSP?WUSER:WIUSER, pid, addr & ~1, w);
		errflg = errno;
		return;
	}
	if (idsep==0&&addr<regbuf->u_tsize || idsep&&space==ISP) {
		seek(fsym, addr+020, 0);
		if (write(fsym, &w, 2) != 2)
			errflg++;
		return;
	}
	errflg++;
}
#endif
symset()
{
	symct = symlen;
	symptr = symbuf;
	seek(fsym, symoff, 0);
}

symget()
{
	register int *p, *q;
	if ((symct =- 12) < 0)
		return(0);
	if (symptr < &symbuf[symcor]) {
		p = symptr;
		for (q=symbol; q <= &symval;)
			*q++ = *p++;
		symptr = p;
		return(1);
	}
	return(read(fsym, symbol, 12) == 12);
}

savsym(skip)
{
	register int ch;
	register char *p, *q;

	p = symbol;
	q = ssymbol;
	while (p<symbol+8 && (ch = *p++)) {
		if (ch == skip)
			continue;
		*q++ = ch;
	}
	while (q < ssymbol+8)
		*q++ = '\0';
	ssymflg = symflg;
	ssymval = symval;
}

onintr()
{
#ifndef unsw_orig
	if (ssflg) {
		modifier = 's';
		runcom();
	} else {
		putchar('\n');
		errflg++;
	}
#endif
#ifdef unsw_orig
	putchar('\n');
	errflg++;
#endif
	reset();
}
