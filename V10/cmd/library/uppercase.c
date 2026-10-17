/*	sdp:uppercase.c	1.4	*/
#include "sccsid.h"
VERSION(@(#)sdp:uppercase.c	1.4)


/********** uppercase.c  *********/

/* simply a function to make sure a string is all uppercase
 */
#include <ctype.h>

uppercase(str)
register char *str;
 {
	register int i;
	for (i=0; *str != '\0'; i++, str++)
		if (islower(*str)) *str = toupper(*str);
	return(i);
	}
