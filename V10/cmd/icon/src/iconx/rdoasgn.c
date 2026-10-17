/*
 * File: rdoasgn.c
 *  Contents: doasgn.c
 */

#include "../h/rt.h"

/*
 * doasgn - assign value of dp2 to variable dp1.
 *  Does the work for asgn, swap, rasgn, and rswap.
 */

/* >doasgn1 */
doasgn(dp1, dp2)
struct descrip *dp1, *dp2;
   {
   register word l1, l2;
   register union block *bp;
   register struct b_table *tp;
   int (*putf) ();
   union block *hook;
   long l3;
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen];
   extern struct descrip tended[];
   extern struct b_lelem *alclstb();
   extern char *alcstr();

   tended[1] = *dp1;
   tended[2] = *dp2;
   ntended = 2;

assign:

   if (!Tvar(tended[1]))
      *VarLoc(tended[1]) = tended[2];
   else switch (Type(tended[1])) {
/* <doasgn1 */
/* >doasgn2 */
         case T_Tvsubs:
            /*
             * An assignment is being made to a substring trapped
             *  variable.  The tended descriptors are used as
             *  follows:
             *
             *    tended[1] - the substring trapped variable
             *    tended[2] - the value to assign
             *    tended[3] - the string containing the substring
             *    tended[4] - the substring
             *    tended[5] - the result string
             */

            /*
             * Be sure that the value to be assigned is a string.
             */
            ntended = 5;
            DeRef(tended[2]);
            if (cvstr(&tended[2], sbuf1) == NULL)
               runerr(103, &tended[2]);

            /*
             * Be sure that the variable in the trapped variable points
             *  to a string.
             */
            tended[3] = BlkLoc(tended[1])->tvsubs.ssvar;
            DeRef(tended[3]);
            if (!Qual(tended[3]))
               runerr(103, &tended[3]);
            strreq(StrLen(tended[3]) + StrLen(tended[2]));

            /*
             * Get a pointer to the tvsubs block and make l1 a C-style
             *  index to the character that begins the substring.
             */
            bp = BlkLoc(tended[1]);
            l1 = bp->tvsubs.sspos - 1;

            /*
             * Make tended[4] a descriptor for the substring.
             */
            StrLen(tended[4]) = bp->tvsubs.sslen;
            StrLoc(tended[4]) = StrLoc(tended[3]) + l1;

            /*
             * Make l2 a C-style index to the character after the
             *  substring. If l2 is greater than the length of the
             *  substring, it is an error because the string being
             *  assigned will not fit.
             */
            l2 = l1 + StrLen(tended[4]);
            if (l2 > StrLen(tended[3]))
               runerr(205, NULL);

            /*
             * Form the result string.	First, copy the portion of the
             *  substring string to the left of the substring into the
             *  string space.
             */
            StrLoc(tended[5]) = alcstr(StrLoc(tended[3]), l1);

            /*
             * Copy the string to be assigned into the string space,
             *  effectively concatenating it.
             */
            alcstr(StrLoc(tended[2]), StrLen(tended[2]));

            /*
             * Copy the portion of the substring to the right of
             *  the substring into the string space, completing the
             *  result.
             */
            alcstr(StrLoc(tended[3]) + l2, StrLen(tended[3]) - l2);

            /*
             * Calculate the length of the new string.
             */
            StrLen(tended[5]) = StrLen(tended[3]) - StrLen(tended[4]) +
               StrLen(tended[2]);
            bp->tvsubs.sslen = StrLen(tended[2]);
            tended[1] = bp->tvsubs.ssvar;
            tended[2] = tended[5];

            /*
             * Everything is set up for the actual assignment.  Go
             *  back to the beginning of the routine to do it.
             */
            goto assign;
/* <doasgn2 */

/* >doasgn3 */
         case T_Tvtbl:
            /*
             *
             * The tended descriptors are used as follows:
             *
             *    tended[1] - the table element trapped variable
             *    tended[2] - the value to be assigned
             *    tended[3] - subscripting value
             */

            /*
             * Point bp to the trapped variable block; point tended[3]
             *  to the subscripting value; point tp to the table
             *  header block.
             */
            ntended = 3;
            bp = BlkLoc(tended[1]);

            if (bp->tvtbl.title == T_Telem) {
               /*
                * The trapped variable block already has been converted
                *  to a table element block.  Just assign to it and return.
                */
               bp->telem.tval = tended[2];
               ntended = 0;
               return 1;
               }
            tended[3] = bp->tvtbl.tref;
            tp = (struct b_table *) BlkLoc(bp->tvtbl.clink);

            /*
             * Get the hash number for the subscripting value and locate
             *  the chain that contains the element to which assignment is
             *  to be made.
             */
            l1 = bp->tvtbl.hashnum;
            l2 = SlotNum(l1,TSlots);
            bp = BlkLoc(tp->buckets[l2]);

            /*
             * Traverse the chain to see if the value is already in the
             *  table.  If it is there, assign to it and return.
             */
            hook = bp;
            while (bp != NULL && bp->telem.hashnum <= l1) {
              if (bp->telem.hashnum == l1 &&
                 equiv(&bp->telem.tref, &tended[3])) {
                    bp->telem.tval = tended[2];
                    ntended = 0;
                    return 1;
                    }
               hook = bp;
               bp = BlkLoc(bp->telem.clink);
               }

            /*
             * The value being assigned is new.  Increment the table size,
             *  convert the tvtbl to a telem, and link it into the chain.
             */
            tp->size++;
            if (hook == bp) {		/* it goes at front of chain */
               bp = BlkLoc(tended[1]);
               bp->telem.clink = tp->buckets[l2];
               BlkLoc(tp->buckets[l2]) = bp;
               tp->buckets[l2].dword = D_Telem;
               }

            else {			/* it follows hook */
               bp = BlkLoc(tended[1]);
               bp->telem.clink = hook->telem.clink;
               BlkLoc(hook->telem.clink) =  bp;
               hook->telem.clink.dword = D_Telem;
               }

            bp->tvtbl.title = T_Telem;
            bp->telem.tval = tended[2];
            ntended = 0;
            return 1;
/* <doasgn3 */

/* >doasgn4 */
         case T_Tvkywd:
            ntended = 2;
            putf = BlkLoc(tended[1])->tvkywd.putval;
            if ((*putf)(&tended[2]) == NULL) {
               ntended = 0;
               return 0;
               }
            ntended = 0;
            return 1;
/* <doasgn4 */


         default:
            syserr("doasgn: illegal trapped variable");
         }

   ntended = 0;
   return 1;
   }
