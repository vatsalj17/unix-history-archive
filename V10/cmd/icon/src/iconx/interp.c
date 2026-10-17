/*
 * The intepreter proper.
 */

/*
 * Instrumentation selection is based on Instr_level, which is given
 * as a product of prime numbers.
 */
#ifndef Instr
#define Instr 997
#endif Instr
#ifndef Instr_level
#define Instr_level	2
#endif Instr_level

#include "../h/rt.h"
#include "gc.h"
#include "../h/opdef.h"

/*
 * Istate variables.
 */
struct pf_marker *pfp;		/* Procedure frame pointer */
struct ef_marker *efp;		/* Expression frame pointer */
struct gf_marker *gfp;		/* Generator frame pointer */
word *ipc;			/* Interpreter program counter */
struct descrip *argp;		/* Pointer to argument zero */
word *sp;			/* Stack pointer */
extern int mstksize;		/* Size of main stack */
int ilevel;			/* Depth of recursion in interp() */

#if (Instr % Instr_level) == 0
int maxilevel;			/* Maximum ilevel */
word *maxsp;			/* Maximum interpreter sp */
#endif Instr

extern word *stack;		/* Interpreter stack */
word *stackend;			/* End of main interpreter stack */

#ifdef MSDOS
#ifdef LPTR
static union {
       char *stkadr;
       word stkint;
   } stkword;
#define PushAVal(v)  {sp++; \
		      stkword.stkadr = (char *)(v); \
		      *sp = stkword.stkint;}
#else LPTR
#define PushAVal PushVal
#endif LPTR
#else MSDOS
#define PushAVal PushVal
#endif MSDOS

/*
 * Initial icode sequence. This is used to invoke the main procedure with one
 *  operand.  If main returns, the Op_Quit is executed.
 */
word istart[3] = {Op_Invoke, 1, Op_Quit};
word mterm = Op_Quit;

/*
 * The tended descriptors.
 */
struct descrip tended[6];

/*
 * Descriptor to hold result for eret across potential interp unwinding.
 */
struct descrip eret_tmp;
/*
 * Last co-expression action.
 */
int coexp_act;
struct descrip *xargp;

main(argc, argv)
int argc; char **argv;
{
   int i;
   extern int tallyopt;

#if (Instr % Instr_level) == 0
   maxilevel = 0;
   maxsp = 0;
#endif Instr

#ifdef VMS
   redirect(&argc, argv, 0);
#endif VMS

   /*
    * Set tallying flag if -T option given
    */
   if (!strcmp(argv[1],"-T")) {
      tallyopt = 1;
      argc--;
      argv++;
      }


   /*
    * Call init with the name of the file to interpret.
    */
   init(argv[1]);

   /*
    *  Point sp at word after b_coexpr block for &main, point ipc at initial
    *   icode segment, and clear the gfp.
    */
   stackend = stack + mstksize/WordSize;
   sp = stack + Wsizeof(struct b_coexpr);
   ipc = istart;
   gfp = 0;

   /*
    * Set up expression frame marker to contain execution of the
    *  main procedure.  If failure occurs in this context, control
    *  is transferred to mterm, the address of an Op_Quit.
    */
   efp = (struct ef_marker *)(sp);
   efp->ef_failure = &mterm;
   efp->ef_gfp = 0;
   efp->ef_efp = 0;
   efp->ef_ilevel = 1;
   sp += Wsizeof(*efp) - 1;

   /*
    * The first global variable holds the value of "main".  If it
    *  is not of type procedure, this is noted as run-time error 117.
    *  Otherwise, this value is pushed on the stack.
    */
   if (globals[0].dword != D_Proc)
      runerr(117, NULL);
   PushDesc(globals[0]);

   /*
    * Main is to be invoked with one argument, a list of the command
    *  line arguments.  The command line arguments are pushed on the
    *  stack as a series of descriptors and llist is called to create
    *  the list.  The null descriptor first pushed serves as Arg0 for
    *  llist and receives the result of the computation.
    */
   PushNull;
   argp = (struct descrip *)(sp - 1);
   for (i = 2; i < argc; i++) {
      PushVal(strlen(argv[i]));
      PushAVal(argv[i]);
      }
   llist(argc - 2, argp);
   sp = (word *)argp + 1;
   argp = 0;

   /*
    * Start things rolling by calling interp.  This call to interp
    *  returns only if an Op_Quit is executed.  If this happens,
    *  c_exit() is called to wrap things up.
    */
   interp(0,NULL);
   c_exit(NormalExit);
}

/*
 * Macros for use inside the main loop of the interpreter.
 */

/*
 * Setup_Op sets things up for a call to the C function for an operator.
 */
#define Setup_Op(nargs)  \
   rargp = (struct descrip *) (rsp - 1) - nargs; \
   ExInterp;

/*
 * Call_Op(n) calls an unconditional operator. The C routine associated
 *  with the current opcode is called. After the routine performs the operation
 *  and returns, the stack pointer is reset to point to the result.
 */
#define Call_Op(n) (*(optab[op]))(rargp); \
         rsp = (word *) rargp + 1;

/*
 * Call_Cond(n) calls a conditional operator. The C routine associated
 * with the current opcode is called.  The routine returns a signal of
 * success or failure.  If the operation succeeds, the stack
 * pointer is reset to point to the result. If the routine fails, control
 * transfers to efail.
 */
#define Call_Cond(n) if ((*(optab[op]))(rargp) == A_Failure) \
         goto efail; \
         else \
         rsp = (word *) rargp + 1;

/*
 * Call_Gen(n) - Call a generator. A C routine associated with the
 *  current opcode is called. When it when it terminates, control is 
 *  passed to C_rtn_term to deal with the termination condition appropriately.
 */
#define Call_Gen(n)   signal = (*(optab[op]))(rargp); \
         goto C_rtn_term;

/*
 * GetWord fetches the next icode word.  PutWord(x) stores x at the current
 * icode word.
 */
#define GetWord (*ipc++)
#define PutWord(x) ipc[-1] = (x)
/*
 * DerefArg(n) dereferences the n'th argument.
 */
#define DerefArg(n)   DeRef(rargp[n])

/*
 * For the sake of efficiency, the stack pointer is kept in a register
 *  variable, rsp, in the interpreter loop.  Since this variable is
 *  only accessible inside the loop, and the global variable sp is used
 *  for the stack pointer elsewhere, rsp must be stored into sp when
 *  the context of the loop is left and conversely, rsp must be loaded
 *  from sp when the loop is reentered.  The macros ExInterp and EntInterp,
 *  respectively, handle these operations.  Currently, this register/global
 *  scheme is only used for the stack pointer, but it can be easily extended
 *  to other variables.
 */

#define ExInterp   sp = rsp;
#define EntInterp   rsp = sp;

/*
 * Inside the interpreter loop, PushDesc, PushNull, and
 *  PushVal use rsp instead of sp for efficiency.
 */

#undef PushDesc
#undef PushNull
#undef PushVal
#undef PushAVal
#define PushDesc(d)   {rsp++;*rsp=((d).dword);rsp++;*rsp=((d).vword.integr);}
#define PushNull   {rsp++; *rsp = D_Null; rsp++; *rsp = 0;}
#define PushVal(v)   {rsp++; *rsp = (word)(v);}
#ifdef MSDOS
#ifdef LPTR
#define PushAVal(v) {rsp++; \
                       stkword.stkadr = (char *)(v); \
                       *rsp = stkword.stkint; \
                       }
#else LPTR
#define PushAVal PushVal
#endif LPTR
#else MSDOS
#define PushAVal PushVal
#endif MSDOS

/*
 * The main loop of the interpreter.
 */

interp(fsig,cargp)
int fsig;
struct descrip *cargp;
   {
   register word opnd, op;
   register word *rsp;
   register struct descrip *rargp;
   register struct ef_marker *newefp;
   register struct gf_marker *newgfp;
   register word *wd;
   register word *firstwd, *lastwd;
   word *oldsp;
   int type, signal;
   extern int (*optab[])();
   extern char *ident;
   extern word tallybin[];

   ilevel++;
#if (Instr % Instr_level) == 0
   if (ilevel > maxilevel)
      maxilevel = ilevel;
#endif Instr
   EntInterp;
   if (fsig == G_Csusp) {
      oldsp = rsp;
   
      /*
       * Create the generator frame.
       */
      newgfp = (struct gf_marker *)(rsp + 1);
      newgfp->gf_gentype = G_Csusp;
      newgfp->gf_gfp = gfp;
      newgfp->gf_efp = efp;
      newgfp->gf_ipc = ipc;
      newgfp->gf_line = line;
      rsp += Wsizeof(struct gf_smallmarker);
   
      /*
       * Region extends from first word after the marker for the generator
       *  or expression frame enclosing the call to the now-suspending
       *  routine to the first argument of the routine.
       */
      if (gfp != 0) {
         if (gfp->gf_gentype == G_Psusp)
            firstwd = (word *)gfp + Wsizeof(*gfp);
         else
            firstwd = (word *)gfp + Wsizeof(struct gf_smallmarker);
         }
      else
         firstwd = (word *)efp + Wsizeof(*efp);
      lastwd = (word *)cargp + 1;

      /*
       * Copy the portion of the stack with endpoints firstwd and lastwd
       *  (inclusive) to the top of the stack.
       */
      for (wd = firstwd; wd <= lastwd; wd++)
         *++rsp = *wd;
      gfp = newgfp;
      }
   for (;;) {			/* Top of the interpreter loop */
#if (Instr % Instr_level) == 0
      if (sp > maxsp)
         maxsp = sp;
#endif Instr
      op = GetWord;		/* Instruction fetch */

      switch (op) {		/*
				 * Switch on opcode.  The cases are
				 * organized roughly by functionality
				 * to make it easier to find things.
				 * For some C compilers, there may be
				 * an advantage to arranging them by
				 * likelihood of selection.
				 */

				/* ---Constant construction--- */

         case Op_Cset:		/* cset */
            PutWord(Op_Acset);
            PushVal(D_Cset);
            opnd = GetWord;
            opnd += (word)ipc;
            PutWord(opnd);
            PushAVal(opnd);
            break;

         case Op_Acset:		/* cset, absolute address */
            PushVal(D_Cset);
            PushAVal(GetWord);
            break;


/* >int */
         case Op_Int:		/* integer */
            PushVal(D_Integer);
            PushVal(GetWord);
            break;
/* <int */

#if IntSize == 16
         case Op_Long:		/* long integer */
            PutWord(Op_Along);
            PushVal(D_Longint);
            opnd = GetWord;
            opnd += (word)(ipc);
            PutWord(opnd);
            PushAVal(opnd);
            break;

         case Op_Along:		/* long integer, absolute address */
            PushVal(D_Longint);
            PushAVal(GetWord);
            break;
#endif IntSize == 16

         case Op_Real:		/* real */
            PutWord(Op_Areal);
            PushVal(D_Real);
            opnd = GetWord;
            opnd += (word)ipc;
            PushAVal(opnd);
            PutWord(opnd);
            break;

         case Op_Areal:		/* real, absolute address */
            PushVal(D_Real);
            PushAVal(GetWord);
            break;

         case Op_Str:		/* string */
            PutWord(Op_Astr);
            PushVal(GetWord)
            opnd = (word)ident + GetWord;
            PutWord(opnd);
            PushAVal(opnd);
            break;

         case Op_Astr:		/* string, absolute address */
            PushVal(GetWord);
            PushAVal(GetWord);
            break;

				/* ---Variable construction--- */

         case Op_Arg:		/* argument */
            PushVal(D_Var);
            PushAVal(&argp[GetWord + 1]);
            break;

         case Op_Global:	/* global */
            PutWord(Op_Aglobal);
            PushVal(D_Var);
            opnd = GetWord;
            PushAVal(&globals[opnd]);
            PutWord((word)&globals[opnd]);
            break;

         case Op_Aglobal:	/* global, absolute address */
            PushVal(D_Var);
            PushAVal(GetWord);
            break;

         case Op_Local:		/* local */
            PushVal(D_Var);
            PushAVal(&pfp->pf_locals[GetWord]);
            break;

         case Op_Static:	/* static */
            PutWord(Op_Astatic);
            PushVal(D_Var);
            opnd = GetWord;
            PushAVal(&statics[opnd]);
            PutWord((word)&statics[opnd]);
            break;

         case Op_Astatic:	/* static, absolute address */
            PushVal(D_Var);
            PushAVal(GetWord);
            break;

				/* ---Operators--- */

    				/* Unconditional unary operators */

         case Op_Compl:		/* ~e */
         case Op_Neg:		/* -e */
         case Op_Number:	/* +e */
         case Op_Refresh:	/* ^e */
         case Op_Size:		/* *e */
         case Op_Value:		/* .e */
            Setup_Op(1);
            DerefArg(1);
            Call_Op(1);
            break;
				/* Conditional unary operators */

         case Op_Nonnull:	/* \e */
         case Op_Null:		/* /e */
            Setup_Op(1);
            Call_Cond(1);
            break;

         case Op_Random:	/* ?e */
            PushNull;
            Setup_Op(2)
            Call_Cond(2)
            break;

				/* Generative unary operators */

         case Op_Tabmat:	/* =e */
            Setup_Op(1);
            DerefArg(1);
            Call_Gen(1);
            break;

         case Op_Bang:		/* !e */
            PushNull;
            Setup_Op(2);
            Call_Gen(2);
            break;

				/* Unconditional binary operators */

         case Op_Cat:		/* e1 || e2 */
         case Op_Diff:		/* e1 -- e2 */
         case Op_Div:		/* e1 / e2 */
         case Op_Inter:		/* e1 ** e2 */
         case Op_Lconcat:	/* e1 ||| e2 */
         case Op_Minus:		/* e1 - e2 */
         case Op_Mod:		/* e1 % e2 */
         case Op_Mult:		/* e1 * e2 */
         case Op_Power:		/* e1 ^ e2 */
         case Op_Unions:	/* e1 ++ e2 */
/* >plus */
         case Op_Plus:		/* e1 + e2 */
            Setup_Op(2);
            DerefArg(1);
            DerefArg(2);
            Call_Op(2);
            break;
/* <plus */
				/* Conditional binary operators */

         case Op_Eqv:		/* e1 === e2 */
         case Op_Lexeq:		/* e1 == e2 */
         case Op_Lexge:		/* e1 >>= e2 */
         case Op_Lexgt:		/* e1 >> e2 */
         case Op_Lexle:		/* e1 <<= e2 */
         case Op_Lexlt:		/* e1 << e2 */
         case Op_Lexne:		/* e1 ~== e2 */
         case Op_Neqv:		/* e1 ~=== e2 */
         case Op_Numeq:		/* e1 = e2 */
         case Op_Numge:		/* e1 >= e2 */
         case Op_Numgt:		/* e1 > e2 */
         case Op_Numle:		/* e1 <= e2 */
         case Op_Numne:		/* e1 ~= e2 */
/* >numlt */
         case Op_Numlt:		/* e1 < e2 */
            Setup_Op(2);
            DerefArg(1);
            DerefArg(2);
            Call_Cond(2);
            break;
/* <numlt */

         case Op_Asgn:		/* e1 := e2 */
            Setup_Op(2);
            DerefArg(2);
            Call_Cond(2);
            break;

         case Op_Swap:		/* e1 :=: e2 */
            PushNull;
            Setup_Op(3);
            Call_Cond(3);
            break;

         case Op_Subsc:		/* e1[e2] */
            PushNull;
            Setup_Op(3);
            DerefArg(2);
            Call_Cond(3);
            break;
   				/* Generative binary operators */ 

         case Op_Rasgn:		/* e1 <- e2 */
            Setup_Op(2);
            DerefArg(2);
            Call_Gen(2);
            break;

         case Op_Rswap:		/* e1 <-> e2 */
            PushNull;
            Setup_Op(3);
            Call_Gen(3);
            break;

				/* Conditional ternary operators */

         case Op_Sect:		/* e1[e2:e3] */
            PushNull;
            Setup_Op(4);
            DerefArg(2);
            DerefArg(3);
            Call_Cond(4);
            break;
   				/* Generative ternary operators */ 

         case Op_Toby:		/* e1 to e2 by e3 */
            Setup_Op(3);
            DerefArg(1);
            DerefArg(2);
            DerefArg(3);
            Call_Gen(3);
            break;

				/* ---String Scanning--- */

         case Op_Bscan:		/* prepare for scanning */
            PushDesc(k_subject);
            PushVal(D_Integer);
            PushVal(k_pos);
            Setup_Op(0);
            signal = bscan(0,rargp);
            goto C_rtn_term;

         case Op_Escan:		/* exit from scanning */
            Setup_Op(3);
            signal = escan(3,rargp);
            goto C_rtn_term;

				/* ---Other Language Operations--- */

         case Op_Invoke: {	/* invoke */
            ExInterp;
            { int nargs;
              struct descrip *carg;

            type = invoke((int)GetWord, &carg, &nargs);
            rargp = carg;
            EntInterp;
            if (type == I_Goal_Fail)
               goto efail;
            if (type == I_Continue)
               break;
            else {
               int (*bfunc)();
               struct b_proc *bproc;

               bproc = (struct b_proc *)BlkLoc(*rargp);
               bfunc = bproc->entryp.ccode;

               /* ExInterp -- not needed since no change
                     since last EntInterp */
               if (type == I_Vararg)
                  signal = (*bfunc)(nargs,rargp);
               else
                  signal = (*bfunc)(rargp);
               goto C_rtn_term;
               }
            }
            break;
            }

         case Op_Keywd:		/* keyword */
            PushVal(D_Integer);
            PushVal(GetWord);
            Setup_Op(0);
            signal = keywd(0,rargp);
            break;

         case Op_Llist:		/* construct list */
            opnd = GetWord;
            Setup_Op(opnd);
            llist((int)opnd,rargp);
            rsp = (word *) rargp + 1;
            break;

				/* ---Marking and Unmarking--- */

         case Op_Mark:		/* create expression frame marker */
            PutWord(Op_Amark);
            opnd = GetWord;
            opnd += (word)ipc;
            PutWord(opnd);
            newefp = (struct ef_marker *)(rsp + 1);
            newefp->ef_failure = (word *)opnd;
            goto mark;

         case Op_Amark:		/* mark with absolute fipc */
            newefp = (struct ef_marker *)(rsp + 1);
            newefp->ef_failure = (word *)GetWord;
mark:
            newefp->ef_gfp = gfp;
            newefp->ef_efp = efp;
            newefp->ef_ilevel = ilevel;
            rsp += Wsizeof(*efp);
            efp = newefp;
            gfp = 0;
            break;

         case Op_Mark0:		/* create expression frame with 0 ipl */
mark0:
            newefp = (struct ef_marker *)(rsp + 1);
            newefp->ef_failure = 0;
            newefp->ef_gfp = gfp;
            newefp->ef_efp = efp;
            newefp->ef_ilevel = ilevel;
            rsp += Wsizeof(*efp);
            efp = newefp;
            gfp = 0;
            break;

/* >unmark */
         case Op_Unmark:	/* remove expression frame */
            gfp = efp->ef_gfp;
            rsp = (word *)efp - 1;

            /*
             * Remove any suspended C generators.
             */
Unmark_uw:
            if (efp->ef_ilevel != ilevel) {
               --ilevel;
               ExInterp;
               return A_Unmark_uw;
               }
            efp = efp->ef_efp;
            break;
/* <unmark */

				/* ---Suspensions--- */

         case Op_Esusp:	{	/* suspend from expression */

            /*
             * Create the generator frame.
             */
            oldsp = rsp;
            newgfp = (struct gf_marker *)(rsp + 1);
            newgfp->gf_gentype = G_Esusp;
            newgfp->gf_gfp = gfp;
            newgfp->gf_efp = efp;
            newgfp->gf_ipc = ipc;
            newgfp->gf_line = line;
            gfp = newgfp;
            rsp += Wsizeof(struct gf_smallmarker);

            /*
             * Region extends from first word after enclosing generator or
             *  expression frame marker to marker for current expression frame.
             */
            if (efp->ef_gfp != 0) {
               newgfp = (struct gf_marker *)(efp->ef_gfp);
               if (newgfp->gf_gentype == G_Psusp)
                  firstwd = (word *)efp->ef_gfp + Wsizeof(*gfp);
               else
                  firstwd = (word *)efp->ef_gfp + Wsizeof(struct gf_smallmarker);
                }
            else
               firstwd = (word *)efp->ef_efp + Wsizeof(*efp);
            lastwd = (word *)efp - 1;
            efp = efp->ef_efp;

            /*
             * Copy the portion of the stack with endpoints firstwd and lastwd
             *  (inclusive) to the top of the stack.
             */
            for (wd = firstwd; wd <= lastwd; wd++)
               *++rsp = *wd;
            PushVal(oldsp[-1]);
            PushVal(oldsp[0]);
            break;
            }

         case Op_Lsusp: {	/* suspend from limitation */
            struct descrip sval;

            /*
             * The limit counter is contained in the descriptor immediately
             *  prior to the current expression frame.  lval is established
             *  as a pointer to this descriptor.
             */
            struct descrip *lval = (struct descrip *)((word *)efp - 2);

            /*
             * Decrement the limit counter and check it.
             */
            if (--IntVal(*lval) != 0) {
               /*
                * The limit has not been reached, set up stack.
                */

               sval = *(struct descrip *)(rsp - 1);	/* save result */

               /*
                * Region extends from first word after enclosing generator or
                *  expression frame marker to the limit counter just prior to
                *  to the current expression frame marker.
                */
               if (efp->ef_gfp != 0) {
                  newgfp = (struct gf_marker *)(efp->ef_gfp);
                  if (newgfp->gf_gentype == G_Psusp)
                     firstwd = (word *)efp->ef_gfp + Wsizeof(*gfp);
                  else
                     firstwd = (word *)efp->ef_gfp + Wsizeof(struct gf_smallmarker);
                  }
               else
                  firstwd = (word *)efp->ef_efp + Wsizeof(*efp);
               lastwd = (word *)efp - 3;
               efp = efp->ef_efp;

               /*
                * Copy the portion of the stack with endpoints firstwd and lastwd
                *  (inclusive) to the top of the stack.
                */
               rsp -= 2;		/* overwrite result */
               for (wd = firstwd; wd <= lastwd; wd++)
                  *++rsp = *wd;
               PushDesc(sval);		/* push saved result */
               }
            else {
               /*
                * Otherwise, the limit has been reached.  Instead of
                *  suspending, remove the current expression frame and
                *  replace the limit counter with the value on top of
                *  the stack (which would have been suspended had the
                *  limit not been reached).
                */
               *lval = *(struct descrip *)(rsp - 1);
               gfp = efp->ef_gfp;

               /*
                * Since an expression frame is being removed, inactive
                *  C generators contained therein are deactivated.
                */
Lsusp_uw:
               if (efp->ef_ilevel != ilevel) {
                  --ilevel;
                  ExInterp;
                  return A_Lsusp_uw;
                  }
               rsp = (word *)efp - 1;
               efp = efp->ef_efp;
               }
            break;
            }

         case Op_Psusp: {	/* suspend from procedure */
            /*
             * An Icon procedure is suspending a value.  Determine if the
             *  value being suspended should be dereferenced and if so,
             *  dereference it. If tracing is on, strace is called
             * to generate a message.  Appropriate values are
             *  restored from the procedure frame of the suspending procedure.
             */

            struct descrip sval, *svalp;
            struct b_proc *sproc;

            svalp = (struct descrip *)(rsp - 1);
            sval = *svalp;
            if (Var(sval)) {
               word *loc;

               if (Tvar(sval)) {
                  if (sval.dword == D_Tvsubs) {
                      struct b_tvsubs *tvb;

                     tvb = (struct b_tvsubs *)BlkLoc(sval);
                     loc = (word *)BlkLoc(tvb->ssvar);
                     }
                  else
                     goto ps_noderef;
                  }
               else
                  loc = (word *)BlkLoc(sval);
               if (loc >= (word *)BlkLoc(current) && loc <= rsp)
                  deref(svalp);
               }
ps_noderef:

            /*
             * Create the generator frame.
             */
            oldsp = rsp;
            newgfp = (struct gf_marker *)(rsp + 1);
            newgfp->gf_gentype = G_Psusp;
            newgfp->gf_gfp = gfp;
            newgfp->gf_efp = efp;
            newgfp->gf_ipc = ipc;
            newgfp->gf_line = line;
            newgfp->gf_argp = argp;
            newgfp->gf_pfp = pfp;
            gfp = newgfp;
            rsp += Wsizeof(*gfp);

            /*
             * Region extends from first word after the marker for the generator
             *  or expression frame enclosing the call to the now-suspending
             *  procedure to Arg0 of the procedure.
             */
            if (pfp->pf_gfp != 0) {
               newgfp = (struct gf_marker *)(pfp->pf_gfp);
               if (newgfp->gf_gentype == G_Psusp)
                  firstwd = (word *)pfp->pf_gfp + Wsizeof(*gfp);
               else
                  firstwd = (word *)pfp->pf_gfp + Wsizeof(struct gf_smallmarker);
               }
            else
               firstwd = (word *)pfp->pf_efp + Wsizeof(*efp);
            lastwd = (word *)argp - 1;
               efp = efp->ef_efp;

            /*
             * Copy the portion of the stack with endpoints firstwd and lastwd
             *  (inclusive) to the top of the stack.
             */
            for (wd = firstwd; wd <= lastwd; wd++)
               *++rsp = *wd;
            PushVal(oldsp[-1]);
            PushVal(oldsp[0]);
            --k_level;
            if (k_trace) {
               sproc = (struct b_proc *)BlkLoc(*argp);
               strace(sproc, svalp);
               }
            line = pfp->pf_line;
            efp = pfp->pf_efp;
            ipc = pfp->pf_ipc;
            argp = pfp->pf_argp;
            pfp = pfp->pf_pfp;
            break;
            }

				/* ---Returns--- */

         case Op_Eret: {	/* return from expression */
            /*
             * Op_Eret removes the current expression frame, leaving the
             *  original top of stack value on top.
             */
            /*
             * Save current top of stack value in global temporary (no
             *  danger of reentry).
             */
            eret_tmp = *(struct descrip *)&rsp[-1];
            gfp = efp->ef_gfp;
Eret_uw:
            /*
             * Since an expression frame is being removed, inactive
             *  C generators contained therein are deactivated.
             */
            if (efp->ef_ilevel != ilevel) {
               --ilevel;
               ExInterp;
               return A_Eret_uw;
               }
            rsp = (word *)efp - 1;
            efp = efp->ef_efp;
            PushDesc(eret_tmp);
            break;
            }

         case Op_Pret: {	/* return from procedure */
            /*
             * An Icon procedure is returning a value.  Determine if the
             *  value being returned should be dereferenced and if so,
             *  dereference it.  If tracing is on, rtrace is called to
             *  generate a message.  Inactive generators created after
             *  the activation of the procedure are deactivated.  Appropriate
             *  values are restored from the procedure frame.
             */
            struct descrip rval;
            struct b_proc *rproc = (struct b_proc *)BlkLoc(*argp);

            *argp = *(struct descrip *)(rsp - 1);
            rval = *argp;
            if (Var(rval)) {
               word *loc;

               if (Tvar(rval)) {
                  if (rval.dword == D_Tvsubs) {
                      struct b_tvsubs *tvb;

                     tvb = (struct b_tvsubs *)BlkLoc(rval);
                     loc = (word *)BlkLoc(tvb->ssvar);
                     }
                  else
                     goto pr_noderef;
                  }
               else
                  loc = (word *)BlkLoc(rval);
               if (loc >= (word *)BlkLoc(current) && loc <= rsp)
                  deref(argp);
               }

pr_noderef:
            --k_level;
            if (k_trace)
               rtrace(rproc, argp);
Pret_uw:
            if (pfp->pf_ilevel != ilevel) {
               --ilevel;
               ExInterp;
               return A_Pret_uw;
               }
            rsp = (word *)argp + 1;
            line = pfp->pf_line;
            efp = pfp->pf_efp;
            gfp = pfp->pf_gfp;
            ipc = pfp->pf_ipc;
            argp = pfp->pf_argp;
            pfp = pfp->pf_pfp;
            break;
            }

				/* ---Failures--- */

/* >efail1 */
         case Op_Efail:
efail:
            /*
             * Failure has occurred in the current expression frame.
             */
            if (gfp == 0) {
               /*
                * There are no inactive generators to resume.  Remove
                *  the current expression frame, restoring values.
                *
                * If the failure address is 0, propagate failure to the
                *  enclosing frame by branching back to efail.
                */
               ipc = efp->ef_failure;
               gfp = efp->ef_gfp;
               rsp = (word *)efp - 1;
               efp = efp->ef_efp;
               if (ipc == 0)
                  goto efail;
               break;
               }

            else {
               /*
                * There is a generator that can be resumed.  Make
                *  the stack adjustments and then switch on the 
                *  type of the generator frame marker.
                */
               register struct gf_marker *resgfp = gfp;

               type = resgfp->gf_gentype;
/* <efail1 */
               if (type == G_Psusp) {
                  argp = resgfp->gf_argp;
                  if (k_trace) {	/* procedure tracing */
                     ExInterp;
                     atrace(BlkLoc(*argp));
                     EntInterp;
                     }
                  }
/* >efail2 */
               ipc = resgfp->gf_ipc;
               efp = resgfp->gf_efp;
               line = resgfp->gf_line;
               gfp = resgfp->gf_gfp;
               rsp = (word *)resgfp - 1;
/* <efail2 */
               if (type == G_Psusp) {
                  pfp = resgfp->gf_pfp;
                  ++k_level;		/* adjust procedure level */
                  }

/* >efail3 */
               switch (type) {

                  case G_Csusp: {
                     --ilevel;
                     ExInterp;
                     return A_Resumption;
                     break;
                     }

                  case G_Esusp:
                     goto efail;

                  case G_Psusp:
                     break;
                  }

               break;
               }
/* <efail3 */

         case Op_Pfail:		/* fail from procedure */
            /*
             * An Icon procedure is failing.  Generate tracing message if
             *  tracing is on.  Deactivate inactive C generators created
             *  after activation of the procedure.  Appropriate values
             *  are restored from the procedure frame.
             */
            --k_level;
            if (k_trace)
               ftrace(BlkLoc(*argp));
Pfail_uw:
            if (pfp->pf_ilevel != ilevel) {
               --ilevel;
               ExInterp;
               return A_Pfail_uw;
               }
            line = pfp->pf_line;
            efp = pfp->pf_efp;
            gfp = pfp->pf_gfp;
            ipc = pfp->pf_ipc;
            argp = pfp->pf_argp;
            pfp = pfp->pf_pfp;
            goto efail;

				/* ---Odds and Ends--- */

         case Op_Ccase:		/* case clause */
            PushNull;
            PushVal(((word *)efp)[-2]);
            PushVal(((word *)efp)[-1]);
            break;

         case Op_Chfail:	/* change failure ipc */
            opnd = GetWord;
            opnd += (word)ipc;
            efp->ef_failure = (word *)opnd;
            break;

         case Op_Dup:		/* duplicate descriptor */
            PushNull;
            rsp[1] = rsp[-3];
            rsp[2] = rsp[-2];
            rsp += 2;
            break;

         case Op_Field:		/* e1.e2 */
            PushVal(D_Integer);
            PushVal(GetWord);
            Setup_Op(2);
            signal = field(2,rargp);
            goto C_rtn_term;

         case Op_Goto:          /* goto */
            PutWord(Op_Agoto);
            opnd = GetWord;
            opnd += (word)ipc;
            PutWord(opnd);
            ipc = (word *)opnd;
            break;

         case Op_Agoto:         /* goto absolute address */
            opnd = GetWord;
            ipc = (word *)opnd;
            break;

         case Op_Init:		/* initial */
            *--ipc = Op_Goto;
            opnd = sizeof(*ipc) + sizeof(*rsp);
            opnd += (word)ipc;
            ipc = (word *)opnd;
            break;

         case Op_Limit:		/* limit */
            Setup_Op(0);
            if (limit(0,rargp) == A_Failure)
               goto efail;
            else
               rsp = (word *) rargp + 1;
            goto mark0;

         case Op_Line:		/* line */
            line = GetWord;
            break;

         case Op_Tally:		/* tally */
            tallybin[GetWord]++;
            break;

         case Op_Pnull:		/* push null descriptor */
            PushNull;
            break;

         case Op_Pop:		/* pop descriptor */
            rsp -= 2;
            break;

         case Op_Push1:		/* push integer 1 */
            PushVal(D_Integer);
            PushVal(1);
            break;

         case Op_Pushn1:	/* push integer -1 */
            PushVal(D_Integer);
            PushVal(-1);
            break;

         case Op_Sdup:		/* duplicate descriptor */
            rsp += 2;
            rsp[-1] = rsp[-3];
            rsp[0] = rsp[-2];
            break;

					/* ---Co-expressions--- */

         case Op_Create:	/* create */
	    PushNull;
            Setup_Op(0);
            opnd = GetWord;
            opnd += (word)ipc;
            signal = create((word *)opnd, rargp);
            goto C_rtn_term;


	 case Op_Coact: {	/* @e */
	    register struct b_coexpr *ccp, *ncp;
	    struct descrip *dp, *tvalp;
            word first;
	    
	    ExInterp;
	    dp = (struct descrip *)(sp - 1);
	    DeRef(*dp);
	    if (dp->dword != D_Coexpr)
	       runerr(118, dp);
	    ccp = (struct b_coexpr *)BlkLoc(current);
	    ncp = (struct b_coexpr *)BlkLoc(*dp);
            if (ncp->tvalloc != NULL)		/* Cannot activate co-expression */
               runerr(214, NULL);		/*  that is already active */
	    /*
	     * Save Istate of current co-expression.
	     */
	    ccp->es_pfp = pfp;
	    ccp->es_argp = argp;
	    ccp->es_efp = efp;
	    ccp->es_gfp = gfp;
	    ccp->es_ipc = ipc;
	    ccp->es_sp = sp;
	    ccp->es_ilevel = ilevel;
	    ccp->es_line = line;
	    ccp->tvalloc = (struct descrip *)(sp - 3);
	    /*
	     * Establish Istate for new co-expression.
	     */
	    pfp = ncp->es_pfp;
	    argp = ncp->es_argp;
	    efp = ncp->es_efp;
	    gfp = ncp->es_gfp;
	    ipc = ncp->es_ipc;
	    sp = ncp->es_sp;
	    ilevel = ncp->es_ilevel;
	    line = ncp->es_line;

            if (tvalp = ncp->tvalloc) {
	       ncp->tvalloc = NULL;
	       *tvalp = *(struct descrip *)(&ccp->es_sp[-3]);
               if (Var(*tvalp)) {
                  word *loc;
            
                  if (Tvar(*tvalp)) {
                     if (tvalp->dword == D_Tvsubs) {
                        struct b_tvsubs *tvb;
               
                        tvb = (struct b_tvsubs *)BlkLoc(*tvalp);
                        loc = (word *)BlkLoc(tvb->ssvar);
                        }
                     else
                        goto ca_noderef;
                     }
                  else
                     loc = (word *)BlkLoc(*tvalp);
                  if (loc >= (word *)ccp && loc <= ccp->es_sp)
                     deref(tvalp);
                  }
	       }
ca_noderef:
	    /*
	     * Set activator in new co-expression.
	     */
            if (ncp->activator.dword == D_Null)
               first = 0;
            else
               first = 1;
	    ncp->activator.dword = D_Coexpr;
	    BlkLoc(ncp->activator) = (union block *)ccp;
	    BlkLoc(current) = (union block *)ncp;
	    coexp_act = A_Coact;
	    coswitch(ccp->cstate,ncp->cstate,first);
	    EntInterp;
	    if (coexp_act == A_Cofail)
	       goto efail;
	    else
	       rsp -= 2;
	    break;
	    }
	    
         case Op_Coret: {	/* return from co-expression */
	    register struct b_coexpr *ccp, *ncp;
	    struct descrip *rvalp;
	    
	    ExInterp;
	    ccp = (struct b_coexpr *)BlkLoc(current);
	    ccp->size++;
	    ncp = (struct b_coexpr *)BlkLoc(ccp->activator);
	    ncp->tvalloc = NULL;
	    rvalp = (struct descrip *)(&ncp->es_sp[-3]);
	    *rvalp = *(struct descrip *)&sp[-1];
            if (Var(*rvalp)) {
               word *loc;
            
               if (Tvar(*rvalp)) {
                  if (rvalp->dword == D_Tvsubs) {
                      struct b_tvsubs *tvb;
               
                     tvb = (struct b_tvsubs *)BlkLoc(*rvalp);
                     loc = (word *)BlkLoc(tvb->ssvar);
                     }
                  else
                     goto cr_noderef;
                  }
               else
                  loc = (word *)BlkLoc(*rvalp);
               if (loc >= (word *)ccp && loc <= sp)
                  deref(rvalp);
               }
cr_noderef:
	    /*
	     * Save Istate of current co-expression.
	     */
	    ccp->es_pfp = pfp;
	    ccp->es_argp = argp;
	    ccp->es_efp = efp;
	    ccp->es_gfp = gfp;
	    ccp->es_ipc = ipc;
	    ccp->es_sp = sp;
	    ccp->es_ilevel = ilevel;
	    ccp->es_line = line;
	    /*
	     * Establish Istate for new co-expression.
	     */
	    pfp = ncp->es_pfp;
	    argp = ncp->es_argp;
	    efp = ncp->es_efp;
	    gfp = ncp->es_gfp;
	    ipc = ncp->es_ipc;
	    sp = ncp->es_sp;
	    ilevel = ncp->es_ilevel;
	    line = ncp->es_line;
	    BlkLoc(current) = (union block *)ncp;
	    coexp_act = A_Coret;
	    coswitch(ccp->cstate, ncp->cstate,(word)1);
	    break;
	    }

         case Op_Cofail: {	/* fail from co-expression */
	    register struct b_coexpr *ccp, *ncp;
	    
	    ExInterp;
	    ccp = (struct b_coexpr *)BlkLoc(current);
	    ncp = (struct b_coexpr *)BlkLoc(ccp->activator);
	    ncp->tvalloc = NULL;
	    /*
	     * Save Istate of current co-expression.
	     */
	    ccp->es_pfp = pfp;
	    ccp->es_argp = argp;
	    ccp->es_efp = efp;
	    ccp->es_gfp = gfp;
	    ccp->es_ipc = ipc;
	    ccp->es_sp = sp;
	    ccp->es_ilevel = ilevel;
	    ccp->es_line = line;
	    /*
	     * Establish Istate for new co-expression.
	     */
	    pfp = ncp->es_pfp;
	    argp = ncp->es_argp;
	    efp = ncp->es_efp;
	    gfp = ncp->es_gfp;
	    ipc = ncp->es_ipc;
	    sp = ncp->es_sp;
	    ilevel = ncp->es_ilevel;
	    line = ncp->es_line;
	    BlkLoc(current) = (union block *)ncp;
	    coexp_act = A_Cofail;
	    coswitch(ccp->cstate, ncp->cstate,(word)1);
	    break;
	    }

         case Op_Quit:		/* quit */
            goto interp_quit;

         default: {
            char buf[50];

            sprintf(buf, "unimplemented opcode: %ld\n",(long)op);
            syserr(buf);
            }
         }
         continue;

/* >crtn */
C_rtn_term:
         EntInterp;
         switch (signal) {

            case A_Failure:
               goto efail;

            case A_Unmark_uw:		/* unwind for unmark */
               goto Unmark_uw;

            case A_Lsusp_uw:		/* unwind for lsusp */
               goto Lsusp_uw;

            case A_Eret_uw:		/* unwind for eret */
               goto Eret_uw;

            case A_Pret_uw:		/* unwind for pret */
               goto Pret_uw;

            case A_Pfail_uw:		/* unwind for pfail */
               goto Pfail_uw;
            }

         rsp = (word *) rargp + 1;	/* set rsp to result */
         continue;
         }
/* <crtn */

interp_quit:
   --ilevel;
#if (Instr % Instr_level) == 0
   fprintf(stderr,"maximum ilevel = %d\n",maxilevel);
   fprintf(stderr,"maximum sp = %d\n",(long)maxsp - (long)stack);
   fflush(stderr);
#endif Instr
   if (ilevel != 0)
      syserr("Interpreter termination with inactive generators!");
   }
