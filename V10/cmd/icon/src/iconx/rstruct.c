/*
 * File: rstruct.c
 *  Contents: addmem, cplist, locate, memb
 */

#include "../h/rt.h"
/*
 * addmem - add a new set element block in the correct spot in
 *  the bucket chain.
 */

addmem(ps,pe,pl)
struct descrip *pl;
struct b_set *ps;
struct b_selem *pe;
   {
   ps->size++;
   if (!ChkNull(*pl) ) {
      BlkLoc(pe->clink) = BlkLoc(*pl);
      pe->clink.dword = D_Selem;
      }
   BlkLoc(*pl) = (union block *) pe;
   pl->dword = D_Selem;
   }


/*
 * cplist(d1,d2,i,j) - copy sublist d1[i:j] into d2.
 */

cplist(d1, d2, i, j)
struct descrip *d1, *d2;
word i, j;
   {
   register struct descrip *dp;
   word size, nelem;
   struct b_list *lp1, *lp2;
   struct b_lelem *bp1, *bp2;

   /*
    * Calculate the size of the sublist and fail if it's less than 0.
    *  Also round nelem up to the minimum list block size.
    */
   size = nelem = j - i;
   if (nelem < MinListSlots)
      nelem = MinListSlots;

   /*
    * Get pointers to the list and list elements for the source list
    *  (bp1, lp1) and the sublist (bp2, lp2).
    */
   blkreq(sizeof(struct b_list) + sizeof(struct b_lelem) +
         nelem * sizeof(struct descrip));
   lp1 = (struct b_list *) BlkLoc(*d1);
   bp1 = (struct b_lelem *) BlkLoc(lp1->listhead);
   lp2 = (struct b_list *) alclist(size);
   bp2 = (struct b_lelem *) alclstb(nelem, (word)0, size);
   lp2->listhead.dword = lp2->listtail.dword = D_Lelem;
   BlkLoc(lp2->listhead) = BlkLoc(lp2->listtail) = (union block *) bp2;
   dp = bp2->lslots;

   /*
    * Locate the block containing element i in the source list.
    */
   if (size > 0) {
      while (i > bp1->nused) {
         i -= bp1->nused;
         bp1 = (struct b_lelem *) BlkLoc(bp1->listnext);
         }
      }

   /*
    * Copy elements from the source list into the sublist, moving to
    *  the next list block in the source list when all elements in a
    *  block have been copied.
    */
   while (size > 0) {
      j = bp1->first + i - 1;
      if (j >= bp1->nelem)
         j -= bp1->nelem;
      *dp++ = bp1->lslots[j];
      if (++i > bp1->nused) {
         i = 1;
         bp1 = (struct b_lelem *) BlkLoc(bp1->listnext);
         }
      size--;
      }

   /*
    * Fix type and location fields for the new list.
    */
   d2->dword = D_List;
   BlkLoc(*d2) = (union block *) lp2;
   }


/*
 * locate - returns 1 if obj is in the hash chain which
 *  starts at ep in some set, returns 0 if not there
 *  (used only in diff.c and inter.c).
 */

locate(ep, obj)
struct b_selem *ep, *obj;
   {
   while (ep != NULL) {
      if (ep->hashnum > obj->hashnum)
          return 0;
      else if ((ep->hashnum == obj->hashnum) &&
                 (equiv(&ep->setmem, &obj->setmem)))
          return 1;
      ep = (struct b_selem *) BlkLoc(ep->clink);
      }
   return 0;
   }


/*
 * memb - sets res flag to 1 if x is a member of set S,
 *  sets res flag to 0 if not.
 *  Returns pointer to the descriptor which points to
 *  the element (or which would point to it if it were
 *  there).
 */

struct descrip *memb(ps,x,hn,res)
int *res;				/* pointer to integer result flag */
struct b_set *ps;
struct descrip  *x;
word hn;
   {
    register struct descrip *lp;
    register struct b_selem  *pe;
    extern int equiv();

   lp = &(ps->sbucks[SlotNum(hn,SSlots)]);
   /*
    * Look for x in the hash chain.
    */
   *res = 0;
   while (BlkLoc(*lp) != NULL) {
       pe = (struct b_selem *) BlkLoc(*lp);
       if ( pe->hashnum > hn)		/* too far - it isn't there */
         return lp;
       else if (( pe->hashnum == hn ) && ( equiv(&pe->setmem, x )))  {
         *res = 1;
         return lp;
         }
   /*
    * We haven't reached the right hashnumber yet or
    *  the element isn't the right one so keep looking.
    */
      lp = &(pe->clink);
   }
   /*
    *  At end of chain - not there.
    */
   return lp;
   }
