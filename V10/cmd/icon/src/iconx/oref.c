/*
 * File: oref.c
 *  Contents: bang, sect, subsc
 */

#include "../h/rt.h"

/*
 * !x - generate successive values from object x.
 */

OpDclV(bang,1,"!")
   {
   register word i, j, slen;
   register union block *bp, *ep;
   register struct descrip *dp;
   int typ1;
   char sbuf[MaxCvtLen];
   FILE *fd;
   extern char *alcstr();

   Arg2 = Arg1;

   DeRef(Arg1);
   if ((typ1 = cvstr(&Arg1, sbuf)) != NULL) {
      /*
       * A string is being banged.
       */
      i = 1;
      while (i <= StrLen(Arg1)) {
         /*
          * Loop through the string using i as an index.
          */
         if (typ1 == Cvt) {
            /*
             * x was converted to a string, thus, the resulting string
             *	can't be modified and a trapped variable isn't needed.
             *	Make a one-character string out of the next character
             *	in x and suspend it.
             */
            strreq((word)1);
            StrLen(Arg0) = 1;
            StrLoc(Arg0) = alcstr(StrLoc(Arg1)+i-1, (word)1);
            Suspend;
            }
         else {
            /*
             * x a string and thus a trapped variable must be made
             *	for the one character string being suspended.
             */
            blkreq((word)sizeof(struct b_tvsubs));
            mksubs(&Arg2, &Arg1, i, (word)1, &Arg0);
            Suspend;
            Arg1 = Arg2;
            DeRef(Arg1);
            if (!Qual(Arg1))
               runerr(103, &Arg1);
            }
         i++;
         }
      }
   else {
      /*
       * x isn't a string.
       */
      switch (Type(Arg1)) {
         case T_List:
            /*
             * x is a list.  Chain through each list element block and for
             *	each one, suspend with a variable pointing to each
             *	element contained in the block.
             */
            bp = BlkLoc(Arg1);
            for (Arg1 = bp->list.listhead; Arg1.dword == D_Lelem;
               Arg1 = BlkLoc(Arg1)->lelem.listnext) {
               bp = BlkLoc(Arg1);
               for (i = 0; i < bp->lelem.nused; i++) {
		  j = bp->lelem.first + i;
		  if (j >= bp->lelem.nelem)
		     j -= bp->lelem.nelem;
		  dp = &bp->lelem.lslots[j];
		  Arg0.dword = D_Var + ((word *)dp - (word *)bp);
		  VarLoc(Arg0) = dp;
		  Suspend;
		  bp = BlkLoc(Arg1);   /* bp is untended, must reset */
		  }
               }
            break;


         case T_File:
            /*
             * x is a file.  Read the next line into the string space
             *	and suspend the newly allocated string.
             */
            fd = BlkLoc(Arg1)->file.fd;
            if ((BlkLoc(Arg1)->file.status & Fs_Read) == 0)
               runerr(212, &Arg1);
            while ((slen = getstr(sbuf,MaxCvtLen,fd)) >= 0) {
               strreq(slen);
               StrLen(Arg0) = slen;
               StrLoc(Arg0) = alcstr(sbuf,slen);
               Suspend;
               }
            break;

         case T_Table:
            /*
             * x is a table.  Chain down the element list in each bucket
             *	and suspend a variable pointing to each element in turn.
             */
            for (i = 0; i < TSlots; i++) {
               bp = BlkLoc(Arg1);
               for (Arg2 = bp->table.buckets[i]; Arg2.dword == D_Telem;
		    Arg2 = BlkLoc(Arg2)->telem.clink) {
		  ep = BlkLoc(Arg2);
		  dp = &ep->telem.tval;
		  Arg0.dword = D_Var + ((word *)dp - (word *)ep);
		  VarLoc(Arg0) = dp;
		  Suspend;
		  bp = BlkLoc(Arg1);   /* bp is untended, must reset */
		  }
               }
            break;

         case T_Set:
           /*
            *  This is similar to the method for tables except that a
            *  value is returned instead of a variable.
            */
               for(i = 0; i < SSlots; i++) {
		  bp = BlkLoc(Arg1);
		  for(Arg2 = bp->set.sbucks[i]; Arg2.dword == D_Selem;
		     Arg2 = BlkLoc(Arg2)->selem.clink) {
		     Arg0 = BlkLoc(Arg2)->selem.setmem;
		     Suspend;
		  bp = BlkLoc(Arg1);	/* bp untended, must be reset */
		    }
		}
		break;

         case T_Record:
            /*
             * x is a record.  Loop through the fields and suspend
             *	a variable pointing to each one.
             */
            bp = BlkLoc(Arg1);
            j = BlkLoc(bp->record.recdesc)->proc.nfields;
            for (i = 0; i < j; i++) {
               dp = &bp->record.fields[i];
               Arg0.dword = D_Var + ((word *)dp - (word *)bp);
               VarLoc(Arg0) = dp;
               Suspend;
               bp = BlkLoc(Arg1);   /* bp is untended, must reset */
               }
            break;

         default: /* This object can not be compromised. */
            runerr(116, &Arg1);
         }
      }

   /*
    * Eventually fail.
    */
   Fail;
   }


/*
 * x[i:j] - form a substring or list section of x.
 */

OpDclV(sect,3,":")
   {
   register word i, j, t;
   int typ1;
   char sbuf[MaxCvtLen];
   long l1, l2;
   extern char *alcstr();

   blkreq((word)sizeof(struct b_tvsubs));	 /* request space */

   if (cvint(&Arg2, &l1) == NULL)
      runerr(101, &Arg2);
   if (cvint(&Arg3, &l2) == NULL)
      runerr(101, &Arg3);

   Arg4 = Arg1;
   DeRef(Arg1);

   if (!Qual(Arg1) && Arg1.dword == D_List) {
      i = cvpos(l1, BlkLoc(Arg1)->list.size);
      if (i == 0)
         Fail;
      j = cvpos(l2, BlkLoc(Arg1)->list.size);
      if (j == 0)
         Fail;
      if (i > j) {
         t = i;
         i = j;
         j = t;
         }
      cplist(&Arg1, &Arg0, i, j);
      Return;
      }

   if ((typ1 = cvstr(&Arg1, sbuf)) == NULL)
      runerr(110, &Arg1);

   i = cvpos(l1, StrLen(Arg1));
   if (i == 0)
      Fail;
   j = cvpos(l2, StrLen(Arg1));
   if (j == 0)
      Fail;
   if (i > j) { 			/* convert section to substring */
      t = i;
      i = j;
      j = t - j;
      }
   else
      j = j - i;

   if (typ1 == Cvt) {			/* if string was created, */
      strreq(j);				/*   just return a string */
      StrLen(Arg0) = j;
      StrLoc(Arg0) = alcstr(StrLoc(Arg1)+i-1, j);
      }
   else 				/* else make a substring tv */
      mksubs(&Arg4, &Arg1, i, j, &Arg0);
   Return;
   }


/*
 * x[y] - access yth character or element of x.
 */

OpDclV(subsc,2,"[]")
   {
   register word i, j;
   register union block *bp;
   int typ1;
   long l1;
   struct descrip *dp;
   char sbuf[MaxCvtLen];
   extern char *alcstr();
   extern struct b_tvtbl *alctvtbl();

   /*
    * Make a copy of x.
    */
   Arg3 = Arg1;

   DeRef(Arg1);
   if ((typ1 = cvstr(&Arg1, sbuf)) != NULL) {
      /*
       * x is a string, make sure that y is an integer.
       */
      if (cvint(&Arg2, &l1) == NULL)
         runerr(101, &Arg2);
      /*
       * Convert y to a position in x and fail if the position is out
       *  of bounds.
       */
      i = cvpos(l1, StrLen(Arg1));
      if (i == 0 || i > StrLen(Arg1))
         Fail;
      if (typ1 == Cvt) {
         /*
          * x was converted to a string, so it can't be assigned back into.
          *  Just return a string containing the selected character.
          */
         strreq((word)1);
         StrLen(Arg0) = 1;
         StrLoc(Arg0) = alcstr(StrLoc(Arg1)+i-1, (word)1);
         }
      else {
         /*
          * x is a string, make a substring trapped variable for the one
          *  character substring selected and return it.
          */
         blkreq((word)sizeof(struct b_tvsubs));
         mksubs(&Arg3, &Arg1, i, (word)1, &Arg0);
         }
      Return;
      }

   /*
    * x isn't a string or convertible to one, see if it's an aggregate.
    */
   DeRef(Arg1);
/* >listref */
   switch (Type(Arg1)) {
      case T_List:
         /*
          * Make sure that Arg2 is an integer and that the
          *  subscript is in range.
          */
         if (cvint(&Arg2, &l1) == NULL)
            runerr(101, &Arg2);
         i = cvpos(l1, BlkLoc(Arg1)->list.size);
         if (i == 0 || i > BlkLoc(Arg1)->list.size)
            Fail;

         /*
          * Locate the list block containing the desired element.
          */
         bp = BlkLoc(BlkLoc(Arg1)->list.listhead);
         j = 1;
         while (i >= j + bp->lelem.nused) {
            j += bp->lelem.nused;
            if ((bp->lelem.listnext).dword != D_Lelem)
               syserr("list reference out of bounds in subsc");
            bp = BlkLoc(bp->lelem.listnext);
            }

         /*
          * Locate the desired element in the block that contains it
          *  and return a pointer to it.
          */
         i += bp->lelem.first - j;
         if (i >= bp->lelem.nelem)
            i -= bp->lelem.nelem;
         dp = &bp->lelem.lslots[i];
         Arg0.dword = D_Var + ((int *)dp - (int *)bp);
         VarLoc(Arg0) = dp;
         Return;
/* <listref */

      case T_Table:
         /*
          * x is a table.  Locate the appropriate bucket
          *  based on the hash value.
          */
         blkreq((word)sizeof(struct b_tvtbl));
         i = hash(&Arg2);		/* get hash number of subscript  */
         bp = BlkLoc(BlkLoc(Arg1)->table.buckets[SlotNum(i,TSlots)]);
         /*
          * Work down the chain of elements for the bucket and if an
          *  element with the desired subscript value is found, return
          *  a pointer to it.
          * Elements are ordered in the chain by hash number value
          * from smallest to largest.
          */
         while (bp != NULL) {
           if (bp->telem.hashnum > i)		/* past it - not there */
               break;
            if ((bp->telem.hashnum == i)  &&  (equiv(&bp->telem.tref, &Arg2))) {
               dp = &bp->telem.tval;
               Arg0.dword = D_Var + ((word *)dp - (word *)bp);
               VarLoc(Arg0) = dp;
               Return;
               }
            /* We haven't reached the right hash number yet or
             *	the element is not the right one.
             */
            bp = BlkLoc(bp->telem.clink);
            }
           /*
            * x[y] is not in the table, make a table element trapped variable
            *  and return it as the result.
            */
         Arg0.dword = D_Tvtbl;
         BlkLoc(Arg0) = (union block *) alctvtbl(&Arg1, &Arg2, i);
         Return;

      case T_Record:
         /*
          * x is a record.  Convert y to an integer and be sure that it
          *  it is in range as a field number.
          */
         if (cvint(&Arg2, &l1) == NULL)
            runerr(101, &Arg2);
         bp = BlkLoc(Arg1);
         i = cvpos(l1, (word)(BlkLoc(bp->record.recdesc)->proc.nfields));
         if (i == 0 || i > BlkLoc(bp->record.recdesc)->proc.nfields)
            Fail;
         /*
          * Locate the appropriate field and return a pointer to it.
          */
         dp = &bp->record.fields[i-1];
           Arg0.dword = D_Var + ((word *)dp - (word *)bp);
         VarLoc(Arg0) = dp;
         Return;

      default:
         /*
          * x is of a type that can't be subscripted.
          */
         runerr(114, &Arg1);
      }
   Return;
   }
