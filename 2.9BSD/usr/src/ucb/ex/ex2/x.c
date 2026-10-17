char	xstr[];
#line 1 "xprintf.c"









#line 13 "xprintf.c"


printf(fmt, args)
char *fmt;
{
	_doprnt(fmt, &args, 0);
}

_strout(string, count, adjust, file, fillch)
register char *string;
register count;
int adjust;
register struct _iobuf *file;
{
	while (adjust < 0) {
		if (*string=='-' && fillch=='0') {
			putchar(*string++);
			count--;
		}
		putchar(fillch);
		adjust++;
	}
	while (--count>=0)
		putchar(*string++);
	while (adjust) {
		putchar(fillch);
		adjust--;
	}
}
