/*
char id_sfe[] = "@(#)sfe.c	1.6";
 *
 * sequential formatted external routines
 */

#include "fio.h"

/*
 * read sequential formatted external
 */

extern int rd_ed(),rd_ned();
int x_tab();

extern char rsfe[];
extern char wsfe[];

e_rsfe()
{	int n;
	n=en_fio();
	fmtbuf=NULL;
	return(n);
}

c_sfe(a,flag) cilist *a; /* check */
{	unit *p;
	int n;
	external=sequential=formatted=FORMATTED;
	fmtbuf=a->cifmt;
	lfname = NULL;
	elist = NO;
	errflag = a->cierr;
	endflag = a->ciend;
	lunit = a->ciunit;
	if(not_legal(lunit)) err(errflag,F_ERUNIT,rsfe+5);
	curunit = p = &units[lunit];
	if(!p->ufd && (n=fk_open(flag,SEQ,FMT,(ftnint)lunit)) )
		err(errflag,n,rsfe+5)
	cf = curunit->ufd;
	elist = YES;
	lfname = curunit->ufnm;
	if(!p->ufmt) err(errflag,F_ERNOFIO,rsfe+5)
	if(p->url) err(errflag,F_ERNOSIO,rsfe+5)
	cursor=recpos=scale=reclen=0;
	radix = 10;
	signit = YES;
	cblank = curunit->ublnk;
	cplus = NO;
	return(OK);
}

/*
 * write sequential formatted external
 */

extern int w_ed(),w_ned();

x_tab()
{	int n;
	if(reclen < recpos) reclen = recpos;
	if(curunit->useek)
	{	if((recpos+cursor) < 0) cursor = -recpos;	/* to BOR */
		n = reclen - recpos;	/* distance to eor, n>=0 */
		if((cursor-n) > 0)
		{	fseek(cf,(long)n,1);  /* find current eor */
			recpos = reclen;
			cursor -= n;
		}
		else
		{	fseek(cf,(long)cursor,1);  /* do not pass go */
			recpos += cursor;
			return(cursor=0);
		}
	}
	else
		if(cursor < 0) return(F_ERSEEK);   /* can't go back */
	while(cursor--)
	{	if(reading)
		{	n = (*getn)();
			if(n=='\n')
			{	(*ungetn)(n,cf);
				return(F_EREREC);
			}
			if(n==EOF) return(EOF);
		}
		else	(*putn)(' ');	/* fill in the empty record */
	}
	return(cursor=0);
}

/*
/*xw_rev()
/*{
/*	if(workdone) x_wSL();
/*	return(workdone=0);
/*}
/*
*/
e_wsfe()
{	return(e_rsfe()); }
