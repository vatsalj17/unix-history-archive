# include	<ingres.h>
# include	<func.h>
# include	<sccs.h>

SCCSID(@(#)sysdump.c	8.1	12/31/84)



char	Qbuf[400];
int	QbufSize = sizeof Qbuf;

extern struct fn_def	SysDmpFn;

struct fn_def	*FuncVect[] =
{
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
	&SysDmpFn,
};

int	NumFunc = sizeof FuncVect / sizeof FuncVect[0];

DESC	Btreesec;
int	Btree_fd;
