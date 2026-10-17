/*
 * File: lscan.c
 *  Contents: bscan, escan
 */

#include "../h/rt.h"

/*
 * bscan - set &subject and &pos upon entry to a scanning expression.
 *
 *  Arguments are:
 *	cargp[-2] - new value for &subject
 *	cargp[-1] - saved value of &subject
 *	cargp[0]  - saved value of &pos
 */

#define newsubj (cargp[-2])
#define savsubj (cargp[-1])
#define savpos	(cargp[0])

LibDcl(bscan,2,"?")
   {
   char sbuf[MaxCvtLen];
   extern char *alcstr();
   struct descrip tsubject;

   /*
    * Make a copy of the value for &subject and convert it to a string.
    */
   tsubject = newsubj;
   DeRef(tsubject);
   switch (cvstr(&tsubject, sbuf)) {
      case NULL:
	 runerr(103, &tsubject);

      case Cvt:
	 /*
	  * The new value for &subject wasn't a string.  Allocate the
	  *  new value and fall through.
	  */
	 strreq(StrLen(tsubject));
	 StrLoc(tsubject) = alcstr(StrLoc(tsubject), StrLen(tsubject));

      case NoCvt:
	 /*
	  * Establish a new &subject value and set &pos to 1.
	  */
	 k_subject = tsubject;
	 k_pos = 1;
      }
   Suspend;

   /*
    * bscan has been resumed. Restore the old &subject and &pos values
    *  and fail.
    */
   k_subject = savsubj;
   k_pos = IntVal(savpos);
   Fail;
   }


/*
 * escan - restore &subject and &pos at the end of a scanning expression.
 *
 *  Arguments:
 *    Arg0 - value being scanned
 *    Arg1 - old value of &subject
 *    Arg2 - old value of &pos
 *    Arg3 - result of the scanning expression
 *
 * The result of the scanning expression is dereferenced if it refers to &subject
 *  or &pos, then copied to the first argument (the last three will
 *  be popped when escan returns).  Then the previous values of &subject
 *  and &pos are restored.
 *
 * Escan suspends once it has restored the old &subject; on failure
 *  the new &subject and &pos are "unrestored", and the failure is
 *  propagated into the using clause.
 */

LibDcl(escan,3,"escan")
   {
   struct descrip tmp;

   /*
    * If the result of the scanning expression is &subject or &pos,
    *  it is dereferenced.
    */
   if (((word)BlkLoc(Arg3) == (word)&tvky_sub) || ((word)BlkLoc(Arg3) == (word)&tvky_pos))
      DeRef(Arg3);

   /*
    * Copy the result of the scanning expression into Arg0, which will
    *  be the result of the scan.
    */
   Arg0 = Arg3;

   /*
    * Swap new and old values of &subject, leaving the new value in Arg1.
    */
   tmp = k_subject;
   k_subject = Arg1;
   Arg1 = tmp;

   /*
    * Swap new and old values of &pos, leaving the new value in Arg2.
    */
   tmp = Arg2;
   IntVal(Arg2) = k_pos;
   k_pos = IntVal(tmp);

   /*
    * Suspend the value of the scanning expression.
    */
   Suspend;

   /*
    * Upon resumption, restore the new values for &subject and &pos
    *  and fail.
    */
   k_subject = Arg1;
   k_pos = IntVal(Arg2);

   Fail;
   }
