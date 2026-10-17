/*
 *	TRIG
 *
 *	Trigonometric functions
 *
 *	Ralph E. Griswold and Stephen B. Wampler
 *
 *	Last modified 5/2/86 by Ralph E. Griswold
 *
 */

#include "../h/rt.h"
#include <errno.h>

int errno;

/*
 * sin(x), x in radians
 */
FncDcl(sin,1)
   {
   int t;
   union numeric r;
   double sin();
   
   if ((t = cvreal(&Arg1, &r)) == NULL) runerr(102, &Arg1);
   mkreal(sin(r.real),&Arg0);
   Return;
   }

/*
 * cos(x), x in radians
 */
FncDcl(cos,1)
   {
   int t;
   union numeric r;
   double cos();
   
   if ((t = cvreal(&Arg1, &r)) == NULL) runerr(102, &Arg1);
   mkreal(cos(r.real),&Arg0);
   Return;
   }

/*
 * tan(x), x in radians
 */
FncDcl(tan,1)
   {
   int t;
   double y;
   union numeric r;
   double tan();
   
   if ((t = cvreal(&Arg1, &r)) == NULL) runerr(102, &Arg1);
   y = tan(r.real);
   if (errno == ERANGE) runerr(252, NULL);
   mkreal(y,&Arg0);
   Return;
   }

/*
 * acos(x), x in radians
 */
FncDcl(acos,1)
   {
   int t;
   double y;
   union numeric r;
   double acos();
   
   if ((t = cvreal(&Arg1, &r)) == NULL) runerr(102, &Arg1);
   y = acos(r.real);
   if (errno == EDOM) runerr(251, NULL);
   mkreal(y,&Arg0);
   Return;
   }

/*
 * asin(x), x in radians
 */
FncDcl(asin,1)
   {
   int t;
   double y;
   union numeric r;
   double asin();
   
   if ((t = cvreal(&Arg1, &r)) == NULL) runerr(102, &Arg1);
   y = asin(r.real);
   if (errno == EDOM) runerr(251, NULL);
   mkreal(y,&Arg0);
   Return;
   }

/*
 * atan(x), x in radians
 */
FncDcl(atan,1)
   {
   int t;
   union numeric r;
   double atan();
   
   if ((t = cvreal(&Arg1, &r)) == NULL) runerr(102, &Arg1);
   mkreal(atan(r.real),&Arg0);
   Return;
   }

/*
 * atan2(x,y), x, y in radians
 */
FncDcl(atan2,2)
   {
   int t;
   union numeric r1, r2;
   double atan2();

   if ((t = cvreal(&Arg2, &r2)) == NULL) runerr(102, &Arg2);
   if ((t = cvreal(&Arg1, &r1)) == NULL) runerr(102, &Arg1);
   mkreal(atan2(r1.real,r2.real),&Arg0);
   Return;
   }

#define PI 3.14159

/*
 * dtor(x), x in degrees
 */
FncDcl(dtor,1)
   {
   union numeric r;

   if (cvreal(&Arg1, &r) == NULL) runerr(102, &Arg1);
   mkreal(r.real * PI / 180, &Arg0);
   Return;
   }

/*
 * rtod(x), x in radians
 */
FncDcl(rtod,1)
   {
   union numeric r;

   if (cvreal(&Arg1, &r) == NULL) runerr(102, &Arg1);
   mkreal(r.real * 180 / PI, &Arg0);
   Return;
   }
