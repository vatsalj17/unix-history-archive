# include	<ingres.h>
# include	<aux.h>
# include	<access.h>
# include	<sccs.h>
# include 	<func.h>

SCCSID(@(#)sysmodfunc.c	8.1	12/31/84)

/*
**  Configuration table for SYSMOD
*/

char	Qbuf[10000];
int	QbufSize = sizeof Qbuf;

extern struct fn_def	KsortFn;
extern struct fn_def	ModifyFn;
extern struct fn_def	SysFuncFn;


struct fn_def	*FuncVect[] =
{
	&SysFuncFn,
	&ModifyFn,	
	&KsortFn,
};

int	NumFunc = sizeof FuncVect / sizeof FuncVect[0];

