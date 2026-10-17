#include "ded.h"

/* procedures to open an old .dlog file and to create a new one.
 * The .dlog file contains information which enables ded to
 * return to precisely the same editing position on restart -
 * note that in order to do this ded must always return to 'input'
 * mode after each 'w' or 'reallywrite' command, which sets up
 * the new .dlog file
 */

int do_dlog(name, startfn, iomode, iofn, amax)
char *name;
int (*startfn)();
int iomode;
int (*iofn)();
int *amax;
 { register int dlog;

    if ((dlog = (*startfn)(name, iomode)) == -1 ||
	(*iofn)(dlog, &topl, 2) != 2 ||
	(*iofn)(dlog, amax, 2) != 2 ||
	(*iofn)(dlog, &in_c, 4) != 4 ||
	(*iofn)(dlog, &edit_c, 4) != 4 ||
	(*iofn)(dlog, erow, ENOUGH) != ENOUGH ||
	(*iofn)(dlog, b_range, 26*2) != 26*2 ||
	(*iofn)(dlog, e_range, 26*2) != 26*2)
      return(-1);
    else
      return(dlog);
 }

extern int read(), write(), open(), creat();

int open_dlog(name)
 { register int dlog;
    int oldmaxl;

    if (dlog = do_dlog(name, open, 2, read, &oldmaxl))
      while (oldmaxl > maxl) inc_maxl();

    return(dlog);
 }

int creat_dlog(name)
char *name;
 { return(do_dlog(name, creat, 0666, write, &maxl)); }
