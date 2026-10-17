/*
 * File: oarith.c
 *  Contents: div, minus, mod, mult, neg, number, plus, power
 */

#include "../h/rt.h"
#ifdef SUN
#include <math.h>
#include <signal.h>
#endif SUN

#ifdef NoOver
#define Add(x,y) (x + y)
#define Sub(x,y) (x - y)
#define Mpy(x,y) (x * y)
#else NoOver
#define Add(x,y) ckadd(x,y)
#define Sub(x,y) cksub(x,y)
#define Mpy(x,y) ckmul(x,y)
#endif NoOver

/*
 * x / y - divide y into x.
 */

OpDcl(div,2,"/")
   {
   register int t1, t2;
   union numeric n1, n2;

   /*
    * x and y must be numbers.
    */
   if ((t1 = cvnum(&Arg1, &n1)) == NULL)
      runerr(102, &Arg1);
   if ((t2 = cvnum(&Arg2, &n2)) == NULL)
      runerr(102, &Arg2);

   if (!(t1 == T_Real || t2 == T_Real)) {
      /*
       * x and y are both integers, just divide them and return the result.
       */
      if (n2.integer == 0L)
         runerr(201, &Arg2);
      Mkint(n1.integer / n2.integer, &Arg0);
      }
   else {
      /*
       * Either x or y or both is real, convert the real values to integers,
       *  divide them, and return the result.
       */
      if (!(t1 == T_Real))
         n1.real = n1.integer;
      if (!(t2 == T_Real))
         n2.real = n2.integer;
#ifdef ZeroDivide
      if (n2.real == 0.0)
         runerr(204,0);
#endif ZeroDivide
      mkreal(n1.real / n2.real, &Arg0);
#ifdef SUN
      if (((struct b_real *)BlkLoc(Arg0))->realval == HUGE)
         kill(getpid(),SIGFPE);
#endif SUN
      }
   Return;
   }


/*
 * x - y - subtract y from x.
 */

OpDcl(minus,2,"-")
   {
   register int t1, t2;
   union numeric n1, n2;
#ifndef NoOver
   extern long cksub();
#endif NoOver

   /*
    * x and y must be numeric.  Save the cvnum return values for later use.
    */
   if ((t1 = cvnum(&Arg1, &n1)) == NULL)
      runerr(102, &Arg1);
   if ((t2 = cvnum(&Arg2, &n2)) == NULL)
      runerr(102, &Arg2);

   if (!(t1 == T_Real || t2 == T_Real)) {
      /*
       * Both x and y are integers.  Perform integer subtraction and place
       *  the result in Arg0 as the return value.
       */
      Mkint(Sub(n1.integer, n2.integer), &Arg0);
      }
   else {
      /*
       * Either x or y is real, convert the other to a real, perform
       *  the subtraction and place the result in Arg0 as the return value.
       */
      if (!(t1 == T_Real))
         n1.real = n1.integer;
      if (!(t2 == T_Real))
         n2.real = n2.integer;
      mkreal(n1.real - n2.real, &Arg0);
      }
   Return;
   }


/*
 * x % y - take remainder of x / y.
 */

OpDcl(mod,2,"%")
   {
   register int t1, t2;
   union numeric n1, n2;

   /*
    * x and y must be numeric.  Save the cvnum return values for later use.
    */
   if ((t1 = cvnum(&Arg1, &n1)) == NULL)
      runerr(102, &Arg1);
   if ((t2 = cvnum(&Arg2, &n2)) == NULL)
      runerr(102, &Arg2);

   if (!(t1 == T_Real || t2 == T_Real)) {
      /*
       * Both x and y are integers.  If y is 0, generate an error because
       *  it's divide by 0.  Otherwise, just return the modulus of the
       *  two arguments.
       */
      if (n2.integer == 0L)
         runerr(202, &Arg2);
      Mkint(n1.integer % n2.integer, &Arg0);
      }
   else {
      /*
       * Either x or y is real, convert the other to a real, perform
       *  the modulation, convert the result to an integer and place it
       *  in Arg0 as the return value.
       */
      if (!(t1 == T_Real))
         n1.real = n1.integer;
      if (!(t2 == T_Real))
         n2.real = n2.integer;
      mkreal(n1.real - n2.real * (int)(n1.real / n2.real), &Arg0);
      }
   Return;
   }


/*
 * x * y - multiply x and y.
 */

OpDcl(mult,2,"*")
   {
   register int t1, t2;
   union numeric n1, n2;
#ifndef NoOver
   extern long ckmul();
#endif NoOver

   /*
    * x and y must be numeric.  Save the cvnum return values for later use.
    */
   if ((t1 = cvnum(&Arg1, &n1)) == NULL)
      runerr(102, &Arg1);
   if ((t2 = cvnum(&Arg2, &n2)) == NULL)
      runerr(102, &Arg2);

   if (!(t1 == T_Real || t2 == T_Real)) {
      /*
       * Both x and y are integers.  Perform the multiplication and
       *  and place the result in Arg0 as the return value.
       */
      Mkint(Mpy(n1.integer,n2.integer), &Arg0);
      }
   else {
      /*
       * Either x or y is real, convert the other to a real, perform
       *  the subtraction and place the result in Arg0 as the return value.
       */
      if (!(t1 == T_Real))
         n1.real = n1.integer;
      if (!(t2 == T_Real))
         n2.real = n2.integer;
      mkreal(n1.real * n2.real, &Arg0);
      }
   Return;
   }


/*
 * -x - negate x.
 */

OpDcl(neg,1,"-")
   {
   union numeric n;
   long l;

   /*
    * x must be numeric.
    */
   switch (cvnum(&Arg1, &n)) {

      case T_Integer:
      case T_Longint:
         /*
          * If it's an integer, check for overflow by negating it and
          *  seeing if the negation didn't "work".  Use Mkint to
          *  construct the return value.
          */
         l = -n.integer;
         if (n.integer < 0 && l < 0)
            runerr(203, &Arg1);
         Mkint(l, &Arg0);
         break;

      case T_Real:
         /*
          * x is real, just negate it and use mkreal to construct the
          *  return value.
          */
         mkreal(-n.real, &Arg0);
         break;

      default:
         /*
          * x isn't numeric.
          */
         runerr(102, &Arg1);
      }
   Return;
   }


/*
 * +x - convert x to numeric type.
 *  Operational definition: generate runerr if x is not numeric.
 */

OpDcl(number,1,"+")
   {
   union numeric n;

   switch (cvnum(&Arg1, &n)) {

      case T_Integer:
      case T_Longint:
         Mkint(n.integer, &Arg0);
         break;

      case T_Real:
         mkreal(n.real, &Arg0);
         break;

      default:
         runerr(102, &Arg1);
      }
   Return;
   }


/*
 * x + y - add x and y.
 */

OpDcl(plus,2,"+")
   {
   register int t1, t2;
   union numeric n1, n2;
#ifndef NoOver
   extern long ckadd();
#endif NoOver

   /*
    * x and y must be numeric.  Save the cvnum return values for later use.
    */
   if ((t1 = cvnum(&Arg1, &n1)) == NULL)
      runerr(102, &Arg1);
   if ((t2 = cvnum(&Arg2, &n2)) == NULL)
      runerr(102, &Arg2);

   if (!(t1 == T_Real || t2 == T_Real)) {
      /*
       * Both x and y are integers.  Perform integer addition and plcae the
       *  result in Arg0 as the return value.
       */
      Mkint(Add(n1.integer, n2.integer), &Arg0);
      }
   else {
      /*
       * Either x or y is real, convert the other to a real, perform
       *  the addition and place the result in Arg0 as the return value.
       */
      if (!(t1 == T_Real))
         n1.real = n1.integer;
      if (!(t2 == T_Real))
         n2.real = n2.integer;
      mkreal(n1.real + n2.real, &Arg0);
      }
   Return;
   }




/*
 * x ^ y - raise x to the y power.
 */

OpDcl(power,2,"^")
   {
   register int t1, t2;
   union numeric n1, n2;
   extern double pow();
   extern long ipow();

   /*
    * x and y must be numeric.  Save the cvnum return values for later use.
    */
   if ((t1 = cvnum(&Arg1, &n1)) == NULL)
      runerr(102, &Arg1);
   if ((t2 = cvnum(&Arg2, &n2)) == NULL)
      runerr(102, &Arg2);

   if (!(t1 == T_Real || t2 == T_Real)) {
      /*
       * Both x and y are integers.  Perform integer exponentiation
       *  and place the result in Arg0 as the return value.
       */
      Mkint(ipow(n1.integer, n2.integer), &Arg0);
      }
   else {
      /*
       * Either x or y is real, convert the other to a real, perform
       *  real exponentiation and place the result in Arg0 as the
       *  return value.
       */
      if (!(t1 == T_Real))
         n1.real = n1.integer;
      if (!(t2 == T_Real))
         n2.real = n2.integer;
      if (n1.real == 0.0 && n2.real <= 0.0)
         /*
          * Tried to raise zero to a negative power.
          */
         runerr(204, NULL);
      if (n1.real < 0.0 && t2 == T_Real)
         /*
          * Tried to raise a negative number to a real power.
          */
         runerr(206, NULL);
      mkreal(pow(n1.real,n2.real), &Arg0);
      }
   Return;
   }

long ipow(n1, n2)
long n1, n2;
   {
   long result;

   if (n1 == 0 && n2 <= 0)
      runerr(204, NULL);
   if (n2 < 0)
      return 0.0;
   result = 1L;
   while (n2 > 0) {
      if (n2 & 01L)
         result *= n1;
      n1 *= n1;
      n2 >>= 1;
      }
   return result;
   }
