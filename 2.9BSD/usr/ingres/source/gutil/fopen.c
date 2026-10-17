# include	"../ingres.h"
# include	"../aux.h"
# include	"../fileio.h"

extern FILE	*_open();

/*
**  FOPEN -- open file
**
**	Opens a file for buffered input or output.
**
**	Parameters:
**		name -- name of file to open
**		mode -- file mode (see _open)
**		buffer -- a pointer to a data buffer.  If zero, the
**			file is unbuffered.
**
**	Return:
**		NULL -- could not open file
**		otherwise -- returns a pointer to a FILE, to be used
**			with other functions.
**
**	Requires:
**		_open() -- to open file in specified mode
**		_Iob -- to find a free slot
**
**	Defines:
**		fopen()
**
**	History:
**		2/22/79 (eric) -- iop->_size & _cnt are now set
**			correctly initially, so that even 512 byte
**			boundaries are written.
**		12/29/77 -- originally written by eric
*/

FILE *
fopen(name, mode, buffer)
char	*name;
char	*mode;
char	*buffer;
{
	register FILE	*iop;

	for (iop = _Iob; (iop->_flag & (_IOREAD|_IOWRITE)) != 0; iop++)
		if (iop >= &_Iob[_NFILES])
			return (NULL);

	/* got a pointer for the io status */
	iop->_base = buffer;
	if (buffer != NULL)
		iop->_size = IOBUFSIZ;
	else
		iop->_size = 0;
	iop->_flag = _IOBSET;
	return (_open(name, mode, iop));
}



/*
**  FREOPEN -- reopen a stream
**
**	Used to close and open an i/o stream with the same file
**	pointer (which will usually be stdin, stdout, or stderr).
**
**	Parameters:
**		name -- name of file
**		mode -- file mode (see _open)
**		iop -- file pointer
**
**	Return:
**		NULL -- failed
**		iop -- succeeded
**
**	Requires:
**		fclose() -- to close the old file
**		_open() -- to open the new file
**
**	Defines:
**		freopen()
**
**	History:
**		12/30/77 -- written by eric
*/

FILE *
freopen(name, mode, iop1)
char	*name;
char	*mode;
FILE	*iop1;
{
	register FILE	*iop;

	iop = iop1;

	fclose(iop);

	return (_open(name, mode, iop));
}



/*
**  _OPEN -- real open
**
**	This routine actually opens a file.
**
**	Parameters:
**		name -- file name
**		mode -- file mode
**			"r" -- read
**			"w" -- write
**			"a" -- append
**			"m" -- modify (not yet implemented)
**		iop -- file pointer
**
**	Return:
**		NULL -- failure
**		iop -- success
**
**	Requires:
**		creat -- to create new files (for "write" mode)
**		open -- to open old files (for "read", "append")
**		lseek -- to find end of file (for "append")
**
**	Defines:
**		_open()
**		_Filemode -- file mode on creat() call.
**
**	Trace Flags:
**		98.0 -- print parameters on entrance.
**
**	History:
**		2/22/79 (eric) -- changed to set _cnt to _size,
**			not zero.
**		7/6/78 (eric) -- _Filemode changed to use FILEMODE.
**		2/16/78 [eric] -- trace flags added.
**		2/6/78 -- modified to clean up _flag by eric.
**		12/30/77 -- written by eric.
*/

int	_Filemode	FILEMODE;

FILE *
_open(name, mode1, iop1)
char	*name;
char	*mode1;
FILE	*iop1;
{
	register FILE	*iop;
	register int	fd;
	register char	*mode;

	iop = iop1;
	mode = mode1;

#	ifdef xATR3
	tTfp(98, 0, "_open(\"%s\",\"%s\",%u)\n", name, mode, iop);
#	endif

	fd = -1;
	if (*mode == 'r')
	{
		fd = open(name, 0);
	}
	else if (*mode == 'a')
	{
		if ((fd = open(name, 1)) >= 0)
			lseek(fd, 0, 2);
	}
	if (*mode == 'w' || (*mode == 'a' && fd < 0))
	{
		fd = creat(name, _Filemode);
	}

	/* check for successful open */
	if (fd < 0)
		return (NULL);

	/* set up the file status */
	iop->_file = fd;
	iop->_flag =& ~(_IOREAD|_IOWRITE|_IOERR|_IOEOF);
	if (*mode == 'r')
	{
		iop->_flag =| _IOREAD;
		iop->_cnt = 0;
	}
	else
	{
		iop->_flag =| _IOWRITE;
		iop->_cnt = iop->_size;
	}
	iop->_ptr = iop->_base;

	return (iop);
}
