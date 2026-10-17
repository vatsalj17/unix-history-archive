#
/*
 *
 *	UNIX debugger
 *
 */

#include "proc.h"
#include "defs.h"

CHAR		*lp;
INT		sigint;
INT		sigqit;

/* breakpoints */
BKPTR		bkpthead;

REGLIST		reglist[];

CHAR		lastc;
POS		corhdr[];
POS		*endhdr;

INT		fcor;
INT		fsym;
STRING		errflg;
INT		errno;
INT		signo;

INT		devswap;
INT		proctab;
INT		devmem;

L_INT		dot;
STRING		symfil;
INT		wtflag;
INT		pid;
L_INT		expv;
INT		adrflg;
L_INT		loopcnt;





/* service routines for sub process control */

getsig(sig)
{	return(expr(0) ? shorten(expv) : sig);
}

runpcs(execsig)
{
	INT		rc;
	REG BKPTR	bkpt;
	IF adrflg
	THEN ptrace(WUREGS,pid,USERPC,shorten(dot));
	FI
	ptrace(WUREGS,pid,USERPS,endhdr[ps]);
	setbp();
	printf("%s: running\n", symfil);

	WHILE (loopcnt--)>0
	DO	/*DEBUG printf("\ncontinue %d\n",execsig); */
		ptrace(CONTIN,pid,0,execsig);
		bpwait(); chkerr(); readregs();

		/*look for bkpt*/
		IF signo==0 ANDF (bkpt=scanbkpt(endhdr[pc]-2))
		THEN /*stopped at bkpt*/
		     ptrace(WUREGS,pid,USERPC,endhdr[pc]=bkpt->loc);
		     IF bkpt->flag==BKPTEXEC
			ORF (bkpt->flag=BKPTEXEC, command(bkpt->comm,':')) ORF --bkpt->count
		     THEN execbkpt(bkpt); execsig=0; loopcnt++;
		     ELSE bkpt->count=bkpt->initcnt;
			  rc=1;
		     FI
		ELSE rc=0; execsig=signo;
		FI
	OD
	return(rc);
}

endpcs()
{
	REG BKPTR	bkptr;
	IF pid
	THEN ptrace(EXIT,pid,0,0); pid=0;
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
	     signal(SIGINT,sigint); signal(SIGQIT,sigqit);
	     devclose();
	     doexec(); exit(0);
	ELIF pid == -1
	THEN error(NOFORK);
	ELSE bpwait(); readregs(); lp[0]=EOR; lp[1]=0;
	     fsym=open(symfil,wtflag);
	     IF errflg
	     THEN printf("%s: cannot execute\n",symfil);
		  endpcs(); error(0);
	     ELSE proctab=ptrace(RUREGS,pid,UPROCP,0);
	     FI
	FI
}

execbkpt(bkptr)
BKPTR	bkptr;
{	INT		saveps, bkptloc;
	/*DEBUG printf("exbkpt: %d\n",bkptr->count); */
	bkptloc = bkptr->loc;
	IF ((saveps=gtrace(RUREGS, USERPS, 0))&TBIT)==0
	THEN ptrace(WUREGS,pid,USERPS,saveps|TBIT);
	FI
	ptrace(WIUSER,pid,bkptloc,bkptr->ins);
	ptrace(CONTIN,pid,0,0);
	bpwait(); chkerr();
	ptrace(WIUSER,pid,bkptloc,BPT);
	bkptr->flag=BKPTSET;
	IF (saveps&TBIT)==0
	THEN ptrace(WUREGS,pid,USERPS,gtrace(RUREGS,USERPS,0)&~TBIT);
	FI
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
	exect(symfil, argl);
}

BKPTR	scanbkpt(adr)
{
	REG BKPTR	bkptr;
	FOR bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt
	DO IF bkptr->flag ANDF bkptr->loc==adr
	   THEN break;
	   FI
	OD
	return(bkptr);
}

delbp()
{
	REG INT		a;
	REG BKPTR	bkptr;
	FOR bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt
	DO IF bkptr->flag
	   THEN a=bkptr->loc;
		ptrace(WIUSER,pid,a,bkptr->ins);
	   FI
	OD
}

setbp()
{
	REG INT		a;
	REG BKPTR	bkptr;

	FOR bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt
	DO IF bkptr->flag
	   THEN a = bkptr->loc;
		bkptr->ins = ptrace(RIUSER, pid, a, 0);
		ptrace(WIUSER, pid, a, BPT);
		IF errno
		THEN prints("cannot set breakpoint: ");
		     psymoff(leng(bkptr->loc),ISYM,"\n");
		FI
	   FI
	OD
}

bpwait()
{
	REG INT w;
	INT stat;

	signal(SIGINT, 1);
	WHILE (w = wait(&stat))!=pid ANDF w != -1 DONE
	signal(SIGINT,sigint);
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
	     IF signo!=SIGTRC
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
	FOR i=0; i<9; i++
	DO endhdr[reglist[i].roffs] =
		    gtrace(RUREGS, 2*(ENDHDR+reglist[i].roffs), 0);
	OD

	/* REALing poINT		*/
	FOR i=FROFF; i<FRLEN+FROFF; i++
	DO corhdr[i] = gtrace(RUREGS,i,0); OD
}


