#include "../h/rt.h"

#define PerilDelta 100
extern word *stackend;		/* End of main interpreter stack */
/*
 * invoke -- Perform setup for invocation.  
 */
invoke(nargs,cargp,n)
struct descrip **cargp;
int nargs, *n;
{
   register struct pf_marker *newpfp;
   register struct descrip *newargp;
   register word *newsp = sp;
   register word i;
   struct b_proc *proc;
   int nparam;
   long longint;
   char strbuf[MaxCvtLen];

   /*
    * Point newargp at Arg0 and dereference it.
    */
   newargp = (struct descrip *)(sp - 1) - nargs;
   DeRef(newargp[0]);
   
   /*
    * See what course the invocation is to take.
    */
   if (newargp->dword != D_Proc) {
      /*
       * Arg0 is not a procedure.
       */
      if (cvint(&newargp[0], &longint) == T_Integer) {
         /*
	  * Arg0 is an integer, select result.
	  */
         i = cvpos(longint, (word)nargs);
         if (i > nargs)
            return I_Goal_Fail;
         newargp[0] = newargp[i];
         sp = (word *)newargp + 1;
         return I_Continue;
         }
      else {
         /*
	  * See if Arg0 can be converted to a string that names a procedure
	  *  or operator.  If not, generate run-time error 106.
	  */
         if (cvstr(&newargp[0],strbuf) == NULL || strprc(&newargp[0],(word)nargs) == 0)
            runerr(106, newargp);
	 }
      }
   
   /*
    * newargp[0] is now a descriptor suitable for invocation.  Dereference
    *  the supplied arguments.
    */
   for (i = 1; i <= nargs; i++)
      DeRef(newargp[i]);
      
   /*
    * Adjust the argument list to conform to what the routine being invoked
    *  expects (proc->nparam).  If nparam is -1, the number of arguments
    *  is variable and no adjustment is required.  If too many arguments
    *  were supplied, adjusting the stack pointer is all that is necessary.
    *  If too few arguments were supplied, null descriptors are pushed
    *  for each missing argument.
    */
   proc = (struct b_proc *)BlkLoc(newargp[0]);
   nparam = proc->nparam;
   if (nparam != -1) {
      if (nargs > nparam)
         newsp -= (nargs - nparam) * 2;
      else if (nargs < nparam) {
         i = nparam - nargs;
         while (i--) {
            *++newsp = D_Null;
            *++newsp = 0;
            }
         }
      nargs = nparam;
      }

   if (proc->ndynam < 0) {
      /*
       * A built-in procedure is being invoked, so nothing else here
       *  needs to be done.
       */
      *n = nargs;
      *cargp = newargp;
      sp = newsp;
      if ((nparam == -1) || (proc->ndynam == -2))
         return I_Vararg;
      else
         return I_Builtin;
      }

   /*
    * Make a stab at catching interpreter stack overflow.  This does
    * nothing for invocation in a co-expression other than &main.
    */
   if (BlkLoc(current) == BlkLoc(k_main) && (sp + PerilDelta) > stackend)
      runerr(301, NULL);
   /*
    * Build the procedure frame.
    */
   newpfp = (struct pf_marker *)(newsp + 1);
   newpfp->pf_nargs = nargs;
   newpfp->pf_argp = argp;
   newpfp->pf_pfp = pfp;
   newpfp->pf_ilevel = ilevel;

   newpfp->pf_ipc = ipc;
   newpfp->pf_gfp = gfp;
   newpfp->pf_efp = efp;

   argp = newargp;
   pfp = newpfp;
   newsp += Vwsizeof(*pfp);
   
   /*
    * Point ipc at the icode entry point of the procedure being invoked.
    */
   ipc = (word *)proc->entryp.icode;
   efp = 0;
   gfp = 0;

   newpfp->pf_line = line;

   /*
    * If tracing is on, use ctrace to generate a message.
    */   
   if (k_trace != 0)
      ctrace(proc, nargs, &newargp[1]);

   /*
    * Push a null descriptor on the stack for each dynamic local.
    */
   for (i = proc->ndynam; i > 0; i--) {
      *++newsp = D_Null;
      *++newsp = 0;
      }

   sp = newsp;
   k_level++;
   return I_Continue;
}
