/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(_dmdio, @(#)dmdio.h	1.1.1.2	(5/10/87));

#define	BUFSIZ	100
#define	_NFILE	8
# ifndef FILE
extern	struct	_iobuf {
	int	count;
	char	*cp;
	char	*base;
	char	flag;
	char	file;
} _iob[_NFILE];
# endif

#define jgetchar()	(wait(RCV),rcvchar())
#define exit()		fstx_exit()

#define	_IOREAD	01
#define	_IOWRT	02
#define _IOTTY	04
#define	_IOEOF	010
#define	_IOERR	020
#ifndef NULL
#define	NULL	((char *)0)
#endif
#define	FILE	struct _iobuf
#define	EOF	(-1)

#define	stdin	(&_iob[0])
#define	stdout	(&_iob[1])
#define	stderr	(&_iob[2])
#define	feof(p)		(((p)->flag & _IOEOF) != 0)
#define	ferror(p)	(((p)->flag & _IOERR) != 0)
#define isatty(p)	(((p)->flag & _IOTTY) != 0)
#define	fileno(p)	((p)->file)

FILE	*fopen();
FILE	*popen();
char	*fgets();
FILE	*freopen();

