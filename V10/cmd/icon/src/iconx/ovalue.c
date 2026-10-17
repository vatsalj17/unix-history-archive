/*
 * File: ovalue.c
 *  Contents: nonnull, null, value
 */

#include "../h/rt.h"

/*
 * \x - test x for nonnull value.
 */

OpDcl(nonnull,1,"\\")
   {

   /*
    * If x is not null, it is returned, otherwise, the function fails.
    *  Because the pre-dereference value of x is the return value (if
    *  any), x is copied into Arg0.
    */
   Arg0 = Arg1;
   DeRef(Arg1);
   if (ChkNull(Arg1))
      Fail;
   Return;
   }


/*
 * /x - test x for null value.
 */

OpDcl(null,1,"/")
   {

   /*
    * If x is null, it is returned, otherwise, the function fails.
    *  Because the pre-dereference value of x is the return value (if
    *  any), x is copied into Arg0.
    */
   Arg0 = Arg1;
   DeRef(Arg1);
   if (!ChkNull(Arg1))
      Fail;
   Return;
   }


/*
 * .x - produce value of x by dereferencing it.
 */

OpDcl(value,1,".")
   {

   Arg0 = Arg1;
   Return;
   }
