#include "util.h"

char *
	gwdir ()                    /* what is current working directory? */
{
	static char path[1024];

	if (getwd (path) == 0)
		return ((char *)NOTOK);
	else
		return (path);
}
