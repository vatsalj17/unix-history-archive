# include  "d_returns.h"

/*  4 Sep 81  D. Crocker        getuid() was 11-dependent

/*
 *     D_INIT
 *
 *     routine which is called to do any initialization that's required
 *     before the rest of the routines can be used.  all that's done here
 *     now is to get the user's name and path.
 */

d_init()
    {
    extern char  d_uname[], d_upath[];
    extern int  d_errno;
    int  effecid;
    int  realuid;

    getwho (&realuid, &effecid);

    if (d_getuser(realuid, d_uname, d_upath) < 0)
      {
      d_errno = D_INITERR;
      return(D_FATAL);
      }

    return(D_OK);
    }
