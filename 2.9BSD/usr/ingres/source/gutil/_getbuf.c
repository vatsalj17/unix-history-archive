# include	"../fileio.h"

/*
**  _GETBUF -- Get Internal I/O Buffer
**
**	This routine arranges for every file (even "unbuffered") to
**	have a buffer, even if it is only one character.  It will do
**	nothing on files that have already had a buffer set.
**
**	There are a lot of very serious error conditions which
**	are not checked.  Notably, if _base != NULL and _ptr == NULL,
**	something will blow up sometime.  This can "never happen".
**
**	For compatibility with the stdio library, we ought to
**	fetch a buffer by calling "alloc()" when _IOBSET is clear;
**	the third parameter to "fopen" (for the buffer) should then
**	be dropped, and fopen should not set the _IOBSET bit.
**	The current technique is better for us though, because we want
**	to keep better track of memory.
**
**	Parameters:
**		iop1 -- the file pointer of the interesting file.
**
**	Returns:
**		none
**
**	Side Effects:
**		The *iop struct will be updated.
**
**	Requires:
**		none
**
**	Called By:
**		_filbuf, _flsbuf, ungetc
**
**	Diagnostics:
**		none
**
**	Syserrs:
**		none (but probably should be)
**
**	History:
**		2/27/78 (eric) -- dropped automatic buffering on stan-
**			dard output.  Also, it was very nearly com-
**			pletely rewritten for efficiency reasons.
**		2/5/78 -- written by eric
*/

_getbuf(iop1)
FILE	*iop1;
{
	register FILE	*iop;
	static char	smallbuf[_NFILES];

	iop = iop1;

	/* check for buffer already set */
	if (iop->_base != NULL)
		return;

	/* unbuffered */
	iop->_base = iop->_ptr = &smallbuf[iop->_file];
	iop->_cnt = iop->_size = 1;

	return;
}
