/*
char id_wsfe[] = "@(#)wsfe.c	1.1";
 *
 * write sequential formatted external routines
 */

#include "fio.h"

int	pr_put(), x_putc(), x_wend(), x_wnew();
extern	int	w_ed(), w_ned(), x_tab();

char wsfe[] = "write sfe";
ioflag new;

s_wsfe(a) cilist *a;	/*start*/
{	int n;
	reading = NO;
	if(n=c_sfe(a,WRITE)) return(n);
	if(!curunit->uwrt && ! nowwriting(curunit)) err(errflag, errno, wsfe)
	curunit->uend = NO;
	if (curunit->uprnt) putn = pr_put;
	else putn = x_putc;
	new = YES;
	doed= w_ed;
	doned= w_ned;
	doend = x_wend;
	dorevert = donewrec = x_wnew;
	dotab = x_tab;
	if(pars_f(fmtbuf)) err(errflag,F_ERFMT,wsfe)
	fmt_bg();
	return(OK);
}

x_putc(c)
{
	if(c=='\n') recpos = reclen = cursor = 0;
	else recpos++;
	if (c) putc(c,cf);
	return(OK);
}

pr_put(c)
{
	if(c=='\n')
	{	new = YES;
		recpos = reclen = cursor = 0;
	}
	else if(new)
	{	new = NO;
		if(c=='0') c = '\n';
		else if(c=='1') c = '\f';
		else return(OK);
	}
	else recpos++;
	if (c) putc(c,cf);
	return(OK);
}

x_wnew()
{
	if(reclen>recpos) fseek(cf,(long)(reclen-recpos),1);
	return((*putn)('\n'));
}

x_wend(last) char last;
{
	if(reclen>recpos) fseek(cf,(long)(reclen-recpos),1);
	return((*putn)(last));
}
