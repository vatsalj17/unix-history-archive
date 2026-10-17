/*
 * File: ocat.c
 *  Contents: cat, lconcat
 */

#include "../h/rt.h"

/*
 * x || y - concatenate strings x and y.
 */

/* >cat */
OpDcl(cat,2,"||")
   {
   char sbuf1[MaxCvtLen];	/* buffers for conversion to string */
   char sbuf2[MaxCvtLen];
   extern char *alcstr();

   /*
    *  Convert arguments to strings if necessary.
    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   if (cvstr(&Arg2, sbuf2) == NULL)
      runerr(103, &Arg2);

   /*
    * Ensure space for the resulting string.
    */
   strreq(StrLen(Arg1) + StrLen(Arg2));

   if (StrLoc(Arg1) + StrLen(Arg1) == strfree)
      /*
       * The end of Arg1 is at the end of the string space.  Hence, Arg1
       *  was the last string allocated.  Arg1 is not copied.  Instead,
       *  Arg2 is appended to the string space and the result is pointed
       *  to the start of Arg1.
       */
      StrLoc(Arg0) = StrLoc(Arg1);
   else
      /*
       * Otherwise, append Arg1 to the end of the string space and point
       *  the result to the start of Arg1.
       */
      StrLoc(Arg0) = alcstr(StrLoc(Arg1),StrLen(Arg1));
   /*
    * Append Arg2 to the end of the string space.
    */
   alcstr(StrLoc(Arg2),StrLen(Arg2));
   /*
    *  Set the length of the result and return.
    */
   StrLen(Arg0) = StrLen(Arg1) + StrLen(Arg2);
   Return;
   }
/* <cat */


/*
 * x ||| y - concatenate lists x and y.
 */

OpDcl(lconcat,2,"|||")
   {
   register struct b_list *bp1, *bp2;
   register struct b_lelem *lp1, *lp2;
   word size1, size2;

   /*
    * x and y must be lists.
    */
   if (Qual(Arg1) || Arg1.dword != D_List)
      runerr(108, &Arg1);
   if (Qual(Arg2) || Arg2.dword != D_List)
      runerr(108, &Arg2);

   /*
    * Get the size of both lists.
    */
   size1 = BlkLoc(Arg1)->list.size;
   size2 = BlkLoc(Arg2)->list.size;

   /*
    * Make a copy of both lists.
    */
   cplist(&Arg1, &Arg1, (word)1, size1 + 1);
   cplist(&Arg2, &Arg2, (word)1, size2 + 1);

   /*
    * Get a pointer to both lists.  bp1 points to the copy of x and is
    *  the list that will be returned.
    */
   bp1 = (struct b_list *) BlkLoc(Arg1);
   bp2 = (struct b_list *) BlkLoc(Arg2);

   /*
    * Perform the concatenation by hooking the lists together so
    *  that the next list of x is y and the previous list of y is x.
    */
   lp1 = (struct b_lelem *) BlkLoc(bp1->listtail);
   lp2 = (struct b_lelem *) BlkLoc(bp2->listhead);

   lp1->listnext.dword = D_Lelem;
   BlkLoc(lp1->listnext) = (union block *) lp2;

   lp2->listprev.dword = D_Lelem;
   BlkLoc(lp2->listprev) = (union block *) lp1;

   /*
    * Adjust the size field to reflect the length of the concatenated lists.
    */
   bp1->size = size1 + size2;
   BlkLoc(bp1->listtail) = BlkLoc(bp2->listtail);

   Arg0 = Arg1;
   Return;
   }
