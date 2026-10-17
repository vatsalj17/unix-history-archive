/*
 * File: fstruct.c
 *  Contents: delete, get, insert, member, pop, pull, push, put, sort
 */

#include "../h/rt.h"

/*
 * delete(S,x) - delete element x from set S if it is there
 *  (always succeeds and returns S).
 */

FncDcl(delete,2)
   {
   register struct descrip *pd;
   int res ;
   extern struct descrip *memb();

   Arg0 = Arg1;

   if (Arg1.dword != D_Set)
       runerr(119,&Arg1);

      /*
      * The technique and philosophy here are the same
      *  as used in insert - see comment there.
      */
   pd = memb((struct b_set *)BlkLoc(Arg1),&Arg2,hash(&Arg2),&res);
   if (res == 1) {
      /*
      * The element is there so delete it.
      */
      *pd = BlkLoc(*pd)->selem.clink;
      (BlkLoc(Arg1)->set.size)--;
      }
   Return;
   }


/*
 * get(x) - get an element from end of list x.
 *  Identical to pop(x).
 */

FncDcl(get,1)
   {
   register word i;
   register struct b_list *hp;
   register struct b_lelem *bp;
   extern struct b_lelem *alclstb();

   /*
    * x must be a list.
    */
   if (Arg1.dword != D_List)
      runerr(108, &Arg1);

   /*
    * Fail if the list is empty.
    */
   hp = (struct b_list *) BlkLoc(Arg1);
   if (hp->size <= 0)
      Fail;

   /*
    * Point bp at the first list block.  If the first block has no
    *  elements in use, point bp at the next list block.
    */
   bp = (struct b_lelem *) BlkLoc(hp->listhead);
   if (bp->nused <= 0) {
      bp = (struct b_lelem *) BlkLoc(bp->listnext);
      BlkLoc(hp->listhead) = (union block *) bp;
      bp->listprev = nulldesc;
      }
   /*
    * Locate first element and assign it to Arg0 for return.
    */
   i = bp->first;
   Arg0 = bp->lslots[i];
   /*
    * Set bp->first to new first element, or 0 if the block is now
    *  empty.  Decrement the usage count for the block and the size
    *  of the list.
    */
   if (++i >= bp->nelem)
      i = 0;
   bp->first = i;
   bp->nused--;
   hp->size--;
   Return;
   }


/*
 * insert(S,x) - insert element x into set S if not already there
 *  (always succeeds and returns S).
 */

FncDcl(insert,2)
   {
   register struct descrip *pd;
   register word hn;
   int res;
   extern struct b_selem *alcselem();
   extern struct descrip *memb();

   Arg0 = Arg1;

   if (Arg1.dword != D_Set)
      runerr(119,&Arg1);

      /*
      * We may need at most one new element.
      */
   blkreq((word)sizeof(struct b_selem));
   hn = hash(&Arg2);
   /*
    * If Arg2 is a member of set Arg1 then res will have the
    *  value 1 and pd will have a pointer to the descriptor
    *  that points to that member.
    *  If Arg2 is not a member of the set then res will have
    *  the value 0 and pd will point to the descriptor
    *  which should point to the member - thus we know where
    *  to link in the new element without having to do any
    *  repetitive looking.
    */
   pd = memb((struct b_set *)BlkLoc(Arg1),&Arg2,hn,&res);
   if (res == 0)
      /*
      * The element is not in the set - insert it.
      */
      addmem((struct b_set *)BlkLoc(Arg1),alcselem(&Arg2,hn),pd);
   Return;
   }


/*
 * member(S,x) - returns x if x is a member of set S otherwise fails.
 */

FncDcl(member,2)
   {
   int res;
   extern struct descrip *memb();

   if (Arg1.dword != D_Set)
      runerr(119,&Arg1);	/* S is not a set  */

   /* If Arg2 is a member of set Arg1 then "res" will have the
    * value 1 otherwise it will have the value 0.
    */
   memb((struct b_set *)BlkLoc(Arg1),&Arg2,hash(&Arg2),&res);
   if (res == 1) {		/* It is a member. */
      Arg0 = Arg2;		/* Return the member if it is in Arg1. */
      Return;
      }
   Fail;
   }


/*
 * pop(x) - pop an element from beginning of list x.
 */

/* >pop */
FncDcl(pop,1)
   {
   register word i;
   register struct b_list *hp;
   register struct b_lelem *bp;
   extern struct b_lelem *alclstb();

   /*
    * Arg1 must be a list.
    */
   if (Arg1.dword != D_List)
      runerr(108, &Arg1);

   /*
    * Fail if the list is empty.
    */
   hp = (struct b_list *) BlkLoc(Arg1);
   if (hp->size <= 0)
      Fail;

   /*
    * Point bp to the first list block.  If the first block has no
    *  elements in use, point bp at the next list block.
    */
   bp = (struct b_lelem *) BlkLoc(hp->listhead);
   if (bp->nused <= 0) {
      bp = (struct b_lelem *) BlkLoc(bp->listnext);
      BlkLoc(hp->listhead) = (union block *) bp;
      bp->listprev = nulldesc;
      }
   /*
    * Locate first element and assign it to Arg0 for return.
    */
   i = bp->first;
   Arg0 = bp->lslots[i];

   /*
    * Set bp->first to new first element, or 0 if the block is now
    *  empty.  Decrement the usage count for the block and the size
    *  of the list.
    */
   if (++i >= bp->nelem)
      i = 0;
   bp->first = i;
   bp->nused--;
   hp->size--;
   Return;
   }
/* <pop */


/*
 * pull(x) - pull an element from end of list x.
 */

FncDcl(pull,1)
   {
   register word i;
   register struct b_list *hp;
   register struct b_lelem *bp;
   extern struct b_lelem *alclstb();

   /*
    * x must be a list.
    */
   if (Arg1.dword != D_List)
      runerr(108, &Arg1);

   /*
    * Point at list header block and fail if the list is empty.
    */
   hp = (struct b_list *) BlkLoc(Arg1);
   if (hp->size <= 0)
      Fail;
   /*
    * Point bp at the last list element block.  If the last block has no
    *  elements in use, point bp at the previous list element block.
    */
   bp = (struct b_lelem *) BlkLoc(hp->listtail);
   if (bp->nused <= 0) {
      bp = (struct b_lelem *) BlkLoc(bp->listprev);
      BlkLoc(hp->listtail) = (union block *) bp;
      bp->listnext = nulldesc;
      }
   /*
    * Set i to position of last element and assign the element to
    *  Arg0 for return.  Decrement the usage count for the block
    *  and the size of the list.
    */
   i = bp->first + bp->nused - 1;
   if (i >= bp->nelem)
      i -= bp->nelem;
   Arg0 = bp->lslots[i];
   bp->nused--;
   hp->size--;
   Return;
   }


/*
 * push(x,val) - push val onto beginning of list x.
 */
FncDcl(push,2)
   {
   register word i;
   register struct b_list *hp;
   register struct b_lelem *bp;
   extern struct b_lelem *alclstb();

   /*
    * x must be a list.
    */
   if (Arg1.dword != D_List)
      runerr(108, &Arg1);

   /*
    * A new list element block might be needed, so ensure space for it.
    */
   blkreq((word)sizeof(struct b_lelem)+MinListSlots*sizeof(struct descrip));

   /*
    * Point hp at the list header block and bp at the first
    * list element block.
    */
   hp = (struct b_list *) BlkLoc(Arg1);
   bp = (struct b_lelem *) BlkLoc(hp->listhead);
   /*
    * If the first list element block is full,
    * allocate a new list element block, make it the first list
    *  element block and make it the previous block of the
    *  former first list element block.
    */
   if (bp->nused >= bp->nelem) {
      bp = alclstb((word)MinListSlots, (word)0, (word)0);
      BlkLoc(hp->listhead)->lelem.listprev.dword = D_Lelem;
      BlkLoc(BlkLoc(hp->listhead)->lelem.listprev) = (union block *) bp;
      bp->listnext = hp->listhead;
      BlkLoc(hp->listhead) = (union block *) bp;
      }
   /*
    * Set i to position of new first element and assign val (Arg2) to
    *  that element.
    */
   i = bp->first - 1;
   if (i < 0)
      i = bp->nelem - 1;
   bp->lslots[i] = Arg2;
   /*
    * Adjust value of location of first element, block usage count,
    *  and current list size.
    */
   bp->first = i;
   bp->nused++;
   hp->size++;
   /*
    * Return the list.
    */
   Arg0 = Arg1;
   Return;
   }


/*
 * put(x,val) - put val onto end of list x.
 */

FncDcl(put,2)
   {
   register word i;
   register struct b_list *hp;
   register struct b_lelem *bp;
   extern struct b_lelem *alclstb();

   /*
    * Arg1 must be a list.
    */
   if (Arg1.dword != D_List)
      runerr(108, &Arg1);

   /*
    * A new list element block might be needed, so ensure space for it.
    */
   blkreq((word)sizeof(struct b_lelem)+MinListSlots*sizeof(struct descrip));

/* >put */
   /*
    * Point hp to the list header block and bp to the last
    *  list element block.
    */
   hp = (struct b_list *) BlkLoc(Arg1);
   bp = (struct b_lelem *) BlkLoc(hp->listtail);

   /*
    * If the last list element block is full, allocate a new
    *  list element block, make it the first list element block
    *  and it make it the next block of the former last list
    *  element block.
    */
   if (bp->nused >= bp->nelem) {
      bp = alclstb((word)MinListSlots, (word)0, (word)0);
      BlkLoc(hp->listtail)->lelem.listnext.dword = D_Lelem;
      BlkLoc(BlkLoc(hp->listtail)->lelem.listnext) = (union block *) bp;
      bp->listprev = hp->listtail;
      BlkLoc(hp->listtail) = (union block *) bp;
      }

   /*
    * Set i to position of new last element and assign Arg2 to
    *  that element.
    */
   i = bp->first + bp->nused;
   if (i >= bp->nelem)
      i -= bp->nelem;
   bp->lslots[i] = Arg2;

   /*
    * Adjust block usage count and current list size.
    */
   bp->nused++;
   hp->size++;

   /*
    * Return the list.
    */
   Arg0 = Arg1;
   Return;
   }
/* <put */


struct dpair {
   struct descrip dr;
   struct descrip dv;
   };

/*
 * sort(l) - sort list l.
 * sort(S) - sort set S.
 * sort(t,i) - sort table.
 */

FncDcl(sort,2)
   {
   register struct descrip *d1;
   register word size;
   register int i;
   word nelem;
   struct b_list *lp, *tp;
   union block *bp, *ep;
   extern struct b_list *alclist();
   extern struct b_lelem *alclstb();
   extern anycmp(), trefcmp(), tvalcmp(), trcmp3(), tvcmp4();

   if (Arg1.dword == D_List) {
      /*
       * Sort the list by copying it into a new list and then using
       *  qsort to sort the descriptors.  (That was easy!)
       */
      size = BlkLoc(Arg1)->list.size;
      cplist(&Arg1, &Arg0, (word)1, size + 1);
      qsort(BlkLoc(BlkLoc(Arg0)->list.listhead)->lelem.lslots, size,
            sizeof(struct descrip), anycmp);
      }
   else if (Arg1.dword == D_Set) {
      /*
       * Create a list the size of the set (or at least 
       *  MinListSlots), copy each element into the list, and
       *  then sort the list using qsort as in list sorting
       *  and return the sorted list.
       */
   nelem = size = BlkLoc(Arg1)->set.size;
   if(nelem < MinListSlots)
      nelem = MinListSlots;
   blkreq(sizeof(struct b_list) + sizeof(struct b_lelem) +
      nelem * sizeof(struct descrip));

   bp = BlkLoc(Arg1);
   lp = alclist(size);
   lp->listhead.dword = lp->listtail.dword = D_Lelem;
      BlkLoc(lp->listtail) = (union block *) alclstb(nelem, (word)0, size);
   BlkLoc(lp->listhead) = BlkLoc(lp->listtail);
   if (size > 0) {  /* only need to sort non-empty sets */
      d1 = BlkLoc(lp->listhead)->lelem.lslots;
      for(i = 0; i < SSlots; i++) {
      ep = BlkLoc(bp->set.sbucks[i]);
      while (ep != NULL) {
         *d1 = ep->selem.setmem;
         d1++;
         ep = BlkLoc(ep->selem.clink);
         }
      }
      qsort(BlkLoc(lp->listhead)->lelem.lslots,size,
         sizeof(struct descrip),anycmp);
   }
   Arg0.dword = D_List;
   BlkLoc(Arg0) = (union block *) lp;
   }

   else if (Arg1.dword == D_Table) {
      /*
       * Default i (the type of sort) to 1.
       */
      defshort(&Arg2, 1);
      switch (IntVal(Arg2)) {

      /*
       * Cases 1 and 2 are as in standard Version 5.
       */
         case 1:
         case 2:
                {
      /*
       * The list resulting from the sort will have as many elements as
       *  the table has, so get that value and also make a valid list
       *  block size out of it.
       */
      nelem = size = BlkLoc(Arg1)->table.size;
      if (nelem < MinListSlots)
         nelem = MinListSlots;
      /*
       * Ensure space for: the list header block and a list element
       *  block for the list which is to be returned,
       *  a list header block and a list element block for each of the two
       *  element lists the sorted list is to contain.  Note that the
       *  calculation might be better expressed as:
       *    list_header_size + list_block_size + nelem * descriptor_size +
       *     nelem * (list_header_size + list_block_size + 2*descriptor_size)
       */
      blkreq(sizeof(struct b_list) + sizeof(struct b_lelem) +
         nelem * (sizeof(struct b_list) + sizeof(struct b_lelem) +
            3 * sizeof(struct descrip)));
      /*
       * Point bp at the table header block of the table to be sorted
       *  and point lp at a newly allocated list
       *  that will hold the the result of sorting the table.
       */
      bp = BlkLoc(Arg1);
      lp = alclist(size);
      lp->listhead.dword = lp->listtail.dword = D_Lelem;
      BlkLoc(lp->listtail) = (union block *) alclstb(nelem, (word)0, size);
      BlkLoc(lp->listhead) = BlkLoc(lp->listtail);
      /*
       * If the table is empty, there is no need to sort anything.
       */
      if (size <= 0)
         break;
         /*
          * Point d1 at the start of the list elements in the new list
          *  element block in preparation for use as an index into the list.
          */
         d1 = BlkLoc(lp->listhead)->lelem.lslots;
         /*
          * Traverse the element chain for each table bucket.  For each
          *  element, allocate a two-element list and put the table
          *  entry value in the first element and the assigned value in
          *  the second element.  The two-element list is assigned to
          *  the descriptor that d1 points at.  When this is done, the
          *  list of two-element lists is complete, but unsorted.
          */
         for (i = 0; i < TSlots; i++) {
            ep = BlkLoc(bp->table.buckets[i]);
            while (ep != NULL) {
               d1->dword = D_List;
               tp = alclist((word)2);
               BlkLoc(*d1) = (union block *) tp;
               tp->listhead.dword = tp->listtail.dword = D_Lelem;
               BlkLoc(tp->listtail) = (union block *) alclstb((word)2, (word)0,
                  (word)2);
               BlkLoc(tp->listhead) = BlkLoc(tp->listtail);
               BlkLoc(tp->listhead)->lelem.lslots[0] = ep->telem.tref;
               BlkLoc(tp->listhead)->lelem.lslots[1] = ep->telem.tval;
               d1++;
               ep = BlkLoc(ep->telem.clink);
               }
            }
         /*
          * Sort the resulting two-element list using the sorting function
          *  determined by i.
          */
         if (IntVal(Arg2) == 1)
            qsort(BlkLoc(lp->listhead)->lelem.lslots, size,
                  sizeof(struct descrip), trefcmp);
         else
            qsort(BlkLoc(lp->listhead)->lelem.lslots, size,
                  sizeof(struct descrip), tvalcmp);
         break;		/* from cases 1 and 2 */
         }
      /*
       * Cases 3 and 4 were introduced in Version 5.10.
       */
         case 3 :
         case 4 :
                 {
      /*
       * The list resulting from the sort will have twice as many elements as
       *  the table has, so get that value and also make a valid list
       *  block size out of it.
       */
      nelem = size = BlkLoc(Arg1)->table.size * 2;
      if (nelem < MinListSlots)
         nelem = MinListSlots;
      /*
       * Ensure space for: the list header block and a list element
       *  block for the list which is to be returned, and two descriptors for
       *  each table element.
       */
      blkreq(sizeof(struct b_list) + Vsizeof(struct b_lelem) +
            (nelem * sizeof(struct descrip)));
      /*
       * Point bp at the table header block of the table to be sorted
       *  and point lp at a newly allocated list
       *  that will hold the the result of sorting the table.
       */
      bp = BlkLoc(Arg1);
      lp = alclist(size);
      lp->listhead.dword = lp->listtail.dword = D_Lelem;
      BlkLoc(lp->listtail) = (union block *) alclstb(nelem, (word)0, size);
      BlkLoc(lp->listhead) = BlkLoc(lp->listtail);
      /*
       * If the table is empty there's no need to sort anything.
       */
      if (size <= 0)
         break;

         /*
          * Point d1 at the start of the list elements in the new list
          *  element block in preparation for use as an index into the list.
          */
         d1 = BlkLoc(lp->listhead)->lelem.lslots;
         /*
          * Traverse the element chain for each table bucket.  For each
          *  table element copy the the entry descriptor and the value
          *  descriptor into adjacent descriptors in the lslots array
          *  in the list element block.
          *  When this is done we now need to sort this list.
          */
         for (i = 0; i < TSlots; i++) {
            ep = BlkLoc(bp->table.buckets[i]);
            while (ep != NULL) {
               *d1 = ep->telem.tref;
               d1++;
               *d1 = ep->telem.tval;
               d1++;
               ep = BlkLoc(ep->telem.clink);
               }
            }
         /*
          * Sort the resulting two-element list using the sorting function
          *  determined by i.
          */
         if (IntVal(Arg2) == 3)
            qsort(BlkLoc(lp->listhead)->lelem.lslots, size / 2,
                  (2 * sizeof(struct descrip)), trcmp3);
         else
            qsort(BlkLoc(lp->listhead)->lelem.lslots, size / 2,
                  (2 * sizeof(struct descrip)), tvcmp4);
            break; /* from case 3 or 4 */
            }

         default : runerr(205,&Arg2);
         } /* end of switch statement */
      /*
       * Make Arg0 point at the sorted list.
       */
      Arg0.dword = D_List;
      BlkLoc(Arg0) = (union block *) lp;
      }
   else /* Tried to sort something that wasn't a list or a table. */
      runerr(115, &Arg1);
   Return;
   }

/*
 * trefcmp(d1,d2) - compare two-element lists on first field.
 */

trefcmp(d1,d2)
struct descrip *d1, *d2;
   {
   extern anycmp();

#ifdef Debug
   if (d1->dword != D_List || d2->dword != D_List)
      syserr("trefcmp: internal consistency check fails.");
#endif Debug
   return (anycmp(&(BlkLoc(BlkLoc(*d1)->list.listhead)->lelem.lslots[0]),
                  &(BlkLoc(BlkLoc(*d2)->list.listhead)->lelem.lslots[0])));
   }

/*
 * tvalcmp(d1,d2) - compare two-element lists on second field.
 */

tvalcmp(d1,d2)
struct descrip *d1, *d2;
   {
   extern anycmp();

#ifdef Debug
   if (d1->dword != D_List || d2->dword != D_List)
      syserr("tvalcmp: internal consistency check fails.");
#endif Debug
   return (anycmp(&(BlkLoc(BlkLoc(*d1)->list.listhead)->lelem.lslots[1]),
                  &(BlkLoc(BlkLoc(*d2)->list.listhead)->lelem.lslots[1])));
   }

/*
 * The following two routines are used to compare descriptor pairs in the
 *  experimental table sort.
 *
 * trcmp3(dp1,dp2)
 */

trcmp3(dp1,dp2)
struct dpair *dp1,*dp2;
{
   extern anycmp();

   return (anycmp(&((*dp1).dr),&((*dp2).dr)));
}
/*
 * tvcmp4(dp1,dp2)
 */

tvcmp4(dp1,dp2)
struct dpair *dp1,*dp2;

   {
   extern anycmp();

   return (anycmp(&((*dp1).dv),&((*dp2).dv)));
   }
