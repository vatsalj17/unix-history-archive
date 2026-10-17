/*
 * File: ocomp.c
 *  Contents: lexeq, lexge, lexgt, lexle, lexlt, lexne, numeq, numge,
 *		numgt, numle, numlt, numne, eqv, neqv
 */

#include "../h/rt.h"

/*
 * x == y - test if x is lexically equal to y.
 */

OpDcl(lexeq,2,"==")
   {
   register int t;
   word i;
   register char *s1, *s2;
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen];
   extern char *alcstr();

   /*
    * x and y must be strings.  Save the cvstr return value for y because
    *  y is the result (if any).
    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   if ((t = cvstr(&Arg2, sbuf2)) == NULL)
      runerr(103, &Arg2);

   /*
    * If the strings have different lengths they can't be equal
    */
   if (StrLen(Arg1) != StrLen(Arg2))
      Fail;

   /*
    * compare the strings
    */
   i = StrLen(Arg1);
   s1 = StrLoc(Arg1);
   s2 = StrLoc(Arg2);
   while (i--)
      if (*s1++ != *s2++)
         Fail;

   /*
    * Return y as the result of the comparison.  If y was converted to
    *  a string, a copy of it is allocated.
    */
   Arg0 = Arg2;
   if (t == Cvt) {
      strreq(StrLen(Arg0));
      StrLoc(Arg0) = alcstr(StrLoc(Arg0), StrLen(Arg0));
      }
   Return;
   }


/*
 * x >>= y - test if x is lexically greater than or equal to y.
 */

OpDcl(lexge,2,">>=")
   {
   register int t;
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen];
   extern char *alcstr();

   /*
    * x and y must be strings.  Save the cvstr return value for y because
    *  y is the result (if any).
    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   if ((t = cvstr(&Arg2, sbuf2)) == NULL)
      runerr(103, &Arg2);

   /*
    * lexcmp does the work.
    */
   if (lexcmp(&Arg1, &Arg2) < 0)
      Fail;

   /*
    * Return y as the result of the comparison.  If y was converted to
    *  a string, a copy of it is allocated.
    */
   Arg0 = Arg2;
   if (t == Cvt) {
      strreq(StrLen(Arg0));
      StrLoc(Arg0) = alcstr(StrLoc(Arg0), StrLen(Arg0));
      }
   Return;
   }


/*
 * x >> y - test if x is lexically greater than y.
 */

OpDcl(lexgt,2,">>")
   {
   register int t;
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen];
   extern char *alcstr();

   /*
    * x and y must be strings.  Save the cvstr return value for y because
    *  y is the result (if any).
    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   if ((t = cvstr(&Arg2, sbuf2)) == NULL)
      runerr(103, &Arg2);

   /*
    * lexcmp does the work.
    */
   if (lexcmp(&Arg1, &Arg2) <= 0)
      Fail;

   /*
    * Return y as the result of the comparison.  If y was converted to
    *  a string, a copy of it is allocated.
    */
   Arg0 = Arg2;
   if (t == Cvt) {
      strreq(StrLen(Arg0));
      StrLoc(Arg0) = alcstr(StrLoc(Arg0), StrLen(Arg0));
      }
   Return;
   }


/*
 * x <<= y - test if x is lexically less than or equal to y.
 */

OpDcl(lexle,2,"<<=")
   {
   register int t;
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen];
   extern char *alcstr();

   /*
    * x and y must be strings.  Save the cvstr return value for y because
    *  y is the result (if any).
    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   if ((t = cvstr(&Arg2, sbuf2)) == NULL)
      runerr(103, &Arg2);

   /*
    * lexcmp does the work.
    */
   if (lexcmp(&Arg1, &Arg2) > 0)
      Fail;

   /*
    * Return y as the result of the comparison.  If y was converted to
    *  a string, a copy of it is allocated.
    */
   Arg0 = Arg2;
   if (t == Cvt) {
      strreq(StrLen(Arg0));
      StrLoc(Arg0) = alcstr(StrLoc(Arg0), StrLen(Arg0));
      }
   Return;
   }


/*
 * x << y - test if x is lexically less than y.
 */

OpDcl(lexlt,2,"<<")
   {
   register int t;
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen];
   extern char *alcstr();

   /*
    * x and y must be strings.  Save the cvstr return value for y because
    *  y is the result (if any).
    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   if ((t = cvstr(&Arg2, sbuf2)) == NULL)
      runerr(103, &Arg2);

   /*
    * lexcmp does the work.
    */
   if (lexcmp(&Arg1, &Arg2) >= 0)
      Fail;

   /*
    * Return y as the result of the comparison.  If y was converted to
    *  a string, a copy of it is allocated.
    */
   Arg0 = Arg2;
   if (t == Cvt) {		/* string needs to be allocated */
      strreq(StrLen(Arg0));
      StrLoc(Arg0) = alcstr(StrLoc(Arg0), StrLen(Arg0));
      }
   Return;
   }


/*
 * x ~== y - test if x is lexically not equal to y.
 */

OpDcl(lexne,2,"~==")
   {
   register int t;
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen];
   extern char *alcstr();

   /*
    * x and y must be strings.  Save the cvstr return value for y because
    *  y is the result (if any).
    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   if ((t = cvstr(&Arg2, sbuf2)) == NULL)
      runerr(103, &Arg2);

   /*
    * lexcmp does the work.
    */
   if (lexcmp(&Arg1, &Arg2) == 0)
      Fail;

   /*
    * Return y as the result of the comparison.  If y was converted to
    *  a string, a copy of it is allocated.
    */
   Arg0 = Arg2;
   if (t == Cvt) {		/* string needs to be allocated */
      strreq(StrLen(Arg0));
      StrLoc(Arg0) = alcstr(StrLoc(Arg0), StrLen(Arg0));
      }
   Return;
   }


/*
 * x = y - test if x is numerically equal to y.
 */

OpDcl(numeq,2,"=")
   {

   if (numcmp(&Arg1, &Arg2, &Arg0) != 0)
      Fail;
   Return;
   }


/*
 * x >= y - test if x is numerically greater or equal to y.
 */

OpDcl(numge,2,">=")
   {

   if (numcmp(&Arg1, &Arg2, &Arg0) < 0)
      Fail;
   Return;
   }


/*
 * x > y - test if x is numerically greater than y.
 */

OpDcl(numgt,2,">")
   {

   if (numcmp(&Arg1, &Arg2, &Arg0) <= 0)
      Fail;
   Return;
   }


/*
 * x <= y - test if x is numerically less than or equal to y.
 */

OpDcl(numle,2,"<=")
   {

   if (numcmp(&Arg1, &Arg2, &Arg0) > 0)
      Fail;
   Return;
   }


/*
 * x < y - test if x is numerically less than y.
 */

OpDcl(numlt,2,"<")
   {

   if (numcmp(&Arg1, &Arg2, &Arg0) >= 0)
      Fail;
   Return;
   }


/*
 * x ~= y - test if x is numerically not equal to y.
 */

OpDcl(numne,2,"~=")

   {

   if (numcmp(&Arg1, &Arg2, &Arg0) == 0)
      Fail;
   Return;
   }


/*
 * x === y - test equivalence of x and y.
 */

OpDcl(eqv,2,"===")
   {

   /*
    * Let equiv do all the work, failing if equiv indicates non-equivalence.
    */
   if (!equiv(&Arg1, &Arg2))
      Fail;

   Arg0 = Arg2;
   Return;
   }


/*
 * x ~=== y - test inequivalence of x and y.
 */

OpDcl(neqv,2,"~===")
   {

   /*
    * equiv does all the work.
    */
   if (equiv(&Arg1, &Arg2))
      Fail;
   Arg0 = Arg2;
   Return;
   }
