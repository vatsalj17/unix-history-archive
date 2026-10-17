#include "util.h"
#include "mmdf.h"

/* stdio output  of n characters, skipping nulls */

fnput (str, count, fp)
    register char *str;
    register int   count;
    register FILE *fp;
{
    char *base = str;

    for (; count-- > 0; str++) {
	if (isnull (*str)) {
	    if ((str - base) > 0)
		fwrite(base, sizeof(char), str - base, fp);
	    base = str+1;
	}
    }
    if ((str - base) > 0)
	fwrite(base, sizeof(char), str - base, fp);
}
