/*
 * File: oset.c
 *  Contents: compl, diff, inter, unions
 */

#include "../h/rt.h"

/*
 * ~x - complement cset x.
 */

/* >compl */
OpDcl(compl,1,"~")
   {
   register int i, j;
   union block *bp;
   int *cs, csbuf[CsetSize];
   extern struct b_cset *alccset();

   blkreq((word)sizeof(struct b_cset));

   /*
    * Arg1 must be a cset.
    */
   if (cvcset(&Arg1, &cs, csbuf) == NULL)
      runerr(104, &Arg1);

   /*
    * Allocate a new cset and then copy each cset word from Arg1 
    *  itno the new cset words, complementing each.
    */
   bp = (union block *) alccset(0);
   for (i = 0; i < CsetSize; i++) {
       bp->cset.bits[i] = ~cs[i];
       }
      j = 0;
      for (i = 0; i < CsetSize * CIntSize; i++) {
         if (Testb(i, bp->cset.bits))
            j++;
         }
   bp->cset.size = j;

   Arg0.dword = D_Cset;
   BlkLoc(Arg0) = bp;
   Return;
   }
/* <compl */


/*
 * x -- y - difference of csets x and y or of sets x and y.
 */

OpDcl(diff,2,"--")
   {
   register word i, j;
   register union block *bp;
   int *cs1, *cs2, csbuf1[CsetSize], csbuf2[CsetSize];
   extern struct b_cset *alccset();
   struct descrip *dp;
   struct b_set *srcp, *dstp, *tstp;
   struct b_selem *sep;
   extern struct b_set *alcset();
   extern struct b_selem *alcselem();

   if (Qual(Arg1) || Qual(Arg2))
      goto skipsets;
   if (Arg1.dword == D_Set && Arg2.dword != D_Set)
      runerr(119,&Arg2);
   if (Arg2.dword == D_Set && Arg1.dword != D_Set)
      runerr(119,&Arg1);
   if (Arg1.dword == D_Set && Arg2.dword == D_Set) {
      /*
       * Both x and y are sets - do set difference
       *  get enough space for a new set the size of x.
       */
      blkreq(sizeof(struct b_set) + BlkLoc(Arg1)->set.size *
	 sizeof(struct b_selem));
      /*
       * For each element in set x if it isn't in set y
       *  copy it directly into the result set.
       */
      srcp = (struct b_set *) BlkLoc(Arg1);
      tstp = (struct b_set *) BlkLoc(Arg2);
      Arg0.dword = D_Set;
      dstp = alcset();
      BlkLoc(Arg0) = (union block *) dstp;
      for (i = 0; i < SSlots; i++) {
	 sep = (struct b_selem *) BlkLoc(srcp->sbucks[i]);
	 dp = &dstp->sbucks[i];
	 while (sep != NULL) {
	    if ( !locate((struct b_selem *)BlkLoc(tstp->sbucks[i]), sep) ) {
	       dp->dword = D_Selem;
	       BlkLoc(*dp) = (union block *) alcselem(&sep->setmem, sep->hashnum);
	       dp = &BlkLoc(*dp)->selem.clink;
	       dstp->size++;
	       }
	    sep = (struct b_selem *) BlkLoc(sep->clink);
	    }
	 }
      }
   else {
      skipsets:
   blkreq((word)sizeof(struct b_cset));

   /*
    * x and y must be csets.
    */
   if (cvcset(&Arg1, &cs1, csbuf1) == NULL)
      runerr(104, &Arg1);
   if (cvcset(&Arg2, &cs2, csbuf2) == NULL)
      runerr(104, &Arg2);

   /*
    * Allocate a new cset and in each word of it, compute the value
    *  of the bitwise difference of the corresponding words in the
    *  x and y csets.
    */
   bp = (union block *) alccset(0);
   for (i = 0; i < CsetSize; i++) {
      bp->cset.bits[i] = cs1[i] & ~cs2[i];
      }
      j = 0;
      for (i = 0; i < CsetSize*CIntSize; i++) {
	 if (Testb(i,bp->cset.bits))
	    j++;
	 }
   bp->cset.size = j;

   Arg0.dword = D_Cset;
   BlkLoc(Arg0) = bp;
   }
   Return;
   }


/*
 * x ** y - intersection of csets x and y or of sets x and y.
 */

OpDcl(inter,2,"**")
   {
   register word i, j;
   union block *bp;
   int *cs1, csbuf1[CsetSize], *cs2, csbuf2[CsetSize];
   extern struct b_cset *alccset();
   struct descrip *dp;
   struct b_set *srcp, *tstp, *dstp;
   struct b_selem *sep;
   extern struct b_set *alcset();
   extern struct b_selem *alcselem();

   if (Qual(Arg1) || Qual(Arg2))
      goto skipsets;
   if (Arg1.dword == D_Set && Arg2.dword != D_Set)
      runerr(119,&Arg2);
   if (Arg2.dword == D_Set && Arg1.dword != D_Set)
      runerr(119,&Arg1);
   if (Arg1.dword == D_Set && Arg2.dword == D_Set) {
      /*
       * Both x and y are sets - do set intersection
       *  get enough space for a new set the size of the smaller
       *  of the two sets.
       */
      blkreq(sizeof(struct b_set) + Min(BlkLoc(Arg1)->set.size,
	 BlkLoc(Arg2)->set.size) * sizeof(struct b_selem));
      /*
       * Using the smaller of the two sets as the source
       *  copy directly into the result each of its elements
       *  that are also members of the other set.
       */
      if (BlkLoc(Arg1)->set.size <= BlkLoc(Arg2)->set.size) {
	 srcp = (struct b_set *) BlkLoc(Arg1);
	 tstp = (struct b_set *) BlkLoc(Arg2);
	 }
      else {
	 srcp = (struct b_set *) BlkLoc(Arg2);
	 tstp = (struct b_set *) BlkLoc(Arg1);
	 }
      Arg0.dword = D_Set;
      dstp = alcset();
      BlkLoc(Arg0) = (union block *) dstp;
      for (i = 0; i < SSlots; i++) {
	 sep = (struct b_selem *) BlkLoc(srcp->sbucks[i]);
	 dp = &dstp->sbucks[i];
	 while (sep != NULL) {
	    if (locate((struct b_selem *)BlkLoc(tstp->sbucks[i]), sep)) {
	       dp->dword = D_Selem;
	       BlkLoc(*dp) = (union block *) alcselem(&sep->setmem, sep->hashnum);
	       dp = &BlkLoc(*dp)->selem.clink;
	       dstp->size++;
	       }
	    sep = (struct b_selem *) BlkLoc(sep->clink);
	    }
	 }
      }
      else {
	 skipsets:
   blkreq((word)sizeof(struct b_cset));

   /*
    * x and y must be csets.
    */
   if (cvcset(&Arg1, &cs1, csbuf1) == NULL)
      runerr(104, &Arg1);
   if (cvcset(&Arg2, &cs2, csbuf2) == NULL)
      runerr(104, &Arg2);

   /*
    * Allocate a new cset and in each word of it, compute the value
    *  of the bitwise intersection of the corresponding words in the
    *  x and y csets.
    */
   bp = (union block *) alccset(0);
   for (i = 0; i < CsetSize; i++) {
      bp->cset.bits[i] = cs1[i] & cs2[i];
      }
      j = 0;
      for (i = 0; i < CsetSize*CIntSize; i++) {
	 if (Testb(i,bp->cset.bits))
	    j++;
	 }
   bp->cset.size = j;

   Arg0.dword = D_Cset;
   BlkLoc(Arg0) = bp;
   }
   Return;
   }


/*
 * x ++ y - union of csets x and y or of sets x and y.
 */

OpDcl(unions,2,"++")
   {
   register word i, j;
   union block *bp;
   int *cs1, *cs2, csbuf1[CsetSize], csbuf2[CsetSize];
   extern struct b_cset *alccset();
   int res;
   struct b_set *srcp, *tstp, *dstp;
   struct b_selem *ep;
   struct descrip *dp, *hook;
   extern struct b_set *alcset();
   extern struct b_selem *alcselem();
   extern struct descrip *memb();

   if (Qual(Arg1) || Qual(Arg2))
      goto skipsets;
   if (Arg1.dword == D_Set && Arg2.dword != D_Set)
      runerr(119,&Arg2);
   if (Arg2.dword == D_Set && Arg1.dword != D_Set)
      runerr(119,&Arg1);
   if (Arg1.dword == D_Set && Arg2.dword == D_Set) {
      /*
       *  Both x and y are sets - do set union
       *  get enough space for a set as big as x + y.
       */
	 blkreq(sizeof(struct b_set) + (BlkLoc(Arg1)->set.size +
	    BlkLoc(Arg2)->set.size) * sizeof(struct b_selem));
	 /*
	  *  Select the larger of the two sets as the source
	  *  copy each element to a new set for the result
	  *  then insert each member of the second set into the
	  *  result set if it is not already there.
	  */
	 if (BlkLoc(Arg1)->set.size >= BlkLoc(Arg2)->set.size) {
	    srcp = (struct b_set *) BlkLoc(Arg1);
	    tstp = (struct b_set *) BlkLoc(Arg2);
	    }
	 else {
	    srcp = (struct b_set *) BlkLoc(Arg2);
	    tstp = (struct b_set *) BlkLoc(Arg1);
	    }
	 Arg0.dword = D_Set;
	 dstp = alcset();
	 BlkLoc(Arg0) = (union block *) dstp;
	 for (i = 0; i < SSlots; i++) {
	    ep = (struct b_selem *) BlkLoc(srcp->sbucks[i]);
	    dp = &dstp->sbucks[i];
	    while (ep != NULL) {
	       dp->dword = D_Selem;
	       BlkLoc(*dp) = (union block *) alcselem(&ep->setmem, ep->hashnum);
	       dp = &BlkLoc(*dp)->selem.clink;
	       dstp->size++;
	       ep = (struct b_selem *) BlkLoc(ep->clink);
	       }
	    }
	 for (i = 0; i < SSlots; i++) {
	    ep = (struct b_selem *) BlkLoc(tstp->sbucks[i]);
	    while (ep != NULL) {
	       hook = memb(dstp, &ep->setmem, ep->hashnum, &res);
	       if (res == 0)
		  addmem(dstp, alcselem(&ep->setmem,  ep->hashnum), hook);
	       ep = (struct b_selem *) BlkLoc(ep->clink);
	       }
	    }
	 }
      else {
      skipsets:

   blkreq((word)sizeof(struct b_cset));

   /*
    * x and y must be csets.
    */
   if (cvcset(&Arg1, &cs1, csbuf1) == NULL)
      runerr(104, &Arg1);
   if (cvcset(&Arg2, &cs2, csbuf2) == NULL)
      runerr(104, &Arg2);

   /*
    * Allocate a new cset and in each word of it, compute the value
    *  of the bitwise union of the corresponding words in the
    *  x and y csets.
    */
   bp = (union block *) alccset(0);
   for (i = 0; i < CsetSize; i++) {
      bp->cset.bits[i] = cs1[i] | cs2[i];
      }
      j = 0;
      for (i = 0; i < CsetSize*CIntSize; i++) {
	 if (Testb(i,bp->cset.bits))
	    j++;
	 }
   bp->cset.size = j;

   Arg0.dword = D_Cset;
   BlkLoc(Arg0) = bp;
   }
   Return;
   }
