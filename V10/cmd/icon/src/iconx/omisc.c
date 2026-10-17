/*
 * File: omisc.c
 *  Contents: random, refresh, size, tabmat, toby
 */


#include "../h/rt.h"
#define RandVal (RanScale*(k_random=(RandA*k_random+RandC)&MaxLong))

/*
 * ?x - produce a randomly selected element of x.
 */

OpDclV(random,1,"?")
   {
   register word val, i, j;
   register union block *bp;
   long r1;
   char sbuf[MaxCvtLen];
   union block *ep;
   struct descrip *dp;
   double rval;
   extern char *alcstr();

   Arg2 = Arg1;
   DeRef(Arg1);

   if (Qual(Arg1)) {
      /*
       * x is a string, produce a random character in it as the result.
       *  Note that a substring trapped variable is returned.
       */
      if ((val = StrLen(Arg1)) <= 0)
         Fail;
      blkreq((word)sizeof(struct b_tvsubs));
      rval = RandVal;			/* This form is used to get around */
      rval *= val;			/* a bug in a certain C compiler */
      mksubs(&Arg2, &Arg1, (word)rval + 1, (word)1, &Arg0);
      Return;
      }

   switch (Type(Arg1)) {
      case T_Cset:
         /*
          * x is a cset.  Convert it to a string, select a random character
          *  of that string and return it.  Note that a substring trapped
          *  variable is not needed.
          */
         cvstr(&Arg1, sbuf);
         if ((val = StrLen(Arg1)) <= 0)
            Fail;
         strreq((word)1);
         StrLen(Arg0) = 1;
         rval = RandVal;
         rval *= val;
         StrLoc(Arg0) = alcstr(StrLoc(Arg1)+(word)rval, (word)1);
         Return;


      case T_List:
         /*
          * x is a list.  Set i to a random number in the range [1,*x],
          *  failing if the list is empty.
          */
         bp = BlkLoc(Arg1);
         val = bp->list.size;
         if (val <= 0)
            Fail;
         rval = RandVal;
         rval *= val;
         i = (word)rval + 1;
         j = 1;
         /*
          * Work down chain list of list blocks and find the block that
          *  contains the selected element.
          */
         bp = BlkLoc(BlkLoc(Arg1)->list.listhead);
         while (i >= j + bp->lelem.nused) {
            j += bp->lelem.nused;
            if ((bp->lelem.listnext).dword != D_Lelem)
            syserr("list reference out of bounds in random");
            bp = BlkLoc(bp->lelem.listnext);
            }
         /*
          * Locate the appropriate element and return a variable
          * that points to it.
          */
         i += bp->lelem.first - j;
         if (i >= bp->lelem.nelem)
            i -= bp->lelem.nelem;
         dp = &bp->lelem.lslots[i];
         Arg0.dword = D_Var + ((word *)dp - (word *)bp);
         VarLoc(Arg0) = dp;
         Return;

      case T_Table:
          /*
           * x is a table.  Set i to a random number in the range [1,*x],
           *  failing if the table is empty.
           */
         bp = BlkLoc(Arg1);
         val = bp->table.size;
         if (val <= 0)
            Fail;
         rval = RandVal;
         rval *= val;
         i = (word)rval + 1;
         /*
          * Work down the chain of elements in each bucket and return
          *  a variable that points to the i'th element encountered.
          */
         for (j = 0; j < TSlots; j++) {
            for (ep = BlkLoc(bp->table.buckets[j]); ep != NULL;
		     ep = BlkLoc(ep->telem.clink)) {
		if (--i <= 0) {
		   dp = &ep->telem.tval;
		   Arg0.dword = D_Var + ((word *)dp - (word *)bp);
		   VarLoc(Arg0) = dp;
		   Return;
		   }
		}
             }
      case T_Set:
         /*
          * x is a set.  Set i to a random number in the range [1,*x],
          *  failing if the set is empty.
          */
         bp = BlkLoc(Arg1);
         val = bp->set.size;
         if (val <= 0)
            Fail;
         rval = RandVal;
         rval *= val;
         i = (word)rval + 1;
         /*
          * Work down the chain of elements in each bucket and return
          *  the value of the ith element encountered.
          */
         for (j = 0; j < SSlots; j++) {
            for (ep = BlkLoc(bp->set.sbucks[j]); ep != NULL;
               ep = BlkLoc(ep->selem.clink)) {
		  if (--i <= 0) {
		     Arg0 = ep->selem.setmem;
		     Return;
		     }
		 }
             }

      case T_Record:
         /*
          * x is a record.  Set val to a random number in the range [1,*x]
          *  (*x is the number of fields), failing if the record has no
          *  fields.
          */
         bp = BlkLoc(Arg1);
         val = BlkLoc(bp->record.recdesc)->proc.nfields;
         if (val <= 0)
            Fail;
         /*
          * Locate the selected element and return a variable
          * that points to it
          */
            rval = RandVal;
            rval *= val;
            dp = &bp->record.fields[(word)rval];
            Arg0.dword = D_Var + ((word *)dp - (word *)bp);
            VarLoc(Arg0) = dp;
            Return;

      default:
         /*
          * Try converting it to an integer
          */
      switch (cvint(&Arg1, &r1)) {

         case T_Longint:
            runerr(205, &Arg1);

         case T_Integer:
            /*
             * x is an integer, be sure that it's non-negative.
             */
            val = (word)r1;
            if (val < 0)
               runerr(205, &Arg1);
         getrand:
            /*
             * val contains the integer value of x.  If val is 0, return
             *	a real in the range [0,1), else return an integer in the
             *	range [1,val].
             */
            if (val == 0) {
               rval = RandVal;
               mkreal(rval, &Arg0);
               }
            else {
               rval = RandVal;
               rval *= val;
               Mkint((long)rval + 1, &Arg0);
               }
            Return;

         default:
            /*
             * x is of a type for which random generation is not supported
             */
            runerr(113, &Arg1);
            }
         }
   }


/*
 * ^x - return an entry block for co-expression x from the refresh block.
 */

OpDcl(refresh,1,"^")
   {
   register struct b_coexpr *sblkp;
   register struct b_refresh *rblkp;
   register struct descrip *dp, *dsp;
   register word *newsp;
   int na, nl, i;
   extern struct b_coexpr *alcstk();
   extern struct b_refresh *alceblk();

   /*
    * Be sure a co-expression is being refreshed.
    */
   if (Qual(Arg1) || Arg1.dword != D_Coexpr)
      runerr(118, &Arg1);

   /*
    * Get a new co-expression stack and initialize.
    */
   sblkp = alcstk();
   sblkp->activator = nulldesc;
   sblkp->size = 0;
   sblkp->nextstk = stklist;
   stklist = sblkp;
   sblkp->freshblk = BlkLoc(Arg1)->coexpr.freshblk;
   /*
    * Icon stack starts at word after co-expression stack block.  C stack
    *  starts at end of stack region on machines with down-growing C stacks
    *  and somewhere in the middle of the region.
    *
    * The C stack is aligned on a doubleword boundary.	For upgrowing
    *  stacks, the C stack starts in the middle of the stack portion
    *  of the static block.  For downgrowing stacks, the C stack starts
    *  at the last word of the static block.
    */
   newsp = (word *)((char *)sblkp + sizeof(struct b_coexpr));
#ifdef UpStack
   sblkp->cstate[0] =
      ((word)((char *)sblkp + (stksize - sizeof(*sblkp))/2)
       &~(WordSize*2-1));
#else
   sblkp->cstate[0] =
	((word)((char *)sblkp + stksize - WordSize)&~(WordSize*2-1));
#endif UpStack
   sblkp->es_argp = (struct descrip *)newsp;
   /*
    * Get pointer to refresh block and get number of arguments and locals.
    */
   rblkp = (struct b_refresh *)BlkLoc(sblkp->freshblk);
   na = (rblkp->pfmkr).pf_nargs + 1;
   nl = rblkp->numlocals;

   /*
    * Copy arguments onto new stack.
    */
   dp = &rblkp->elems[0];
   dsp = (struct descrip *)newsp;
   for (i = 1; i <= na; i++)
      *dsp++ = *dp++;

   /*
    * Copy procedure frame to new stack and point dsp to word after frame.
    */
   *((struct pf_marker *)dsp) = rblkp->pfmkr;
   sblkp->es_pfp = (struct pf_marker *)dsp;
   dsp = (struct descrip *)((word *)dsp + Vwsizeof(*pfp));
   sblkp->es_ipc = rblkp->ep;
   sblkp->es_gfp = 0;
   sblkp->es_efp = 0;
   sblkp->tvalloc = NULL;
   sblkp->es_ilevel = 0;

   /*
    * Copy locals to new stack and refresh block.
    */
   for (i = 1; i <= nl; i++)
      *dsp++ = *dp++;

   /*
    * Push two null descriptors on the stack.
    */
   *dsp++ = nulldesc;
   *dsp++ = nulldesc;

   sblkp->es_sp = (word *)dsp - 1;

   /*
    * Establish line and file values and clear location for transmitted value.
    */
   sblkp->es_line = line;

   /*
    * Return the new co-expression.
    */
   Arg0.dword = D_Coexpr;
   BlkLoc(Arg0) = (union block *) sblkp;
   Return;
   }


/*
 * *x - return size of string or object x.
 */

/* >size */
OpDcl(size,1,"*")
   {
   char sbuf[MaxCvtLen];

   Arg0.dword = D_Integer;
   if (Qual(Arg1)) {
      /*
       * If Arg1 is a string, return the length of the string.
       */
      IntVal(Arg0) = StrLen(Arg1);
      }
   else {
      /*
       * Arg1 is not a string.  For most types, the size is in the size
       *  field of the block.  For records, it is in an auxiliary
       *  structure.
       */
      switch (Type(Arg1)) {
         case T_List:
            IntVal(Arg0) = BlkLoc(Arg1)->list.size;
            break;

         case T_Table:
            IntVal(Arg0) = BlkLoc(Arg1)->table.size;
            break;

         case T_Set:
            IntVal(Arg0) = BlkLoc(Arg1)->set.size;
            break;

         case T_Cset:
            IntVal(Arg0) = BlkLoc(Arg1)->cset.size;
            break;

         case T_Record:
            IntVal(Arg0) = BlkLoc(BlkLoc(Arg1)->record.recdesc)->proc.nfields;
            break;

         case T_Coexpr:
            IntVal(Arg0) = BlkLoc(Arg1)->coexpr.size;
            break;

         default:
            /*
             * Try to convert it to a string.
             */
            if (cvstr(&Arg1, sbuf) == NULL)
               runerr(112, &Arg1);		/* no notion of size */
            IntVal(Arg0) = StrLen(Arg1);
         }
      }
   Return;
   }
/* <size */

/*
 * =x - tab(match(x)).
 * Reverses effects if resumed.
 */

OpDcl(tabmat,1,"=")
   {
   register word l;
   register char *s1, *s2;
   word i, j;
   char sbuf[MaxCvtLen];

   /*
    * x must be a string.
    */
   if (cvstr(&Arg1,sbuf) == NULL)
      runerr(103, &Arg1);

   /*
    * Make a copy of &pos.
    */
   i = k_pos;

   /*
    * Fail if &subject[&pos:0] is not of sufficient length to contain x.
    */
   j = StrLen(k_subject) - i + 1;
   if (j < StrLen(Arg1))
      Fail;

   /*
    * Get pointers to x (s1) and &subject (s2).  Compare them on a bytewise
    *  basis and fail if s1 doesn't match s2 for *s1 characters.
    */
   s1 = StrLoc(Arg1);
   s2 = StrLoc(k_subject) + i - 1;
   l = StrLen(Arg1);
   while (l-- > 0) {
      if (*s1++ != *s2++)
         Fail;
      }

   /*
    * Increment &pos to tab over the matched string and suspend the
    *  matched string.
    */
   l = StrLen(Arg1);
   k_pos += l;
   Arg0 = Arg1;
   Suspend;

   /*
    * tabmat has been resumed, restore &pos and fail.
    */
   k_pos = i;
   if (k_pos > StrLen(k_subject) + 1)
      runerr(205, &tvky_pos.kyval);
   Fail;
   }


/*
 * i to j by k - generate successive values.
 */

/* >toby */
OpDcl(toby,3,"toby")
   {
   long from, to, by;

   /*
    * Arg1 (from), Arg2 (to), and Arg3 (by) must be integers.
    *  Also, Arg3 must not be zero.
    */
   if (cvint(&Arg1, &from) == NULL)
      runerr(101, &Arg1);
   if (cvint(&Arg2, &to) == NULL)
      runerr(101, &Arg2);
   if (cvint(&Arg3, &by) == NULL)
      runerr(101, &Arg3);
   if (by == 0)
      runerr(211, &Arg3);

   /*
    * Count up or down (depending on relationship of from and to) and
    *  suspend each value in sequence, failing when the limit has been
    *  exceeded.
    */
   while ((from <= to && by > 0) || (from >= to && by < 0)) {
      Mkint(from, &Arg0);
      Suspend;
      from += by;
      }
   Fail;
   }
/* <toby */
