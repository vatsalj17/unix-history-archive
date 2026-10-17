#include	"stdio.h"

struct _iobuf *
_ntof(n)
register int n;
{
	register int i;
	for (i=0; i<_NFILE; i++)
		if (n == _iob[i]._file)
			return(&_iob[i]);
	return(NULL);
}

int	cin	0;
int	cout	1;

cclose(n)
{
	return(fclose(_ntof(n)));
}

ceof(n)
{
	return(feof(_ntof(n)));
}

cexit(x)
{
	exit(x);
}

cflush(n)
{
	return(fflush(_ntof(n)));
}

cgetc(n)
{
	register c;

	if ((c = getc(_ntof(n))) < 0)
		return(0);
	return(c);
}

copen(f, m)
char m;
{
	register struct _iobuf *iop;

	if ((iop = fopen(f, &m)) == NULL)
		return(-1);
	return(fileno(iop));
}

cputc(c, n)
{
	putc(c, _ntof(n));
}

cread(a, b, c, n)
{
	return(fread(a, b, c, _ntof(n)));
}

cwrite(a, b, c, n)
{
	return(fwrite(a, b, c, _ntof(n)));
}
getcharz()
{
	register c;

	if ((c = getc(_ntof(cin))) < 0)
		return(0);
	return(c);
}

printf(a, b, c, d)
{
	struct _iobuf _strbuf;
	if (a==-1) {
		_strbuf._flag = _IOWRT+_IOSTRG;
		_strbuf._ptr = b;
		_strbuf._cnt = 32767;
		_doprnt(c, &d, &_strbuf);
		putc('\0', &_strbuf);
	} else if (a<=10) {
		_doprnt(b, &c, _ntof(a));
	} else
		_doprnt(a, &b, _ntof(cout));
}

putcharz(c)
{
	return(putc(c, _ntof(cout)));
}

gets(s)
char *s;
{
	register c;
	register char *cs;

	cs = s;
	while ((c = getc(_ntof(cin))) != '\n' && c>=0)
		*cs++ = c;
	if (c<0 && cs==s)
		return(NULL);
	*cs++ = '\0';
	return(s);
}

puts(s)
char *s;
{
	register c;

	while (c = *s++)
		putc(c, _ntof(cout));
	putc('\n', _ntof(cout));
}

rew(n)
{
	rewind(_ntof(n));
}

scanf(a, b, c, d)
{
	struct _iobuf strbuf;
	register char *s;

	if (a == -1) {
		strbuf._flag = _IOREAD|_IOSTRG;
		strbuf._ptr = strbuf._base = b;
		strbuf._cnt = 0;
		s = b;
		while (*s++)
			strbuf._cnt++;
		return(_doscan(&strbuf, c, &d));
	} else if (a<=10)
		return(_doscan(_ntof(a), b, &c));
	else
		return(_doscan(_ntof(cin), a, &b));
}
