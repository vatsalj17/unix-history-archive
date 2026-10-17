/*
**  FILEIO.H -- header for file input/output routines
**
**	History:
**		2/24/78 (eric) -- changed 'stdin' to be an extern *FILE
**			instead of a defined constant (part of
**			changing dropping _Stdinb if not used).
**		2/9/78 [eric] -- dropped _peek, added _size to FILE
**			struct.
**		12/29/77 -- introduced by eric
*/

# define	IOBUFSIZ	512		/* buffer size */
# define	_NFILES		20		/* number of open files */

/* io buffer definition */
struct _iobuf
{
	char	*_ptr;		/* ptr to next byte */
	int	_cnt;		/* count of bytes in buffer */
	char	*_base;		/* pointer to beginning of buffer */
	char	_flag;		/* flags about file */
	char	_file;		/* file descriptor */
	int	_size;		/* size of buffer */
};

# define	FILE		struct _iobuf

extern FILE	_Iob[_NFILES];		/* file headers */

/* flags */
# define	_IOREAD		01	/* read file */
# define	_IOWRITE	02	/* write file */
# define	_IOBSET		04	/* a buffer has been set */
# define	_IOEOF		020	/* at end of file */
# define	_IOERR		040	/* at i/o error */

/* error return for fopen() and freopen() */
# ifndef NULL
# define	NULL		0
# endif

/* error or end-of-file return for getc, etc. */
# define	EOF		(-1)

/* standard input and output */
# define	stdin		(&_Iob[0])
# define	stdout		(&_Iob[1])
# define	stderr		(&_Iob[2])

/* declare open routines */
extern FILE	*fopen();
extern FILE	*freopen();
