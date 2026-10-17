#
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"


SYMTAB		symbol;
INT		lastframe;
INT		callpc;



CHAR		*lp;
INT		octal;
STRING		errflg;
L_INT		localval;
CHAR		isymbol[8];

CHAR		lastc;
POS		*endhdr;

L_INT		dot;
L_INT		ditto;
INT		dotinc;
L_INT		var[];
L_INT		expv;




expr(a)
{	/* term | term dyadic expr |  */
	INT		rc;
	L_INT		lhs;

	rdc(); lp--; rc=term(a);

	WHILE rc
	DO  lhs = expv;

	    switch (readchar()) {

		    case '+':
			term(a|1); expv =+ lhs; break;

		    case '-':
			term(a|1); expv = lhs - expv; break;

		    case '#':
			term(a|1); expv = round(lhs,expv); break;

		    case '*':
			term(a|1); expv =* lhs; break;

		    case '%':
			term(a|1); expv = lhs/expv; break;

		    case '&':
			term(a|1); expv =& lhs; break;

		    case '|':
			term(a|1); expv =| lhs; break;

		    case ')':
			IF (a&2)==0 THEN error(BADKET); FI

		    default:
			lp--;
			return(rc);
	    }
	OD
	return(rc);
}

term(a)
{	/* item | monadic item | (expr) | */

	switch (readchar()) {

		    case '*':
			term(a|1); expv=chkget(expv,DSP); return(1);

		    case '@':
			term(a|1); expv=chkget(expv,ISP); return(1);

		    case '-':
			term(a|1); expv = -expv; return(1);

		    case '~':
			term(a|1); expv = ~expv; return(1);

		    case '(':
			expr(2);
			IF *lp!=')'
			THEN	error(BADSYN);
			ELSE	lp++; return(1);
			FI

		    default:
			lp--;
			return(item(a));
	}
}

item(a)
{	/* name [ . local ] | number | . | ^ | <var | <register | 'x | | */
	INT		base, d, frpt, regptr;
	CHAR		savc;
	BOOL		hex;
	L_INT		frame;
	UNION{REAL r; L_INT i;} real;
	SYMPTR		symp;

	hex=FALSE;

	readchar();
	IF symchar(0)
	THEN	readsym();
		IF lastc=='.'
		THEN	frame=endhdr[r5]&EVEN; lastframe=0; callpc=endhdr[pc];
			WHILE errflg==0
			DO  findroutine(frame);
			    IF  eqstr(symbol.symc,isymbol)
			    THEN break;
			    FI
			    lastframe=frame;
			    frame=get(frame,DSP)&EVEN;
			    IF frame==0
			    THEN error(NOCRTN);
			    FI
			OD
			expv=frame; readchar();
			IF symchar(0)
			THEN	readsym();
				REP IF localsym(frame)==0 THEN error(BADSYM); FI
				    expv=localval;
				PER !eqstr(symbol.symc,isymbol) DONE
			FI
		ELIF (symp=lookupsym(isymbol))==0 THEN error(BADSYM);
		ELSE expv = symp->symv;
		FI
		lp--;


	ELIF digit(lastc) ORF (hex=TRUE, lastc=='#' ANDF hexdigit(readchar()))
	THEN	expv = 0;
		base = (lastc == '0' ORF octal ? 8 : (hex ? 16 : 10));
		WHILE (hex ? hexdigit(lastc) : digit(lastc))
		DO  expv =* base;
		    IF (d=convdig(lastc))>=base THEN error(BADSYN); FI
		    expv =+ d; readchar();
		OD
		IF lastc=='.' ANDF (base==10 ORF expv==0) ANDF !hex
		THEN	real.r=expv; frpt=0; base=10;
			WHILE digit(readchar())
			DO	real.r =* base; frpt++;
				real.r =+ lastc-'0';
			OD
			WHILE frpt--
			DO	real.r =/ base; OD
			expv = real.i;
		FI
		lp--;

	ELIF lastc=='.'
	THEN	expv=dot;

	ELIF lastc=='"'
	THEN	expv=ditto;

	ELIF lastc=='+'
	THEN	expv=inkdot(dotinc);

	ELIF lastc=='^'
	THEN	expv=inkdot(-dotinc);

	ELIF lastc=='<'
	THEN	savc=rdc();
		IF regptr=getreg(savc)
		THEN	expv=endhdr[regptr];
		ELIF (base=varchk(savc)) != -1
		THEN	expv=var[base];
		ELSE	error(BADVAR);
		FI

	ELIF lastc=='\''
	THEN	d=4; expv=0;
		WHILE quotchar()
		DO  IF d--
		    THEN IF d==1 THEN expv =<<16; FI
			 expv =| ((d&1)?lastc:lastc<<8);
		    ELSE error(BADSYN);
		    FI
		OD

	ELIF a
	THEN	error(NOADR);
	ELSE	lp--; return(0);
	FI
	return(1);
}

/* service routines for expression reading */

readsym()
{
	register char *p;

	p = isymbol;
	REP IF p < &isymbol[8]
	    THEN *p++ = lastc;
	    FI
	    readchar();
	PER symchar(1) DONE
	WHILE p < &isymbol[8] DO *p++ = 0; OD
}

SYMPTR	lookupsym(symstr)
STRING	symstr;
{
	SYMPTR		symp;
	symset();
	WHILE symp=symget()
	DO  IF eqstr(symp->symc, symstr)
	    THEN return(symp);
	    FI
	OD
	return(0);
}

hexdigit(c)
CHAR c;
{	return((c>='0' ANDF c<='9') ORF (c>='a' ANDF c<='f'));
}

convdig(c)
CHAR c;
{
	IF digit(c)
	THEN	return(c-'0');
	ELIF hexdigit(c)
	THEN	return(c-'a'+10);
	ELSE	return(17);
	FI
}

digit(c) char c;	{return(c>='0' ANDF c<='9');}

letter(c) char c;	{return(c>='a' ANDF c<='z' ORF c>='A' ANDF c<='Z');}

symchar(dig)
{
	IF lastc=='\\' THEN readchar(); return(TRUE); FI
	return( letter(lastc) ORF lastc=='_' ORF dig ANDF digit(lastc) );
}

varchk(name)
{
	IF digit(name) THEN return(name-'0'); FI
	IF letter(name) THEN return((name&037)-1+10); FI
	return(-1);
}

