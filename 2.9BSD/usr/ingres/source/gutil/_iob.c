# include	"../fileio.h"

/*
**  _IOB.C -- definitions of global variables, etc.
**
**	Requires:
**		nothing
**
**	Defines:
**		_Iob (FILE[]) -- input/output buffers
**
**	History:
**		1/17/79 (eric) -- dropped automatic buffering of
**			standard input.  Changed name from _stdin.c
**			to _iob.c.
**		1/9/79 (eric) -- ordering with respect to _stdin.o
**			changed in library.
**		2/27/78 (eric) -- changed to drop automatic standard
**			output buffering.
**		2/24/78 -- split from "data.c" to be "_stdin.c" and
**			"_fstdin.c" so that _Stdinb will not be loaded
**			if not used (as is the common case in INGRES).
**		2/9/78 -- modified by eric for the "_getbuf" routine
**			and the new FILE struct.
**		12/30/77 -- written by eric
*/


FILE	_Iob[_NFILES]
{
	NULL,	0,	NULL,	_IOREAD|_IOBSET,	0,	0,
	NULL,	0,	NULL,	_IOWRITE|_IOBSET,	1,	0,
	NULL,	0,	NULL,	_IOWRITE|_IOBSET,	2,	0,
};
