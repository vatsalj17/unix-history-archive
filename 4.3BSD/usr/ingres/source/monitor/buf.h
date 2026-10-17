#

/*
**  BUF.H -- buffer definitions
**
**	Version:
**		@(#)buf.h	8.1	12/31/84
*/

# define	BUFSIZE		256

struct buf
{
	struct buf	*nextb;
	char		buffer[BUFSIZE];
	char		*ptr;
};

# ifndef NULL
# define	NULL	0
# endif
