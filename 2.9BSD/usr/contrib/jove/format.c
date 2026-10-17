/* Jonathan Payne at Lincoln-Sudbury Regional High School 5-25-83

   format.c

   contains procedures that call _doprnt */

#include "jove.h"

/* VARARGS2 */

format(buf, fmt, args)
char	*buf,
	*fmt;
int	*args;
{
	IOBUF	strbuf;

	strbuf.io_flag = 0;
	strbuf.io_base = strbuf.io_ptr = buf;
	strbuf.io_cnt = 32767;
	_doprnt(fmt, args, &strbuf);
	Putc('\0', &strbuf);
}

/* VARARGS1 */

char *
sprint(fmt, args)
char	*fmt;
{
	static char line[100];

	format(line, fmt, &args);
	return line;
}

/* VARARGS1 */

char *
printf(fmt, args)
char	*fmt;
{
	_doprnt(fmt, &args, &termout);
}

/* VARARGS2 */

char *
sprintf(str, fmt, args)
char	*str,
	*fmt;
{
	format(str, fmt, &args);
	return str;
}

/* VARARGS1 */

s_mess(fmt, args)
char	*fmt;
{
	if (Input)
		return;
	format(mesgbuf, fmt, &args);
	message(mesgbuf);
}

_strout(string, count, adjust, file, fillch)
register char *string;
register int count;
int adjust;
register IOBUF	*file;
{

	while (adjust < 0) {
		if (*string=='-' && fillch=='0') {
			Putc(*string++, file);
			count--;
		}
		Putc(fillch, file);
		adjust++;
	}
	while (--count >= 0)
		Putc(*string++, file);
	while (adjust) {
		Putc(fillch, file);
		adjust--;
	}
}
