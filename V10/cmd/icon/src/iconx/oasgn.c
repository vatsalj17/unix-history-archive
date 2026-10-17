/*
 * File: oasgn.c
 *  Contents: asgn, rasgn, rswap, swap
 */

#include "../h/rt.h"

/*
 * x := y - assign y to x.
 */

/* >asgn */
OpDcl(asgn,2,":=")
   {
   /*
    * Make sure that x is a variable.
    */
   if (Qual(Arg1) || !Var(Arg1))
      runerr(111, &Arg1);

   /*
    * The returned result is the variable to which assignment is being
    *  made.
    */
   Arg0 = Arg1;

   /*
    * All the work is done by doasgn.  Note that Arg1 is known
    *  to be a variable.
    */
   if (!doasgn(&Arg1, &Arg2))
      Fail;

   Return;
   }
/* <asgn */


/*
 * x <- y - assign y to x.
 * Reverses assignment if resumed.
 */

OpDcl(rasgn,2,"<-")
   {

   /*
    * x must be a variable.
    */
   if (Qual(Arg1) || !Var(Arg1))
      runerr(111, &Arg1);
   /*
    * The return value is the variable x, so make a copy of it before
    *  it is dereferenced.
    */
   Arg0 = Arg1;
   DeRef(Arg1);
   /*
    * Assign y to x and suspend.
    */
   if (!doasgn(&Arg0, &Arg2))
      Fail;
   Suspend;
   /*
    * <- has been resumed, reverse the assignment by assigning the old value
    *  of x (present as Arg1) back into x and fail.
    */
   doasgn(&Arg0, &Arg1);
   Fail;
   }



/*
 * x <-> y - swap values of x and y.
 * Reverses swap if resumed.
 */

OpDclV(rswap,2,"<->")
   {
   register union block *bp1, *bp2;
   word adj1, adj2;

   /*
    * x and y must be variables.
    */
   if (Qual(Arg1) || !Var(Arg1))
      runerr(111, &Arg1);
   if (Qual(Arg2) || !Var(Arg2))
      runerr(111, &Arg2);
   /*
    * Make copies of x and y as variables in Arg0 and Arg3.
    */
   Arg0 = Arg1;
   Arg3 = Arg2;
   adj1 = adj2 = 0;
   if (Arg1.dword == D_Tvsubs && Arg2.dword == D_Tvsubs) {
      bp1 = BlkLoc(Arg1);
      bp2 = BlkLoc(Arg2);
      if (VarLoc(bp1->tvsubs.ssvar) == VarLoc(bp2->tvsubs.ssvar)) {
         /*
          * x and y are both substrings of the same string, set
          *  adj1 and adj2 for use in locating the substrings after
          *  an assignment has been made.  If x is to the right of y,
          *  set adj1 := *x - *y, otherwise if y is to the right of x,
          *  set adj2 := *y - *x.  Note that the adjustment values may
          *  be negative.
          */
         if (bp1->tvsubs.sspos > bp2->tvsubs.sspos)
            adj1 = bp1->tvsubs.sslen - bp2->tvsubs.sslen;
         else if (bp2->tvsubs.sspos > bp1->tvsubs.sspos)
            adj2 = bp2->tvsubs.sslen - bp1->tvsubs.sslen;
            }
      }
   DeRef(Arg1);
   DeRef(Arg2);
   /*
    * Do x := y
    */
   if (!doasgn(&Arg0, &Arg2))
      Fail;
   if (adj2 != 0)
      /*
       * y is to the right of x and the assignment x := y has shifted
       *  the position of y.  Add adj2 to the position of y to account
       *  for the replacement of x by y.
       */
      BlkLoc(Arg2)->tvsubs.sspos += adj2;
   if (!doasgn(&Arg3, &Arg1))
      Fail;
   /*
    * Do y := x
    */
   if (adj1 != 0)
      /*
       * x is to the right of y and the assignment y := x has shifted
       *  the position of x.  Add adj2 to the position of x to account
       *  for the replacement of y by x.
       */
      BlkLoc(Arg1)->tvsubs.sspos += adj1;
   /*
    * Suspend x with the assignment in effect.
    */
   Suspend;
   /*
    * If resumed, the assignments are undone.  Note that the string position
    *  adjustments are identical to those done earlier.
    */
   if (!doasgn(&Arg0, &Arg1))        /* restore x */
      Fail;
   if (adj2 != 0)
      BlkLoc(Arg2)->tvsubs.sspos += adj2;
   if (!doasgn(&Arg3, &Arg2))        /* restore y */
      Fail;
   if (adj1 != 0)
      BlkLoc(Arg1)->tvsubs.sspos += adj1;
   Fail;
   }


/*
 * x :=: y - swap values of x and y.
 */

OpDclV(swap,2,":=:")
   {
   register union block *bp1, *bp2;
   word adj1, adj2;

   /*
    * x and y must be variables.
    */
   if (Qual(Arg1) || !Var(Arg1))
      runerr(111, &Arg1);
   if (Qual(Arg2) || !Var(Arg2))
      runerr(111, &Arg2);
   /*
    * Make copies of x and y as variables in Arg0 and Arg3.
    */
   Arg0 = Arg1;
   Arg3 = Arg2;
   adj1 = adj2 = 0;
   if (Arg1.dword == D_Tvsubs && Arg2.dword == D_Tvsubs) {
      bp1 = BlkLoc(Arg1);
      bp2 = BlkLoc(Arg2);
      if (VarLoc(bp1->tvsubs.ssvar) == VarLoc(bp2->tvsubs.ssvar)) {
         /*
	  * x and y are both substrings of the same string, set
	  *  adj1 and adj2 for use in locating the substrings after
	  *  an assignment has been made.  If x is to the right of y,
	  *  set adj1 := *x - *y, otherwise if y is to the right of x,
	  *  set adj2 := *y - *x.  Note that the adjustment values may
	  *  be negative.
	  */
         if (bp1->tvsubs.sspos > bp2->tvsubs.sspos)
            adj1 = bp1->tvsubs.sslen - bp2->tvsubs.sslen;
         else if (bp2->tvsubs.sspos > bp1->tvsubs.sspos)
            adj2 = bp2->tvsubs.sslen - bp1->tvsubs.sslen;
   	 }
      }
   DeRef(Arg1);
   DeRef(Arg2);
   /*
    * Do x := y
    */
   if (!doasgn(&Arg0, &Arg2))
      Fail;
   if (adj2 != 0)
      /*
       * y is to the right of x and the assignment x := y has shifted
       *  the position of y.  Add adj2 to the position of y to account
       *  for the replacement of x by y.
       */
      BlkLoc(Arg2)->tvsubs.sspos += adj2;
   /*
    * Do y := x
    */
   if (!doasgn(&Arg3, &Arg1))
      Fail;
   if (adj1 != 0)
      /*
       * x is to the right of y and the assignment y := x has shifted
       *  the position of x.  Add adj2 to the position of x to account
       *  for the replacement of y by x.
       */
      BlkLoc(Arg1)->tvsubs.sspos += adj1;
   Return;
   }
