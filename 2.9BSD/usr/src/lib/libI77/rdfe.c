/*
char	id_rdfe[] = "@(#)rdfe.c	1.1";
 *
 * read direct formatted external i/o
 */

#include "fio.h"

int	y_getc(), y_rnew();
extern	int	rd_ed(), rd_ned(), y_tab();

char rdfe[] = "read dfe";

s_rdfe(a) cilist *a;
{
	int n;
	reading = YES;
	if(n=c_dfe(a,READ)) return(n);
	if(curunit->uwrt && ! nowreading(curunit)) err(errflag, errno, rdfe)
	getn = y_getc;
	doed = rd_ed;
	doned = rd_ned;
	dotab = y_tab;
	dorevert = doend = donewrec = y_rnew;
	if(pars_f(fmtbuf)) err(errflag,F_ERFMT,rdfe)
	fmt_bg();
	return(OK);
}

y_getc()
{
	int ch;
	if(curunit->uend) return(EOF);
	if(curunit->url==1 || recpos++ < curunit->url)
	{
		if((ch=getc(cf))!=EOF)
		{
				return(ch);
		}
		if(feof(cf))
		{
			curunit->uend = YES;
			return(EOF);
		}
		err(errflag,errno,rdfe);
	}
	else return(' ');
}

y_rnew()
{	if(curunit->url != 1)
	{	fseek(cf,(long)curunit->url*(++recnum),0);
		recpos = reclen = cursor = 0;
	}
	return(OK);
}

y_rend()
{
	return(OK);
}
