#
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"


MSG		NOCFN;

INT		callpc;
BOOL		localok;
SYMTAB		symbol;

STRING		errflg;

OVTAG		curov;
INT		overlay;
L_INT		var[36];


findroutine(cframe)
	L_INT		cframe;
{
	REG INT		narg, inst;
	INT		lastpc, back2;
	BOOL		v;
	OVTAG		savov, curovl;

	v=FALSE; localok=FALSE; lastpc=callpc;
	if(overlay) {
		/*
		 *  Save the previous overlay. Don't restore it,
		 *  so that the next call will have the correct previous
		 *  overlay.  The caller must save and restor the original
		 *  overlay if needed.
		 */
		savov = curov;
		curovl=get(cframe-2, DSP);
		setovmap(curovl);
	}
	callpc=get(cframe+2, DSP); back2=get(leng(callpc-2), ISP);
	IF (inst=get(leng(callpc-4), ISP)) == 04737	/* jsr pc,*$... */
	THEN	narg = 1;
	ELIF (inst&~077)==04700			/* jsr pc,... */
	THEN	narg=0; v=(inst!=04767);
	ELIF (back2&~077)==04700
	THEN	narg=0; v=TRUE;
	ELSE	errflg=NOCFN;
		return(0);
	FI
	if (overlay)
		setovmap(savov);	/* previous overlay, for findsym */
	IF findsym( (v ? lastpc : ((inst==04767?callpc:0) + back2) ),ISYM) == -1
	    ANDF !v
	THEN	symbol.symc[0] = '?';
		symbol.symc[1] = 0;
		symbol.symv = 0;
	ELSE	localok=TRUE;
	FI
	if (overlay)
		setovmap(curovl);
	inst = get(leng(callpc), ISP);
	IF inst == 062706		/* add $n,sp */
	THEN
		narg += get(leng(callpc+2), ISP)/2;
		return(narg);
	FI
	IF inst == 05726		/* tst (sp)+ */
	THEN
		return(narg+1);
	FI
	IF inst == 022626		/* cmp (sp)+,(sp)+ */
	THEN
		return(narg+2);
	FI
	return(narg);
}
