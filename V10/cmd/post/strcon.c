#include <varargs.h>

char *
strcon(dest, va_alist)
register char	*dest;
va_dcl
{
	register va_list ap;
	register char	*s;

	va_start(ap);
	while (s = va_arg(ap, char *)) {
		while (*dest++ = *s++);
		dest--;
	}
	va_end(ap);
	return dest;
}
