#include "util.h"
#include "mmdf.h"
/*
 *  EAN call of MMDF logging
 */

extern LLog *logptr;

eanlog (format, a0, a1, a2, a3, a4, a5,  a6, a7, a8, a9)
char *format, *a0, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9;
{
     ll_log (logptr, LLOGFST,  format,
		a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}
