# include	"../fileio.h"

/*
**  GETPW -- get line from /etc/passwd file
**
**	Returns a line from /etc/passwd based on numeric user id.
**
**	Parameters:
**		uid -- numeric user id
**		buf -- a buffer for the line
**
**	Return:
**		zero -- success
**		one -- failure
**
**	Requires:
**		fopen() -- to open /etc/passwd
**		fclose() -- to close same
**		setbuf()
**		rewind()
**		fgetline() -- to read a line of passwd file.
**
**	Defines:
**		getpw()
**
**	History:
**		8/1/78 (eric) -- changed to use fgetline().
**		2/6/78 (eric) -- made passwd reads buffered (why didn't
**			I do this before??!!).
**		12/30/77 -- adapted from stdio lib by eric
*/

getpw(uid, buf)
int	uid;
char	buf[];
{
	static FILE	*pwf;
	register int	n;
	register int	c;
	register char	*bp;
	auto char	iobuf[IOBUFSIZ];

	if (pwf == NULL)
	{
		pwf = fopen("/etc/passwd", "r", iobuf);
	}
	else if (buf == NULL)
	{
		/* then close file */
		fclose(pwf);
		return (0);
	}
	else
		setbuf(pwf, iobuf, IOBUFSIZ);
	if (pwf == NULL)
		return (1);
	rewind(pwf);
	uid =& 0377;

	for (;;)
	{
		bp = buf;
		if (fgetline(bp, 999, pwf) == NULL)
			break;
		n = 3;
		while (--n)
			while ((c = *bp++) != ':')
				if (c == '\n')
					goto badexit;
		while ((c = *bp++) != ':')
		{
			if (c < '0' || c > '9')
				continue;
			n = n * 10 + c - '0';
		}
		if (n == uid)
		{
			setbuf(pwf, NULL);
			return (0);
		}
	}

badexit:
	setbuf(pwf, NULL);
	return (1);
}
