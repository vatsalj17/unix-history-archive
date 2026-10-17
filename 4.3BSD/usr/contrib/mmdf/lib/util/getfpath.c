#include "util.h"

getfpath (fnam, dflpth, dest)   /* build full pathname              */
    char fnam[],
	 dflpth[];
    char *dest;
{
    if (fnam[0] == '/')       /* fully specified                      */
	(void) strcpy (dest, fnam);
    else
	sprintf (dest, "%s/%s", dflpth, fnam);
}


char *
	dupfpath (fnam, dflpth) /* build full pathname & alloc str  */
    char fnam[],
	 dflpth[];
{
    extern char *strdup ();
    char temppath[128];

    getfpath (fnam, dflpth, temppath);

    return (strdup (temppath));
}
