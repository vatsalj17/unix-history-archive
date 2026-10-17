/*
 * File: fconv.c
 *  Contents: abs, cset, integer, list, numeric, proc, real, set, string, table
 */

#include "../h/rt.h"

/*
 * abs(x) - absolute value of x.
 */
FncDcl(abs,1)
   {
   union numeric result;

   switch (cvnum(&Arg1, &result)) {
      /*
       * If x is convertible to a numeric, turn Arg0 into
       *  a descriptor for the appropriate type and value.  If the
       *  conversion fails, produce an error.  This code assumes that
       *  x = -x is always valid, but this assumption does not always
       *  hold.
       */
      case T_Integer:
      case T_Longint:
         if (result.integer < 0L)
            result.integer = -result.integer;
         Mkint(result.integer, &Arg0);
         break;

      case T_Real:
         if (result.real < 0.0)
            result.real = -result.real;
         mkreal(result.real, &Arg0);
         break;

      default:
         runerr(102, &Arg1);
      }
   Return;
   }


/*
 * cset(x) - convert x to cset.
 */

FncDcl(cset,1)
   {
   register int i, j;
   register struct b_cset *bp;
   int *cs, csbuf[CsetSize];
   extern struct b_cset *alccset();

   blkreq((word)sizeof(struct b_cset));

   if (Arg1.dword == D_Cset)
      /*
       * x is already a cset, just return it.
       */
      Arg0 = Arg1;
   else if (cvcset(&Arg1, &cs, csbuf) != NULL) {
      /*
       * x was convertible to cset and the result resides in csbuf.  Allocate
       *  a cset, make Arg0 a descriptor for it and copy the bits from csbuf
       *  into it.
       */
      Arg0.dword = D_Cset;
      bp = alccset(0);
      BlkLoc(Arg0) =  (union block *) bp;
      for (i = 0; i < CsetSize; i++)
         bp->bits[i] = cs[i];
      j = 0;
      for (i = 0; i < CsetSize*CIntSize; i++) {
         if (Testb(i,cs))
            j++;
         }
      bp->size = j;
      }
   else			/* Not a cset nor convertible to one. */
      Fail;
   Return;
   }


/*
 * integer(x) - convert x to integer.
 */

FncDcl(integer,1)
   {
   long l;

   switch (cvint(&Arg1, &l)) {

      case T_Integer:
      case T_Longint:
         Mkint(l, &Arg0);
         break;

      default:
         Fail;
      }
   Return;
   }


/*
 * list(n,x) - create a list of size n, with initial value x.
 */

/* >list */
FncDcl(list,2)
   {
   register word i, size;
   word nelem;
   register struct b_lelem *bp;
   register struct b_list *hp;
   extern struct b_list *alclist();
   extern struct b_lelem *alclstb();

   defshort(&Arg1, 0);			/* Size defaults to 0 */

   nelem = size = IntVal(Arg1);


   /*
    * Ensure that the size is positive and that the list element block 
    *  has at least MinListSlots element slots.
    */
   if (size < 0)
      runerr(205, &Arg1);
   if (nelem < MinListSlots)
      nelem = MinListSlots;

   /*
    * Ensure space for a list header block, and a list element block
    * with nelem element slots.
    */
   blkreq(sizeof(struct b_list) + sizeof(struct b_lelem) +
         nelem * sizeof(struct descrip));

   /*
    * Allocate the list header block and a list element block.
    *  Note that nelem is the number of elements in the list element
    *  block while size is the number of elements in the
    *  list.
    */
   hp = alclist(size);
   bp = alclstb(nelem, (word)0, size);
   hp->listhead.dword = hp->listtail.dword = D_Lelem;
   BlkLoc(hp->listhead) = BlkLoc(hp->listtail) = (union block *) bp;

   /*
    * Initialize each list element.
    */
   for (i = 0; i < size; i++)
      bp->lslots[i] = Arg2;

   /*
    * Return the new list.
    */
   Arg0.dword = D_List;
   BlkLoc(Arg0) = (union block *) hp;
   Return;
   }
/* <list */


/*
 * numeric(x) - convert x to numeric type.
 */
FncDcl(numeric,1)
   {
   union numeric n1;

   switch (cvnum(&Arg1, &n1)) {

      case T_Integer:
      case T_Longint:
         Mkint(n1.integer, &Arg0);
         break;

      case T_Real:
         mkreal(n1.real, &Arg0);
         break;

      default:
         Fail;
      }
   Return;
   }


/*
 * proc(x,args) - convert x to a procedure if possible; use args to
 *  resolve ambiguous string names.
 */
FncDcl(proc,2)
   {
   char sbuf[MaxCvtLen];
   
   /*
    * If x is already a proc, just return it in Arg0.
    */
   Arg0 = Arg1;
   if (Arg0.dword == D_Proc) {
      Return;
      }
   if (cvstr(&Arg0, sbuf) == NULL)
      Fail;
   /*
    * args defaults to 1.
    */
   defshort(&Arg2, 1);
   /*
    * Attempt to convert Arg0 to a procedure descriptor using args to
    *  discriminate between procedures with the same names.  Fail if
    *  the conversion isn't successful.
    */
   if (strprc(&Arg0,IntVal(Arg2))) {
      Return;
      }
   else
      Fail;
   }


/*
 * real(x) - convert x to real.
 */

FncDcl(real,1)
   {
   double r;

   /*
    * If x is already a real, just return it.  Otherwise convert it and
    *  return it, failing if the conversion is unsuccessful.
    */
   if (Arg1.dword == D_Real)
      Arg0 = Arg1;
   else if (cvreal(&Arg1, &r) == T_Real)
      mkreal(r, &Arg0);
   else
      Fail;
   Return;
   }


/*
 * set(list) - create a set with members in list.
 *  The members are linked into hash chains which are
 *  arranged in increasing order by hash number.
 */
FncDcl(set,1)
   {
   register word hn;
   register struct descrip *pd;
   register struct b_set *ps;
   union block *pb;
   struct b_selem *ne;
   struct descrip *pe;
   int res;
   word i, j;
   extern struct descrip *memb();
   extern struct b_set *alcset();
   extern struct b_selem *alcselem();

   if (Arg1.dword != D_List)
      runerr(108,&Arg1);

   blkreq(sizeof(struct b_set) + (BlkLoc(Arg1)->list.size *
      sizeof(struct b_selem)));

   pb = BlkLoc(Arg1);
   Arg0.dword = D_Set;
   ps = alcset();
   BlkLoc(Arg0) = (union block *) ps;
   /*
    * Chain through each list block and for
    *  each element contained in the block
    *  insert the element into the set if not there.
    */
   for (Arg1 = pb->list.listhead; Arg1.dword == D_Lelem;
      Arg1 = BlkLoc(Arg1)->lelem.listnext) {
         pb = BlkLoc(Arg1);
         for (i = 0; i < pb->lelem.nused; i++) {
            j = pb->lelem.first + i;
            if (j >= pb->lelem.nelem)
               j -= pb->lelem.nelem;
            pd = &pb->lelem.lslots[j];
            pe = memb(ps, pd, hn = hash(pd), &res);
            if (res == 0) {
               ne = alcselem(pd,hn);
                addmem(ps,ne,pe);
                }
            }
      }
   Return;
   }


/*
 * string(x) - convert x to string.
 */

/* >string */
FncDcl(string,1)
   {
   char sbuf[MaxCvtLen];
   extern char *alcstr();

   Arg0 = Arg1;
   switch (cvstr(&Arg0, sbuf)) {

      /*
       * If Arg1 is not a string, allocate it and return it; if it is a
       *  string, just return it; fail otherwise.
       */
      case Cvt:
         strreq(StrLen(Arg0));		/* allocate converted string */
         StrLoc(Arg0) = alcstr(StrLoc(Arg0), StrLen(Arg0));

      case NoCvt:
         Return;

      default:
         Fail;
      }
   }
/* <string */

/*
 * table(x) - create a table with default value x.
 */
FncDcl(table,1)
   {
   extern struct b_table *alctable();

   blkreq((word)sizeof(struct b_table));
   Arg0.dword = D_Table;
   BlkLoc(Arg0) = (union block *) alctable(&Arg1);
   Return;
   }
