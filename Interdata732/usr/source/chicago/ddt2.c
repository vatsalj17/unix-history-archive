#
/*

	Dynamic Debugging Tool
		Bill Allen
			Naval Postgraduate School
			March, 1975

*/

#include "/usr/sys/param.h"
#include "/usr/sys/user.h"
#include "/usr/sys/reg.h"

extern pinstr();
extern prtop();



extern int	fcore;
extern int	fsym;
extern int	wcore;
extern int	wsym;
extern char	*gargv[10];
extern int	pid;
extern int	coroffset;
extern int	symoff;
extern char	*lp;
extern int	errflg;
extern int	symlen;
extern int	symct;
extern char	symbol[8];
extern int	symflg;
extern int	symval;
char tsym[10];
char fsymbol[10];
extern char	ssymbol[8];
extern int	ssymflg;
extern int	ssymval;
extern char	line[128];
extern int	regbuf[512];
extern char	**uregs;
extern char	*rtsize;
int loccsv;
int locsr5;
extern int regloc[];
extern int	dot;
extern int	tdot;
extern int	dotinc;
extern int	lastcom;
extern int	lastype;
extern char	*symfil;
extern char	*corfil;
extern int	callist[50];
int	entpt[50];
int callev;
extern int	*callp;
extern int	leffect;
extern int	headsize;	/* length of file header */
extern int	tsize;		/* length of text segment */
extern int	dsize;		/* length of data segment */
extern int	ssize;		/* length of bss segment */
extern int	gargc;		/* number of arguments from shell */
extern int	sdot;		/* symbolic operand temporary dot */
extern int	ldot;
extern int	elastype;
extern int	adrfg;	/* set if this command had an address */
extern int	status;
extern char *symbuf;
extern int *symptr;
#define BREAK 3		/* system call breakpoint */
#define BRKLEN 20
#define CONDLEN 82
extern struct {
	int value;		/* old value of breakpoint */
	int addr;		/* address of breakpoint */
	char cond[CONDLEN];	/* breakpoint condition */
} brktab[BRKLEN];
extern int brktx;
int symadr;

extern getspsym();
extern putspsym();
extern ssval[];
extern char *ssname[];
extern initssv();

#define	RUSER	1
#define	RIUSER	2
#define	WUSER	4
#define	RUREGS	3
#define	WUREGS	6
#define	SETTRC	0
#define	CONTIN	7
#define	EXIT	8

#define SSR0	0
#define SSR1	1
#define SSR2	2
#define SSR3	3
#define SSR4	4
#define SSR5	5
#define SSR6	6
#define SSR7	7
#define	SSR8	8
#define SSR9	9
#define SSR10	10
#define SSR11	11
#define SSR12	12
#define SSR13	13
#define SSR14	14
#define SSR15	15
#define SSSP	7
#define SSPC	16
#define SSPS	17
#define SS8	18
#define SS9	19
#define SSL	20
#define SSM	21
#define SSQ	22
#define SSA	23
#define SSD	24
#define SSRG	25
#define SSF	26
#define SSSL	27
#define NUMSS	28

/*
 * instruction formats
 */
#define	SF	1
#define	RR	2
#define	RX1	3
#define	RX2	4
#define	RX3	5
#define	RI1	6
#define	RI2	7

#define	BT	010	/* branch instruction (true)	*/
#define	BF	020	/* branch instruction (false)	*/
#define	BBACK	040	/* branch backwards short	*/

/*
 * structure for information about disassembled instruction
 */
extern struct {
	int	type;		/* instruction format	*/
	char	op;		/* opcode	*/
	char	reg[3];		/* registers	*/
	char	*disp;		/* displacement */
} instr;

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


cget(n)
{
	register w;

/***
	w = get(n&0177776);	/* use even address **
	if (errflg)
		reset();
	if(n&1)		/* wants odd char **
		return((w&0177400)>>8);
	return(w);
***/
	w = get(n & ~01);
	if (errflg)
		reset();
	return(w);
}

printc(c)
{
	c =& 0377;		/***/
	if (c<' ' || c>'}')
		printf("\\%o", c);
	else
		printf("%c", c);
}

expr()
{
	register int i,t1,t2, b;
	int donef, adrflg, lastop;

	symadr=0;
	tdot = 0;
	adrflg = 0;
	lastop = '+';
	ssymval = 0;
	donef = 0;
loop:
	fsymbol[0] = 0;
	if (symchar(0)) {
		symadr++;
		adrflg++;
		symcollect('_');
		if (*lp++==':' && symchar(0)) {
			for (i=0; i<8; i++)
				fsymbol[i] = tsym[i];
			fsymbol[0] = '~';
			symcollect(0);
		} else 
			lp--;
		if (symlook(tsym) == 0) {
			printf("*** symbol not found\n");
			reset();
		}
		goto loop;
	}
	if (*lp>='0' && *lp<='9') {
		adrflg++;
		ssymval = 0;
		if (*lp == '0') {
			b = 8;
			if (*++lp == 'x') {
				b = 16;
				lp++;
			}
		}
		else
			b = 10;
		for (;;) {
			if ((i = *lp - '0') > 9)
				if ((i =- 'a'-'9'-1) < 10)
					break;
			if (i >= b || i < 0)
				break;
			ssymval = ssymval * b + i;
			lp++;
		}
		goto loop;
	}
	if(*lp == '.'){		/* dot */
		ssymval = dot;
over:
		adrflg++;
		lp++;
		goto loop;
	}
	if(*lp == '\''){	/* one ASCII char */
		ssymval = *++lp;
		goto over;
	}
	if(*lp == '"'){		/* two ASCII chars */
		ssymval = (*++lp)<<8;
		ssymval =| *++lp;	/* put in second char */
		goto over;
	}
	if(*lp == ':'){		/* special symbol */
		adrflg++;
		lp++;
		ssymval = getspsym();
		goto loop;
	}
	switch (*lp) {

	default:
		donef++;

	case ' ':
	case '+':
	case '-':
	case '*':
	case '\\':
	case '%':
	case '|':
		if(*(lp+1) == '|')		/* relational */
			donef++;
	case '&':
		if(*(lp+1) == '&')		/* relational */
			donef++;
	case '@':
		switch(lastop) {

		case '@':		/* indirection */
			tdot = get(ssymval);
			goto op;

		case ' ':
		case '+':
			tdot =+ ssymval;
			goto op;

		case '*':
			tdot = tdot * ssymval;
			goto op;

		case '\\':
			tdot =/ ssymval;
			goto op;

		case '%':
			tdot =% ssymval;
			goto op;

		case '|':
			tdot =| ssymval;
			goto op;

		case '&':
			tdot =& ssymval;
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

	case '\t':
		lp++;
		goto loop;

	case '(':		/* function argument */
		lp++;
		adrflg++;
		t1 = tdot;
		ssymval = getarg();
		tdot = t1;
		goto loop;

	case '[':
		lp++;
		t1 = ssymval;
		t2 = tdot;
		if (expr() == 0)
			tdot = 0;
		ssymval = get(t1 + (tdot<<2));
		if (errflg)
			reset();
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
	if (c && !ssval[SSA])
		*p++ = c;
	while (symchar(1)) {
		if (p < &tsym[8])
			*p++ = *lp;
		lp++;
	}
	while (p < &tsym[8])
		*p++ = 0;
}

symchar(dig)
{
	if (*lp>='a'&&*lp<='z' || *lp>='A'&&*lp<='Z' || *lp=='_')
		return(1);
	if (dig && *lp>='0' && *lp<='9')
		return(1);
	return(0);
}

setstack()
{
	register int tpc, i;
	int trbase;

	tpc = ssval[SSPC];
	trbase = ssval[SSR14];
	callev = 0;
	while (errflg == 0) {
		findroutine(tpc, &trbase);
		tpc = get(trbase+28);
		if (callev >= 50)
			break;
		entpt[callev] = ssymval;
		callist[callev++] = trbase;
		if ((trbase = get(trbase+24)) == 0)
			break;
	}
	errflg = 0;
	setfunc();
}

getarg()
{
	register level, arg, t1;

	t1 = tdot;
	expr();
	level = tdot;
	if (*lp++ != ',')
		error();
	expr();
	arg = tdot;
	if (*lp++ != ')')
		error();
	if (level >= callp-callist)
		error();
	ssymval = callist[level] - 8 - 4*arg;
	tdot = t1;
}
error()
{
	printf("** invalid function argument specifier\n");
	reset();
}

printtrace()
{
	register int tpc,narg;
	int trbase;
	int argp, i;

	if(fcore<0 && pid==0){	/* no core file */
		printf("no core image file\n");
		return;
	}
	tpc = ssval[SSPC];
	trbase = ssval[SSR14];
	callp = &callist[0];
	while (errflg == 0) {
		narg = findroutine(tpc, &trbase)+2;
		printf("%2d: %.8s(", callp-callist, ssymbol);
		if (--narg >= 0)
			printf("%o", get(trbase+32));
		argp = trbase+32;
		while(--narg >= 0) {
			printf(",%o", get(argp =+ 4));
		}
		printf(")\n");
		tpc = get(trbase+28);
		if (callp < &callist[50])
			*callp++ = trbase;
		if ((trbase = get(trbase+24)) == 0)
			break;
	}
}

findroutine(apc, arbase)
int *arbase;
{
	register callpt, inst, narg;

	if (findbase(apc, arbase) == 0
	  && findbase(ssval[SSR15], arbase) == 0) {
		errflg++;
		return(0);
	}
	callpt = get(*arbase+28);
	if ((inst=get(callpt-6)) >> 24 == 0101)	/* rx3 */
		inst = get(callpt-4) & 03777777;
	else if (((inst>>8) & 0377) == 0101) {	/* rx1, rx2 */
		inst = get(callpt-4) & 0177777;
		if (inst & 0100000) {		/* rx2 */
			if (inst & 040000)
				inst =| ((-1)<<16);
			else
				inst =& 037777;
			inst =+ callpt;
		}
	}
	else {
		errflg++;
/***		printf("*** unable to set stack\n");	***/
		return(0);
	}
	inst = vallook(inst);
	if (inst) {
		ssymbol[0] = '?';
		ssymbol[1] = 0;
		ssymval = 0;
	}
	inst = get(callpt);
	if ((inst>>20) == 01147)	/* ais sp,x */
		return(((inst>>16)&15)/4);
	if ((inst>>20) == 06247)	/* ahi sp,x */
		return((inst&0177777)/4);
	if ((inst>>20) == 07647)	/* ai sp,x */
		return(get(callpt+2)/4);
	return(0);
}

/*
 * Adjust stack base register to point to saved regs
 */
#define	SHI	0xcb
#define	SI	0xfb
#define	STM	0xd0
findbase(apc, arbase)
int *arbase;
{
	register loc;

	vallook(apc);		/* name of current function */
	loc = ssymval;		/* entry pt of current function */
	loc =+ disasm(loc);	/* skip SI */
	if (instr.op != SI && instr.op != SHI)
		return(0);
	loc =+ disasm(loc);	/* check STM */
	if (instr.op != STM || apc < loc+2)
		return(0);
	*arbase =+ instr.disp;	/* adjust stack base by offset in STM */
	return(1);
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
		if(*symstr == '_') {	/* look for a local symbol */
			symset();
			while(symget()) {
				if(symbol[0]!='~' || symval!=ssval[SSF])
					continue;
				while(symget() && symbol[0]!='~' && symflg!=037)
					if(eqstr(symbol,symstr+1))
						return(localsym(ssval[SSF]));
				return(0);
			}
		}
		return(0);
	}
	while (symget()) {
		/* wait for function symbol */
		if (symbol[0]!='~' || !eqstr(symbol, fsymbol))
			continue;
		symv = symval;
		while (symget()&& symbol[0]!='~' &&symflg!=037)
			if (eqstr(symbol, symstr))
				return(localsym(symv));
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
		if (symflg&040 && value>=symval && value-symval<=diff) {
			if (symflg==1 && value!=symval)
				continue;
			savsym('_');
			diff = value-symval;
		}
	return(diff);
}

get(aaddr)
char *aaddr;
{
	int w;
	register a, i;
	register char *addr;

	addr = aaddr&~01;
	if (addr&02)	/* not on word boundary */
		return((get(addr-2)<<16) | (get(addr+2)>>16));
	for(i=0;i<=brktx;i++)
		if((brktab[i].addr&~03) == addr)
			return(brktab[i].value);
	if(pid) {
		return(ptrace(RUSER,pid,addr,0));
	}
	w = 0;
	if(gargc>3){	/* file is not core image */
		seek(fcore,addr,0);
		if(read(fcore,&w,4) != 4)
			printf("** unable to read core file\n");
		return(w);
	}
	if (addr < tsize) {
		if(fcore>0) goto rdcore;
rdsym:	seek(fsym, addr+040, 0);
		if (read(fsym, &w, 4) != 4)
			printf("** unable to read a.out file\n");
		return(w);
	}
	if (addr < rtsize+dsize) {
		if(fcore<0) goto rdsym;		/* no core file */
		addr =- rtsize;
	}
	else if (addr > (14<<16) && addr < (14<<16)+ssize)
			addr =+ dsize - (14<<16);
		else
			printf("** invalid address\n");
	if(fcore<0){
		printf("** can't access bss - no core file\n");
		return(0);
	}
rdcore:
	seek(fcore, addr+03000, 0);
	if (read(fcore, &w, 4) < 4)
		printf("can't read core file\n");
	return(w);
}

symset()
{
	symct = symlen;
	symptr = symbuf;
}

/*
  * convert symbols from symbol table into lowercase
 */
symget()
{
	register int *p, *q;
	register char *s;
	register c;

	if ((symct =- 16) < 0)		/***/
		return(0);
	p = symptr;
	for (q=symbol; q <= &symval; q++)
		*q = *p++;
	symptr = p;
/*** Fudge for Interdata assembler which had only uppercase symbols
	for (s=symbol; c = *s; s++) {
		if (s >= &symbol[8])
			break;
		if (c >= 'A' && c <= 'Z')
			*s = (c =+ 'a'-'A');
	}
***/
	return(1);
}

savsym(skip)
{
	register int ch;
	register char *p, *q;

	p = symbol;
	q = ssymbol;
	if(!ssval[SSA] && (*p==skip || (skip=='_' && *p=='~')))
		p++;
	while (p<symbol+8 && (ch = *p++)) {
		*q++ = ch;
	}
	while (q < ssymbol+8)
		*q++ = '\0';
	ssymflg = symflg;
	ssymval = symval;
	ch = symflg&07;
	if(ch==3 || ch==4)		/* data or bss */
		ssymval =+ ssval[SSD];	/* adjust for D space offset */
}

onintr()
{
	putchar('\n');
	errflg++;
	reset();
}

/* print a local symbol if possible */
plocsym(addr)
{

	register flg;

	symset();	/* beginning of symbol table */
	flg = 0;
	while(symget()) {
		if(symbol[0] == '~' ) {
			if(symval <= dot)
				continue;
			else
				break;
		}
		else
			if((symflg&077) == 1 && symval==addr) {
				savsym(0);
				flg++;
			}
	}
	if(flg)
		printf("%.8s",ssymbol);
	else
		printf("%o(r5)",addr);
}

/* print dot symbolicly if possible */

psym(addr)
{

	register int offset;

	if(leffect==0)		/* need to save effective address */
		leffect = addr;
	if(addr>ssval[SSL] || addr<0)		/* lowest address to use as a symbol */
		if((offset=closeval(addr))>=0){	/* find a close symbol */
			if(offset>0)
				printf("%.8s+%o",ssymbol,offset);
			else
				printf("%.8s",ssymbol);
			return;
		}
	printf("%o",addr);		/* no close symbol */
}

/* find a symbol whose value is close to the given value */

closeval(tvalue)
{

	register int tcval,value;
	int dflag;
	register int t;

	value = tvalue;
	dflag = 1;
	t = ssval[SSD];
	if(t) {		/*  D space offset defined */

/* the following complex code determines if value>=t in unsigned */
/*  16 bit addresses using signed 16 bit arithmetic. */

		if(value>=0){
			if(t>0 && value>=t){	/* true */
				value =- t;			/* convert to symbol table address */
				dflag=0;			/* D space symbol */
			}
		}
		else {	/* value<0 */
			if(t>=0){
				value = (value&077777) + (077777-t) + 1;
				dflag = 0;
			}
			else if(value>=t){	/* both are negative */
				value = (value&077777) - (t&077777);
				dflag = 0;
			}
		}
	}
	if(value<0 && value>=ssval[SSSL])	/* on the stack */
		return(closeloc(value));	/* local symbol */
	tcval=ssval[SSRG];		/* set close symbol range */
	symset();			/* beginning of symbol table */
	while(symget()) {
		t = symflg&037;	/* symbol type */
		if(t != 037 && (t!=1 || ssval[SSA])){ /* no file names not abs or assmebly */
			if(ssval[SSD]==0 || (dflag && t!=3 && t!=4) || (!dflag &&
			  (t==3 || t==4))) {	/* for system I&D split */
				if(value == symval){	/* found it */
					savsym('_');
					return(0);
				}
/* find the signed difference between the two sixteen bit addresses */
				if(value>=0 && symval<0)
					t = -(symval-value);
				else
					t = value-symval;
				if(t>0 && t<tcval){
					tcval = t;			/* offset value */
					savsym('_');
				}
			}
		}
	}
	if(tcval==ssval[SSRG])		/* didnt find one close */
		return(-1);
	return(tcval);
}

/* try to find a local symbol close to value */
/*  return the offset */

closeloc(tvalue)
{

	register value, offset, t;

	offset = ssval[SSRG];	/* symbol range */
	for(t=0; t<callev; t++)
		if(entpt[t] == ssval[SSF]) {
			value = tvalue-callist[t];
			break;
		}
	symset();
	while(symget()) {
		if(symbol[0]!='~' || symval!=ssval[SSF])
			continue;
		while(symget() && symbol[0]!='~' && symflg!=037) {
			if(value==symval) {
				savsym('_');
				return(0);	/* direct hit */
			}
			t = symval - value;
			if(t>0 && t<offset) {
				offset = t;
				savsym('_');
			}
		}
		if(offset==ssval[SSRG])
			return(-1);
		return(offset);
	}
	return(-1);
}


/* store value in the file in the word addressed by dot. */
/*   use the core file if it exists. otherwise use the a.out file. */

cfput(value)
{

	register int addr,i,seektyp;

	seektyp = 0;
	addr = (dot&~01);				/* address in file */
	if(pid) {
		if (addr&02) {
			printf("** not on word boundary\n");
			return;
		}
		if(ptrace(WUSER,pid,addr,value) == -1) {
			printf("Unable to write child memory\n");
			printf("addr = %o  value = %o\n",addr,value);
		}
		return;
	}
	if(gargc>3){		/* not a core image */
		if(wcore<0){
cwf:		printf("no write access to core file\n");
			return;
		}
		if(seek(wcore,addr,0)<0)
			goto cwf;
		if(write(wcore,&value,4) != 4)
			goto cwf;
		return;
	}
	if(wcore>0){	/* can write on core */
		seek(wcore,addr+03000,1);	/* desired address */
		if(write(wcore,&value,4) != 4)
			printf("no write access to core file\n");
	}
	seek(wsym,addr+040,0);	/* write on symbollic file */
	if(write(wsym,&value,4) != 4)
		printf("no write access %s\n",symfil);
}

/* clear one or all breakpoints */
clbkpt(adr)
{

	register int i,found;

	found=0;
	if(brktx<0) return;
	for(i=0;i<=brktx;i++){	/* thru brkpt table */
		if((!adrfg || brktab[i].addr==adr) && brktab[i].addr != -1){
			dot = brktab[i].addr&~03;	/* address to reset */
			cfput(brktab[i].value);	/* reset value */
			brktab[i].value = -1;
			brktab[i].addr = -1;
			found++;
		}
	}
	if(found)
		putchar('\n');
	else if(adrfg)
		printf("?? no breakpoint\n");
	dot = ldot;		/* reset dot */
	return;
}

/* initialize known symbols from the core file */
initfcor()
{
	register t,i ;

	if(fcore<0)  {
		printf("cant initialize from fcore\n");
		return;
	}
	if(seek(fcore,coroffset,3)<0){	/* beginning of core file */
		printf("fcore seek error\n");
		endit();
	}
	if(read(fcore, regbuf, 03000)<03000) {
		printf("fcore read error\n");
		endit();
	}
	t = regbuf->u_ar0;
	t =- (14<<16);
	uregs = &regbuf[t/4];
	status = (regbuf->u_arg[0]);
	tsize=(regbuf->u_tsize)<< 8;
	dsize=(regbuf->u_dsize)<< 8;
	ssize=(regbuf->u_ssize) << 8;
	rtsize = (regbuf->u_tsize+0177777) & ~0177777;
	headsize = 03000;	/* core file header */
	/* copy user regs to special symbols */
	for (i = SSR0; i <= SSPS; i++)
		ssval[i] = uregs[regloc[i]];
/***
	if(pid>0)			/* at a breakpoint **
		ssval[SSPC] =- 2;	/* .-2 is real instr address **
***/
}

/* put special symbol values back in core file */
restcore()
{

	register i;

	for(i=SSR0; i<SSPS; i++)		/* restore special symbols */
		ptrace(WUREGS,pid,regbuf->u_ar0+regloc[i],ssval[i]);
}

/*  get the registers from breakpointed child */
initfmem()
{

	register i;

	regbuf->u_ar0 = ptrace(RUREGS, pid, &0->u_ar0, 0);
	regbuf->u_ar0 =- (14<<14);
	for(i=SSR0; i<SSPS; i++) 
		ssval[i] = 
			ptrace(RUREGS,pid,regbuf->u_ar0+regloc[i],0);
/***
	if(pid) {
		ssval[SSPC] =- 2;
		ssval[SSPS] =& ~020;
	}
***/
}

endit()
{

	if(pid>0) {
		ptrace(EXIT,pid,0,0);
		pid = 0;
	}
	adrfg = 0;
	clbkpt(0);	/* clear all breakpoints */
	exit();
}

/* init file segment sizes from symbolic file */
initfsym()
{

	seek(fsym,0,0);	/* beginning */
	read(fsym, regbuf, 040);
	if (regbuf[0]!=0410 && regbuf[0]!=0407 && regbuf[0]!=0411) {/* magic */
		printf("Not a.out format: %s\n", symfil);
		gargc = 4;
		fcore = fsym;
		wcore = wsym;
		return;
	}
	symoff = regbuf[1] + regbuf[2];
	symlen = regbuf[4];
	if (regbuf[7] != 1)
		symoff =<< 1;
	symoff =+ 040;
	tsize = regbuf[1];	/* text size */
	dsize = regbuf[2];	/* data size */
	ssize = 0;	/* no bss */
	headsize = 040;		/* a.out header size */
	rtsize = tsize;
}

/*  set the default current function */
setfunc()
{

/***
	symset();
	while(symget()) {
		if(symbol[0] == '~') {
			if(symval <= ssval[SSPC])
				ssval[SSF] = symval;
			else
				break;
		}
	}
***/
	vallook(ssval[SSPC]);
	ssval[SSF] = ssymval;
}

/*  check for conditional breakpoint */
/* 	return 0 if no break */
/*          !0 if break */

condbpt()
{
	register bpx;

	for(bpx=0; bpx<BRKLEN; bpx++)
		if(brktab[bpx].addr == ssval[SSPC]) {
			if(brktab[bpx].cond[0] == '\0')
				return(1);		/* unconditional breakpoint */
			else
				return(evalcond(bpx));
		}
	return(1);	/* not in break table */
}

/*  evaluate conditinal expression */
/* 	return */
/* 		0 if false */
/* 		!0 if true */

relexpr()
{

	register condv,nextrel;
	int adrflg;

	condv=0;

	adrflg = expr();
	if(errflg || adrflg==0) {
err:
		printf("*** invalid breakpoint condition\n");
		errflg++;
		return(1);
	}
	if(symadr)
		tdot = cget(tdot);
	condv = tdot;
	switch(*lp++) {

	default:
		return(condv);

	case '=':
		if(*lp != '=')
			goto err;
		lp++;
		nextrel = 2;
		break;

	case '!':
		if(*lp != '=')
			goto err;
		lp++;
		nextrel = 3;
		break;

	case '<':
		if(*lp == '=') {
			lp++;
			nextrel = 5;
		}
		else
			nextrel = 4;
		break;

	case '>':
		if(*lp == '=') {
			lp++;
			nextrel = 7;
		}
		else
			nextrel = 6;
		break;
	}
	if(expr()==0 || errflg!=0)
		goto err;
	if(symadr)
		tdot = cget(tdot);
	switch(nextrel) {

	case 2:		/*  == */
		condv = (condv == tdot);
		break;

	case 3:		/*  != */
		condv = (condv != tdot);
		break;

	case 4:		/*  < */
		condv = (condv < tdot);
		break;

	case 5:		/*  <= */
		condv = (condv <= tdot);
		break;

	case 6:		/*  > */
		condv = (condv > tdot);
		break;

	case 7:		/*  >= */
		condv = (condv >= tdot);
		break;

	}
	return(condv);
}

evalcond(bpx)
{

	register lval,rval;

	lp = &brktab[bpx].cond[0];
	lval = relexpr();
	while(errflg == 0) {
		switch(*lp++) {

		default:
			if(*lp != '\0')
				goto err;
			lp = line;
			return(lval);

		case '|':
			if(*lp != '|')
				goto err;
			lp++;
			rval = relexpr();
			lval = (lval || rval);
			break;

		case '&':
			if(*lp != '&')
				goto err;
			lp++;
			rval = relexpr();
			lval = (lval && rval);
			break;
		}
	}
err:
	printf("*** invalid breakpoint condition\n");
	return(1);
}
