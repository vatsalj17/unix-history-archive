/*
char id_rwiio[] = "@(#)rwiio.c	1.1";
 *
 * internal (character array) i/o
 * common routines
 */

#include "fio.h"
#include "lio.h"

c_fi(a) icilist *a;
{
	fmtbuf=a->icifmt;
	formatted = FORMATTED;
	external = NO;
	cblank=cplus=NO;
	scale=cursor=0;
	radix = 10;
	signit = YES;
	elist = YES;
	svic = a;
	recpos=reclen=0;
	icend = a->iciunit + a->icirnum*a->icirlen;
	errflag = a->icierr;
	endflag = a->iciend;
	if(pars_f(fmtbuf)) err(errflag,F_ERFMT,"ifio")
	fmt_bg();
	return(OK);
}
c_si(a) icilist *a;
{
	sequential = YES;
	recnum = 0;
	icptr = a->iciunit;
	return(c_fi(a));
}
c_di(a) icilist *a;
{
	sequential = NO;
	recnum = a->icirec - 1;
	icptr = a->iciunit + recnum*a->icirlen;
	return(c_fi(a));
}
z_tab()
{	int n;
	if(reclen < recpos) reclen = recpos;
	if((recpos + cursor) < 0) cursor = -recpos;	/* to BOR */
	n = reclen - recpos;
	if(!reading && (cursor-n) > 0)
	{	icptr += n;
		recpos = reclen;
		cursor -= n;
		while(cursor--) if(n=(*putn)(' ')) return(n);
	}
	else
	{	icptr += cursor;
		recpos += cursor;
	}
	return(cursor=0);
}
c_li(a) icilist *a;
{
	fmtbuf="int list io";
	sequential = formatted = LISTDIRECTED;
	external = NO;
	elist = YES;
	svic = a;
	recnum = recpos = 0;
	cplus = cblank = NO;
	icptr = a->iciunit;
	icend = icptr + a->icirlen * a->icirnum;
	errflag = a->icierr;
	endflag = a->iciend;
	leof = NO;
	return(OK);
}
