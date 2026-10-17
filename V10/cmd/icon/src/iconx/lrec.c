/*
 * File: lrec.c
 *  Contents: field, mkrec
 */

#include "../h/rt.h"

/*
 * x.y - access field y of record x.
 */

LibDcl(field,2,".")
   {
   register word fnum;
   register struct b_record *rp;
   register struct descrip *dp;
   extern word *ftab, *records;

   DeRef(Arg1);
   /*
    * x must be a record and y must be a field number.
    */
   if (Qual(Arg1) || Arg1.dword != D_Record)
      runerr(107, &Arg1);

   /*
    * Map the field number into a field number for the record x.
    */
   rp = (struct b_record *) BlkLoc(Arg1);
   fnum = ftab[IntVal(Arg2) * *records + BlkLoc(rp->recdesc)->proc.recnum - 1];
   /*
    * If fnum < 0, x doesn't contain the specified field.
    */
   if (fnum < 0)
      runerr(207, &Arg1);

   /*
    * Return a pointer to the descriptor for the appropriate field.
    */
   dp = &rp->fields[fnum];
   Arg0.dword = D_Var + ((word *)dp - (word *)rp);
   BlkLoc(Arg0) = (union block *) dp;
   Return;
   }


/*
 * mkrec - create a record.
 */

LibDcl(mkrec,-1,"mkrec")
   {
   register int i;
   register struct b_proc *bp;
   register struct b_record *rp;
   extern struct b_record *alcrecd();

   /*
    * Be sure that call is from a procedure.
    */

   /*
    * Ensure space for the record to be created.
    */
   blkreq((uword)sizeof(struct b_record) +
         BlkLoc(Arg(0))->proc.nfields*sizeof(struct descrip));

   /*
    * Get a pointer to the record constructor procedure and allocate
    *  a record with the appropriate number of fields.
    */
   bp = (struct b_proc *) BlkLoc(Arg(0));
   rp = alcrecd((int)bp->nfields, (struct descrip *)bp);

   /*
    * Set all fields in the new record to null value.
    */
   for (i = bp->nfields; i > nargs; i--)
      rp->fields[i-1] = nulldesc;

   /*
    * Assign each argument value to a record element and dereference it.
    */
   for ( ; i > 0; i--) {
      rp->fields[i-1] = Arg(i);
      DeRef(rp->fields[i-1]);
      }

   ArgType(0) = D_Record;
   Arg(0).vword.bptr = (union block *)rp;
   Return;
   }
