#include	"mac.h"
#include	"mac.x"


warning(str)
register char *str;
{
	if (OPTION('e'))
		return;

	if (OPTION('l'))
		newpage();
	if (OPTION('d'))
		putchar('\n');

	printf("%4d: warning: %s\n", nline, str);
	return;
}

synerr(ptr)
register char *ptr;
{
	errcount++;
	length = 0;				/* no instruction */

	if (OPTION('e'))
		return;

	if (OPTION('l'))
		newpage();
	if (OPTION('d'))
		putchar('\n');

	printf("%4d: %s\n", nline, ptr);
	return;
}
