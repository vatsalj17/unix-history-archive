/*
 *	MATH
 *
 *	Miscellaneous math functions.
 *
 *	Ralph E. Griswold
 *
 *	Last modified 5/2/86
 *
 */

#include "../h/rt.h"
#include <errno.h>

int errno;
/*
 * exp(x)
 */
FncDcl(exp,1)
   {
   int t;
   double y;
   union numeric r;
   double exp();
   
   if ((t = cvreal(&Arg1, &r)) == NULL) runerr(102, &Arg1);
   y = exp(r.real);
   if (errno == ERANGE) runerr(252, NULL);
   mkreal(y,&Arg0);
   Return;
   }

/*
 * log(x)
 */
FncDcl(log,1)
   {
   int t;
   double y;
   union numeric r;
   double log();
   
   if ((t = cvreal(&Arg1, &r)) == NULL) runerr(102, &Arg1);
   y = log(r.real);
   if (errno == EDOM) runerr(251, NULL);
   mkreal(y,&Arg0);
   Return;
   }

/*
 * log10(x)
 */
FncDcl(log10,1)
   {
   int t;
   double y;
   union numeric r;
   double log10();
   
   if ((t = cvreal(&Arg1, &r)) == NULL) runerr(102, &Arg1);
   y = log10(r.real);
   if (errno == EDOM) runerr(251, NULL);
   mkreal(y,&Arg0);
   Return;
   }

/*
 * sqrt(x)
 */
FncDcl(sqrt,1)
   {
   int t;
   double y;
   union numeric r;
   double sqrt();
   
   if ((t = cvreal(&Arg1, &r)) == NULL) runerr(102, &Arg1);
   y = sqrt(r.real);
   if (errno == EDOM) runerr(251, NULL);
   mkreal(y,&Arg0);
   Return;
   }
