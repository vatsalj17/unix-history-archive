#include	"../fileio.h"

/*
**  Tooled for INGRES
*/

_strout(string, count, adjust, file, fillch)
register char *string;
register count;
int adjust;
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
