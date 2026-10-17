# include	<func.h>
# include	<sccs.h>


SCCSID(@(#)sysmonitor.c	8.1	12/31/84)



char	Qbuf[400];
int	QbufSize = sizeof Qbuf;

extern struct fn_def	SysTtyMonFn;

struct fn_def	*FuncVect[] =
{
	&SysTtyMonFn,
};

int	NumFunc = sizeof FuncVect / sizeof FuncVect[0];
