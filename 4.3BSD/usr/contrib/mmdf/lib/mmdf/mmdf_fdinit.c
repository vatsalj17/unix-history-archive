#include "util.h"
#include "mmdf.h"

extern	char *malloc();

int *regfdary;
int numfds = 16;	/* init to 16 as a failsafe value, -DPK- */

mmdf_fdinit()
{
    register int i;

#ifdef V4_2BSD
    numfds = getdtablesize();
#else
#ifdef _NFILE
    numfds = _NFILE;
#endif _NFILE
#endif V4_2BSD
    /*NOSTRICT*/
    regfdary = (int *)malloc(numfds * sizeof(int));
    regfdary[0] = 0; regfdary[1] = 1;
    for (i = numfds-1; i > 1; i--)
	regfdary[i] = NOTOK;
}
