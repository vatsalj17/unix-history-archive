#
/* rjestat.h */


/* This information is proprietary and is the property of Bell
   Telephone Laboratories, Incorporated.  Its reproduction or
   disclosure to others, either orally or in writing, is pro-
   hibited without written permission of Bell Laboratories. */


IDMOD() {return (IDSTR (1.2)");};

#define SIGH 1
#define SIGI 2
#define SIGQ 3
#define SIGK 9

int flag;

#define FDSP 1
#define FINT 2
#define FBKS 4
#define FSTA 8

int dash,plus;

char pid[8];

int rtim[2],itim[2];

struct lins
 {char *hst,*sys,*dir,*pfx,*dev,*phne;};

int nlin;

#define NLIN 6

struct lins *line[NLIN];

char trt[128];

/*end*/
