# include	<ingres.h>

/*	@(#)externs.c	8.1	12/31/84	*/

test(map,n)
char	*map;
int	n;
{
	return ((map[n/BITS] & (1<<(n%BITS))) != 0);
}
char	Qbuf[1000];

extern struct fn_def	CopyFn;
extern struct fn_def	CreateFn;
extern struct fn_def	DstroyFn;
extern struct fn_def	HelpFn;
extern struct fn_def	DsplayFn;
extern struct fn_def	KsortFn;
extern struct fn_def	ModifyFn;
extern struct fn_def	PrintFn;
extern struct fn_def	ResetrFn;
extern struct fn_def	RmqmFn;
extern struct fn_def	RupdatFn;
extern struct fn_def	SaveFn;
extern struct fn_def	IndexFn;
extern struct fn_def	SysDmpFn;

struct fn_def	*FuncVect[] =
{
	&CreateFn,	/* 6 */
	&DstroyFn,	/* 7 */
	&RupdatFn,	/* 8 */
	&PrintFn,	/* 9 */
	&HelpFn,	/* 10 */
	&ResetrFn,	/* 11 */
	&CopyFn,	/* 12 */
	&SaveFn,	/* 13 */
	&ModifyFn,	/* 14 */
	&IndexFn,	/* 15 */
	&DsplayFn,	/* 16 */
	&SysDmpFn,	/* 17 -- unused */
	&RmqmFn,	/* 18 */
	&KsortFn,	/* 19 */
};

int	NumFunc = sizeof FuncVect / sizeof FuncVect[0];
