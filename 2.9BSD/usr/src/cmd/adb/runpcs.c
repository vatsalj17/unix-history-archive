#
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"


MSG		NOFORK;
MSG		ENDPCS;
MSG		BADWAIT;

CHAR		*lp;
INT		sigint;
INT		sigqit;

/* breakpoints */
BKPTR		bkpthead;

REGLIST		reglist[];

CHAR		lastc;
POS		corhdr[];
POS		*uar0;
int		overlay;
OVTAG		curov;

INT		fcor;
INT		fsym;
STRING		errflg;
INT		errno;
INT		signo;

L_INT		dot;
STRING		symfil;
INT		wtflag;
INT		pid;
L_INT		expv;
INT		adrflg;
L_INT		loopcnt;
L_INT		var[];





/* service routines for sub process control */

getsig(sig)
{	return(expr(0) ? shorten(expv) : sig);
}

INT		userpc=1;

runpcs(runmode, execsig)
{
	INT		rc;
	REG BKPTR	bkpt;
	IF adrflg
	THEN userpc=shorten(dot);
	FI
	if (overlay)
		choverlay((U *)corhdr->u_ovdata.uo_curov);
	printf("%s: running\n", symfil);

	WHILE (loopcnt--)>0
	DO
#ifdef DEBUG
		printf("\ncontinue %d %d\n",userpc,execsig);
#endif
		stty(0,&usrtty);
		ptrace(runmode,pid,userpc,execsig);
		bpwait(); chkerr(); readregs();

		/*look for bkpt*/
		IF signo==0 ANDF (bkpt=scanbkpt(uar0[PC]-2))
		THEN /*stopped at bkpt*/
		     userpc=uar0[PC]=bkpt->loc;
		     IF bkpt->flag==BKPTEXEC
			ORF ((bkpt->flag=BKPTEXEC, command(bkpt->comm,':')) ANDF --bkpt->count)
		     THEN execbkpt(bkpt); execsig=0; loopcnt++;
			  userpc=1;
		     ELSE bkpt->count=bkpt->initcnt;
			  rc=1;
		     FI
		ELSE rc=0; execsig=signo; userpc=1;
		FI
	OD
	return(rc);
}

endpcs()
{
	REG BKPTR	bkptr;
	IF pid
	THEN ptrace(EXIT,pid,0,0); pid=0; userpc=1;
	     FOR bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt
	     DO IF bkptr->flag
		THEN bkptr->flag=BKPTSET;
		FI
	     OD
	FI
}

setup()
{
	close(fsym); fsym = -1;
	IF (pid = fork()) == 0
	THEN ptrace(SETTRC,0,0,0);
	     signal(SIGINT,sigint); signal(SIGQUIT,sigqit);
	     doexec(); exit(0);
	ELIF pid == -1
	THEN error(NOFORK);
	ELSE bpwait(); readregs(); lp[0]=EOR; lp[1]=0;
	     fsym=open(symfil,wtflag);
	     IF errflg
	     THEN printf("%s: cannot execute\n",symfil);
		  endpcs(); error(0);
	     FI
	FI
}

execbkpt(bkptr)
BKPTR	bkptr;
{	INT		bkptloc;
#ifdef DEBUG
	printf("exbkpt: %d\n",bkptr->count);
#endif
	bkptloc = bkptr->loc;
	ptrace(WIUSER,pid,bkptloc,bkptr->ins);
	stty(0,&usrtty);
	ptrace(SINGLE,pid,bkptloc,0);
	bpwait(); chkerr();
	ptrace(WIUSER,pid,bkptloc,BPT);
	bkptr->flag=BKPTSET;
}


doexec()
{
	STRING		argl[MAXARG];
	CHAR		args[LINSIZ];
	STRING		p, *ap, filnam;
	ap=argl; p=args;
	*ap++=symfil;
	REP	IF rdc()==EOR THEN break; FI
		*ap = p;
		WHILE lastc!=EOR ANDF lastc!=SP ANDF lastc!=TB DO *p++=lastc; readchar(); OD
		*p++=0; filnam = *ap+1;
		IF **ap=='<'
		THEN	close(0);
			IF open(filnam,0)<0
			THEN	printf("%s: cannot open\n",filnam); exit(0);
			FI
		ELIF **ap=='>'
		THEN	close(1);
			IF creat(filnam,0666)<0
			THEN	printf("%s: cannot create\n",filnam); exit(0);
			FI
		ELSE	ap++;
		FI
	PER lastc!=EOR DONE
	*ap++=0;
	execv(symfil, argl);
}

BKPTR	scanbkpt(adr)
{
	REG BKPTR	bkptr;
	FOR bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt
	DO IF bkptr->flag ANDF bkptr->loc==adr ANDF 
	  (bkptr->ovly == 0 || bkptr->ovly==curov)
	   THEN break;
	   FI
	OD
	return(bkptr);
}

delbp()
{
	REG BKPTR	bkptr;
	FOR bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt
	DO IF bkptr->flag
	   THEN del1bp(bkptr);
	   FI
	OD
}

del1bp(bkptr)
BKPTR bkptr;
{
	if (bkptr->ovly)
		choverlay(bkptr->ovly);
	ptrace(WIUSER,pid,bkptr->loc,bkptr->ins);
}

/* change overlay in subprocess */
choverlay(ovno)
OVTAG ovno;
{
	errno = 0;
	if (overlay && pid && ovno>0 && ovno<=7)
		ptrace(WUREGS,pid,&((U *)0->u_ovdata.uo_curov),ovno);
	IF errno
	THEN printf("cannot change to overlay %d\n", ovno);
	FI
}

setbp()
{
	REG BKPTR	bkptr;

	FOR bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt
	DO IF bkptr->flag
	   THEN set1bp(bkptr);
	   FI
	OD
}
set1bp(bkptr)
BKPTR bkptr;
{
	REG INT		a;
	a = bkptr->loc;
	if (bkptr->ovly)
		choverlay(bkptr->ovly);
	bkptr->ins = ptrace(RIUSER, pid, a, 0);
	ptrace(WIUSER, pid, a, BPT);
	IF errno
	THEN prints("cannot set breakpoint: ");
	     psymoff(leng(bkptr->loc),ISYM,"\n");
	FI
}

bpwait()
{
	REG INT w;
	INT stat;

	signal(SIGINT, 1);
	WHILE (w = wait(&stat))!=pid ANDF w != -1 DONE
	signal(SIGINT,sigint);
	gtty(0,&usrtty);
	stty(0,&adbtty);
	IF w == -1
	THEN pid=0;
	     errflg=BADWAIT;
	ELIF (stat & 0177) != 0177
	THEN IF signo = stat&0177
	     THEN sigprint();
	     FI
	     IF stat&0200
	     THEN prints(" - core dumped");
		  close(fcor);
		  setcor();
	     FI
	     pid=0;
	     errflg=ENDPCS;
	ELSE signo = stat>>8;
	     IF signo!=SIGTRAP
	     THEN sigprint();
	     ELSE signo=0;
	     FI
	     flushbuf();
	FI
}

readregs()
{
	/*get REG values from pcs*/
	REG i;
	FOR i=0; i<NREG; i++
	DO uar0[reglist[i].roffs] =
		    ptrace(RUREGS, pid, (int)&uar0[reglist[i].roffs]-
			(int)&corhdr, 0);
	OD
	/* if overlaid, get ov */
	IF overlay
	THEN	OVTAG ovno;
		ovno = ptrace(RUREGS, pid,
		    &(((struct user *)0)->u_ovdata.uo_curov),0);
		var[VARC] = ovno;
		((U *)corhdr)->u_ovdata.uo_curov = ovno;
		setovmap(ovno);
	FI

#ifndef NONFP
	/* REALing poINT		*/
	FOR i=FROFF; i<FRLEN+FROFF; i++
	DO corhdr[i] = ptrace(RUREGS,pid,i,0); OD
#endif
}


