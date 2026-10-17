/*
 * This is take from "send/s_do.c".
 */

#include "./mmdf.h"

char *adrptr;

next_address (addr)
char    *addr;
{
    int     i = -1;               /* return -1 = end; 0 = empty         */

    for (;;)
	switch (*adrptr)
	{
	    default: 
		addr[++i] = *adrptr++;
		continue;

	    case '"':
		for (addr[++i] = *adrptr++; !isnull (*adrptr); )
		{
		    addr[++i] = *adrptr;
		    if (*adrptr++ == '"')
			break;
		}
		continue;

	    case '(':
		for (addr[++i] = *adrptr++; !isnull (*adrptr); )
		{
		    addr[++i] = *adrptr;
		    if (*adrptr++ == ')')
			break;
		}
		continue;

	    case '\n':
	    case ',': 
		addr[++i] = '\0';
		adrptr++;
		return (i);

	    case '\0': 
		if (i >= 0)
		    addr[++i] = '\0';
		return (i);
	}
}
