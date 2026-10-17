# include	<func.h>
# include	<sccs.h>

SCCSID(@(#)monitor.c	8.2	12/18/85)



char	Qbuf[1600];
int	QbufSize = sizeof Qbuf;

extern struct fn_def	TtyMonFn;

struct fn_def	*FuncVect[] =
{
	&TtyMonFn,
};

int	NumFunc = sizeof FuncVect / sizeof FuncVect[0];
