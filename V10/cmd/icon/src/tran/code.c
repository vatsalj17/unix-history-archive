/*
 * Routines for constructing and traversing parse trees and generating code.
 */

#include "itran.h"
#include "token.h"
#include "tree.h"
#include "sym.h"
#include "tstats.h"

#ifdef TranStats
int TokCount[TokSize];
#endif TranStats

/*
 * Code generator parameters.
 */

#define LoopDepth   20		/* max. depth of nested loops */
#define CaseDepth   10		/* max. depth of nested case statements */
#define CreatDepth  10		/* max. depth of nested create statements */

/*
 * loopstk structures hold information about nested loops.
 */
struct loopstk {
   int nextlab;			/* label for next exit */
   int breaklab;		/* label for break exit */
   int markcount;		/* number of marks */
   int ltype;			/* loop type */
   };

/*
 * casestk structure hold information about case statements.
 */
struct casestk {
   int endlab;			/* label for exit from case statement */
   nodeptr deftree;		/* pointer to tree for default clause */
   };

/*
 * creatstk structures hold information about create statements.
 */
struct creatstk {
   int nextlab;			/* previous value of nextlab */
   int breaklab;		/* previous value of breaklab */
   };
static int nextlab;		/* next label allocated by alclab() */

/*
 *  tree[1-7] construct parse tree nodes with specified values.  tfree
 *   points at the next free word in the parse tree space.  Nodes are
 *   built by copying argument values into successive locations starting
 *   at tfree.  Parameters a and b are line and column information,
 *   while parameters c through f are values to be assigned to n_field[0-3].
 *   Note that this could be done with a single routine; a separate routine
 *   for each node size is used for speed and simplicity.
 */

nodeptr tree1(type)
int type;
   {
   register nodeptr t;

   t = tfree;
   tfree = (nodeptr) ((int *)tfree + 1);
   if (tfree > tend)
      syserr("out of tree space");
   t->n_type = type;
   return t;
   }

nodeptr tree3(type, a, b)
int type, a, b;
   {
   register nodeptr t;

   t = tfree;
   tfree = (nodeptr) ((int *)tfree + 3);
   if (tfree > tend)
      syserr("out of tree space");
   t->n_type = type;
   t->n_line = a;
   t->n_col = b;
   return t;
   }

nodeptr tree4(type, a, b, c)
int type, a, b, c;
   {
   register nodeptr t;

   t = tfree;
   tfree = (nodeptr) ((int *)tfree + 4);
   if (tfree > tend)
      syserr("out of tree space");
   t->n_type = type;
   t->n_line = a;
   t->n_col = b;
   t->n_field[0].n_val = c;
   return t;
   }

nodeptr tree5(type, a, b, c, d)
int type, a, b, c, d;
   {
   register nodeptr t;

   t = tfree;
   tfree = (nodeptr) ((int *)tfree + 5);
   if (tfree > tend)
      syserr("out of tree space");
   t->n_type = type;
   t->n_line = a;
   t->n_col = b;
   t->n_field[0].n_val = c;
   t->n_field[1].n_val = d;
   return t;
   }

nodeptr tree6(type, a, b, c, d, e)
int type, a, b, c, d, e;
   {
   register nodeptr t;

   t = tfree;
   tfree = (nodeptr) ((int *)tfree + 6);
   if (tfree > tend)
      syserr("out of tree space");
   t->n_type = type;
   t->n_line = a;
   t->n_col = b;
   t->n_field[0].n_val = c;
   t->n_field[1].n_val = d;
   t->n_field[2].n_val = e;
   return t;
   }

nodeptr tree7(type, a, b, c, d, e, f)
int type, a, b, c, d, e, f;
   {
   register nodeptr t;

   t = tfree;
   tfree = (nodeptr) ((int *)tfree + 7);
   if (tfree > tend)
      syserr("out of tree space");
   t->n_type = type;
   t->n_line = a;
   t->n_col = b;
   t->n_field[0].n_val = c;
   t->n_field[1].n_val = d;
   t->n_field[2].n_val = e;
   t->n_field[3].n_val = f;
   return t;
   }

/*
 * Clear the tree space by setting the free pointer back to the first word
 *  of the tree space.
 */

treeinit()
   {
   tfree = tree;
   }

/*
 * codegen - traverse tree t, generating code.
 */

codegen(t)
nodeptr t;
   {
   nextlab = 1;
   traverse(t);
   }

/*
 * traverse - traverse tree rooted at t and generate code.  This is just
 *  plug and chug code for each of the node types.  The tour goes into
 *  some detail about the code generation process, in particular, Appendix
 *  A describes the parse tree nodes.
 */

traverse(t)
register nodeptr t;
   {
   register int lab, n, i;
   struct loopstk loopsave;
   static struct loopstk loopstk[LoopDepth];	/* loop stack */
   static struct loopstk *loopsp;
   static struct casestk casestk[CaseDepth];	/* case stack */
   static struct casestk *casesp;
   static struct creatstk creatstk[CreatDepth];	/* create stack */
   static struct creatstk *creatsp;

   n = 1;
   switch (Type(t)) {

      case N_Activat:			/* co-expression activation */
         TokInc(AT);
         if (Val0(Tree0(t)) == AUGACT) {
            TokInc(AUGACT);
            TokDec(AT);
            emit("pnull");
            }
         traverse(Tree2(t));		/* evaluate result expression */
         if (Val0(Tree0(t)) == AUGACT)
            emit("sdup");
         traverse(Tree1(t));		/* evaluate activate expression */
         setline(Line(t));
         emit("coact");
         if (Val0(Tree0(t)) == AUGACT)
            emit("asgn");
         break;

      case N_Alt:			/* alternation */
         TokInc(BAR);
         lab = alclab(2);
         emitl("mark", lab);
         loopsp->markcount++;
         traverse(Tree0(t));		/* evaluate first alternative */
         loopsp->markcount--;
         emit("esusp");			/*  and suspend with its result */
         emitl("goto", lab+1);
         emitlab(lab);
         traverse(Tree1(t));		/* evaluate second alternative */
         emitlab(lab+1);
         break;

      case N_Augop:			/* augmented assignment */
      case N_Binop:			/*  or a binary operator */
         emit("pnull");
         traverse(Tree1(t));
         if (Type(t) == N_Augop)
            emit("dup");
         traverse(Tree2(t));
         setline(Line(t));
         binop(Val0(Tree0(t)));
         break;

      case N_Bar:			/* repeated alternation */
         TokInc(REPALT);
         lab = alclab(1);
         emitlab(lab);
         emit("mark0");		/* fail if expr fails first time */
         loopsp->markcount++;
         traverse(Tree0(t));		/* evaluate first alternative */
         loopsp->markcount--;
         emitl("chfail", lab);		/* change to loop on failure */
         emit("esusp");			/* suspend result */
         break;

      case N_Break:			/* break expression */
         TokInc(BREAK);
         if (loopsp->breaklab <= 0)
            lerr(Line(t), "invalid context for break");
         else {
            for (i = 0; i < loopsp->markcount; i++)
               emit("unmark");
            loopsave = *loopsp--;
            traverse(Tree0(t));
            *++loopsp = loopsave;
            emitl("goto", loopsp->breaklab);
            }
         break;

      case N_Case:			/* case expression */
         TokInc(CASE);
         lab = alclab(1);
         casesp++;
         casesp->endlab = lab;
         casesp->deftree = NULL;
         emit("mark0");
         loopsp->markcount++;
         traverse(Tree0(t));		/* evaluate control expression */
         loopsp->markcount--;
         emit("eret");
         traverse(Tree1(t));		/* do rest of case (CLIST) */
         if (casesp->deftree != NULL) {	/* evaluate default clause */
            emit("pop");
            traverse(casesp->deftree);
            }
         else
            emit("efail");
         emitlab(lab);			/* end label */
         casesp--;
         break;

      case N_Ccls:			/* case expression clause */
         TokInc(CCLS);
         if (Type(Tree0(t)) == N_Res &&	/* default clause */
             Val0(Tree0(t)) == DEFAULT) {
            if (casesp->deftree != NULL)
               lerr(Line(t), "more than one default clause");
            else
               casesp->deftree = Tree1(t);
            }
         else {				/* case clause */
            lab = alclab(1);
            emitl("mark", lab);
            loopsp->markcount++;
            emit("ccase");
            traverse(Tree0(t));		/* evaluate selector */
            setline(Line(t));
            emit("eqv");
            loopsp->markcount--;
            emit("unmark");
            emit("pop");
            traverse(Tree1(t));		/* evaluate expression */
            emitl("goto", casesp->endlab); /* goto end label */
            emitlab(lab);		/* label for next clause */
            }
         break;

      case N_Clist:			/* list of case clauses */
         traverse(Tree0(t));
         traverse(Tree1(t));
         break;

      case N_Conj:			/* conjunction */
         TokInc(CONJUNC);
         if (Val0(Tree0(t)) == AUGAND) {
            TokInc(AUGAND);
            TokDec(CONJUNC);
            emit("pnull");
            }
         traverse(Tree1(t));
         if (Val0(Tree0(t)) != AUGAND)
            emit("pop");
         traverse(Tree2(t));
         if (Val0(Tree0(t)) == AUGAND)
            emit("asgn");
         break;

      case N_Create:			/* create expression */
         TokInc(CREATE);
         creatsp++;
         creatsp->nextlab = loopsp->nextlab;
         creatsp->breaklab = loopsp->breaklab;
         loopsp->nextlab = 0;		/* make break and next illegal */
         loopsp->breaklab = 0;
         lab = alclab(3);
         emitl("goto", lab+2);		/* skip over code for co-expression */
         emitlab(lab);			/* entry point */
         emit("pop");			/* pop the result from activation */
         emitl("mark", lab+1);
         loopsp->markcount++;
         traverse(Tree0(t));		/* traverse code for co-expression */
         loopsp->markcount--;
         setline(Line(t));
         emit("coret");			/* return to activator */
         emit("efail");			/* drive co-expression */
         emitlab(lab+1);		/* loop on exhaustion */
         setline(0);
         setline(Line(t));
         emit("cofail");		/* and fail each time */
         emitl("goto", lab+1);
         emitlab(lab+2);
         setline(0);
         setline(Line(t));
         emitl("create", lab);		/* create entry block */
         loopsp->nextlab = creatsp->nextlab;   /* legalize break and next */
         loopsp->breaklab = creatsp->breaklab;
         creatsp--;
         break;

      case N_Cset:			/* cset literal */
         TokInc(CSETLIT);
         emitn("cset", Val0(t));
         break;

      case N_Elist:			/* expression list */
         n = traverse(Tree0(t));
         n += traverse(Tree1(t));
         break;

      case N_Empty:			/* a missing expression */
         TokInc(EMPTY);
         emit("pnull");
         break;

      case N_Field:			/* field reference */
         TokInc(DOT);
         emit("pnull");
         traverse(Tree0(t));
         setline(Line(t));
         emits("field", Str0(Tree1(t)));
         break;

      case N_Id:			/* identifier */
         TokInc(IDENT);
         emitn("var", Val0(t));
         break;

      case N_If:			/* if expression */
         if (Type(Tree2(t)) == N_Empty) {
            TokInc(IF);
            lab = 0;
            emit("mark0");
            }
         else {
            TokInc(ELSE);
            lab = alclab(2);
            emitl("mark", lab);
            }
         loopsp->markcount++;
         traverse(Tree0(t));
         loopsp->markcount--;
         emit("unmark");
         traverse(Tree1(t));
         if (lab > 0) {
            emitl("goto", lab+1);
            emitlab(lab);
            traverse(Tree2(t));
            emitlab(lab+1);
            }
         break;

      case N_Int:			/* integer literal */
         TokInc(INTLIT);
         emitn("int", Val0(t));
         break;

      case N_Invok:			/* invocation */
         if (Type(Tree0(t)) != N_Empty) {
            TokInc(INVOKE);
            traverse(Tree0(t));
             }
         else {
            TokInc(LPAREN);
            emit("pushn1");		/* assume -1(e1,...,en) */
            }
         n = traverse(Tree1(t));
         setline(Line(t));
         emitn("invoke", n);
         n = 1;
         break;

      case N_Key:			/* keyword reference */
          TokInc(KEYWORD);
         setline(Line(t));
         emitn("keywd", Val0(t));
         break;

      case N_Limit:			/* limitation */
         TokInc(BACKSLASH);
         traverse(Tree1(t));
         setline(Line(t));
         emit("limit");
         loopsp->markcount++;
         traverse(Tree0(t));
         loopsp->markcount--;
         emit("lsusp");
         break;

      case N_List:			/* list construction */
         TokInc(LIST);
         emit("pnull");
         if (Type(Tree0(t)) == N_Empty)
            n = 0;
         else
            n = traverse(Tree0(t));
         setline(Line(t));
         emitn("llist", n);
         n = 1;
         break;

      case N_Loop:			/* loop */
         switch (Val0(Tree0(t))) {
            case EVERY:
               TokInc(EVERY);
               lab = alclab(2);
               loopsp++;
               loopsp->ltype = EVERY;
               loopsp->nextlab = lab;
               loopsp->breaklab = lab + 1;
               loopsp->markcount = 1;
               emit("mark0");
               traverse(Tree1(t));
               emit("pop");
               if (Type(Tree2(t)) != N_Empty) {   /* every e1 do e2 */
                  emit("mark0");
                  loopsp->ltype = N_Loop;
                  loopsp->markcount++;
                  traverse(Tree2(t));
                  loopsp->markcount--;
                  emit("unmark");
                  }
               emitlab(loopsp->nextlab);
               emit("efail");
               emitlab(loopsp->breaklab);
               loopsp--;
               break;

            case REPEAT:
               TokInc(REPEAT);
               lab = alclab(3);
               loopsp++;
               loopsp->ltype = N_Loop;
               loopsp->nextlab = lab + 1;
               loopsp->breaklab = lab + 2;
               loopsp->markcount = 1;
               emitlab(lab);
               setline(0);
               setline(Line(t));
               emitl("mark", lab);
               traverse(Tree1(t));
               emitlab(loopsp->nextlab);
               emit("unmark");
               emitl("goto", lab);
               emitlab(loopsp->breaklab);
               loopsp--;
               break;

            case WHILE:
               TokInc(WHILE);
               lab = alclab(3);
               loopsp++;
               loopsp->ltype = N_Loop;
               loopsp->nextlab = lab + 1;
               loopsp->breaklab = lab + 2;
               loopsp->markcount = 1;
               emitlab(lab);
               setline(0);
               setline(Line(t));
               emit("mark0");
               traverse(Tree1(t));
               if (Type(Tree2(t)) != N_Empty) {
                  emit("unmark");
                  emitl("mark", lab);
                  traverse(Tree2(t));
                  }
               emitlab(loopsp->nextlab);
               emit("unmark");
               emitl("goto", lab);
               emitlab(loopsp->breaklab);
               loopsp--;
               break;

            case UNTIL:
               TokInc(UNTIL);
               lab = alclab(4);
               loopsp++;
               loopsp->ltype = N_Loop;
               loopsp->nextlab = lab + 2;
               loopsp->breaklab = lab + 3;
               loopsp->markcount = 1;
               emitlab(lab);
               setline(0);
               setline(Line(t));
               emitl("mark", lab+1);
               traverse(Tree1(t));
               emit("unmark");
               emit("efail");
               emitlab(lab+1);
               emitl("mark", lab);
               traverse(Tree2(t));
               emitlab(loopsp->nextlab);
               emit("unmark");
               emitl("goto", lab);
               emitlab(loopsp->breaklab);
               loopsp--;
               break;
            }
         break;

      case N_Next:			/* next expression */
         TokInc(NEXT);
         if (loopsp < loopstk || loopsp->nextlab <= 0)
            lerr(Line(t), "invalid context for next");
         else {
            if (loopsp->ltype != EVERY && loopsp->markcount > 1)
               for (i = 0; i < loopsp->markcount - 1; i++)
                  emit("unmark");
            emitl("goto", loopsp->nextlab);
            }
         break;

      case N_Not:			/* not expression */
         TokInc(NOT);
         lab = alclab(1);
         emitl("mark", lab);
         loopsp->markcount++;
         traverse(Tree0(t));
         loopsp->markcount--;
         emit("unmark");
         emit("efail");
         emitlab(lab);
         emit("pnull");
         break;

      case N_Proc:			/* procedure */
         TokInc(PROCEDURE);
         loopsp = loopstk;
         loopsp->nextlab = 0;
         loopsp->breaklab = 0;
         loopsp->markcount = 0;
         casesp = casestk;
         creatsp = creatstk;
         fprintf(codefile, "proc %s\n", Str0(Tree0(t)));
         lout(codefile);
         cout(codefile);
         emits("filen", *filep);
         emit("declend");
         setline(0);
         setline(Line(t));
         if (Type(Tree1(t)) != N_Empty) {
            lab = alclab(1);
            emitl("init", lab);
            emitl("mark", lab);
            traverse(Tree1(t));
            emit("unmark");
            emitlab(lab);
            }
         if (Type(Tree2(t)) != N_Empty)
            traverse(Tree2(t));
         setline(Line(Tree3(t)));
         emit("pfail");
         emit("end");
         if (!silence)
            fprintf(stderr, "  %s (%d/%d)\n", Str0(Tree0(t)),
                (int *)tfree - (int *)tree, tsize);
         break;

      case N_Real:			/* real literal */
         TokInc(REALLIT);
         emitn("real", Val0(t));
         break;

      case N_Ret:			/* return expression */
         TokInc(RETURN);
         if (creatsp > creatstk)
            lerr(Line(t), "invalid context for return or fail");
         if (Val0(Tree0(t)) != FAIL) {
            lab = alclab(1);
            emitl("mark", lab);
            loopsp->markcount++;
            traverse(Tree1(t));
            loopsp->markcount--;
            setline(Line(t));
            emit("pret");
            emitlab(lab);
            }
         setline(0);
         setline(Line(t));
         emit("pfail");
         break;

      case N_Scan:			/* scanning expression */
         TokInc(QMARK);
         if (Val0(Tree0(t)) == SCANASGN)
            emit("pnull");
         traverse(Tree1(t));
         if (Val0(Tree0(t)) == SCANASGN)
            emit("sdup");
         setline(Line(t));
         emit("bscan");
         traverse(Tree2(t));
         setline(Line(t));
         emit("escan");
         if (Val0(Tree0(t)) == SCANASGN)
            emit("asgn");
         break;

      case N_Sect:			/* section operation */
         TokInc(SECTION);
         emit("pnull");
         traverse(Tree1(t));
         traverse(Tree2(t));
         if (Val0(Tree0(t)) == PCOLON || Val0(Tree0(t)) == MCOLON)
            emit("dup");
         traverse(Tree3(t));
         setline(Line(Tree0(t)));
         if (Val0(Tree0(t)) == PCOLON)
            emit("plus");
         else if (Val0(Tree0(t)) == MCOLON)
            emit("minus");
         setline(Line(t));
         emit("sect");
         break;

      case N_Slist:			/* semicolon separated list of expressions */
         TokInc(COMPOUND);
         lab = alclab(1);
         emitl("mark", lab);
         loopsp->markcount++;
         traverse(Tree0(t));
         loopsp->markcount--;
         emit("unmark");
         emitlab(lab);
         traverse(Tree1(t));
         break;

      case N_Str:			/* string literal */
         TokInc(STRINGLIT);
         emitn("str", Val0(t));
         break;

      case N_Susp:			/* suspension expression */
         TokInc(SUSPEND);
         if (creatsp > creatstk)
            lerr(Line(t), "invalid context for suspend");
         emit("mark0");
         loopsp->markcount++;
         traverse(Tree0(t));
         loopsp->markcount--;
         setline(Line(t));
         emit("psusp");
         emit("efail");
         break;

      case N_To:			/* to expression */
         TokInc(TO);
         emit("pnull");
         traverse(Tree0(t));
         traverse(Tree1(t));
         emit("push1");
         setline(Line(t));
         emit("toby");
         break;

      case N_ToBy:			/* to-by expression */
         TokInc(BY);
         emit("pnull");
         traverse(Tree0(t));
         traverse(Tree1(t));
         traverse(Tree2(t));
         setline(Line(t));
         emit("toby");
         break;

      case N_Unop:			/* unary operator */
         unopa(Val0(Tree0(t)));
         traverse(Tree1(t));
         setline(Line(t));
         unopb(Val0(Tree0(t)));
         break;

      default:
         emitn("?????", Type(t));
         syserr("traverse: undefined node type");
      }
   return n;
   }
/*
 * binop emits code for binary operators.  For non-augmented operators,
 *  the name of operator is emitted.  For augmented operators, an "asgn"
 *  is emitted after the name of the operator.
 */
binop(op)
int op;
   {
   register int asgn;
   register char *name;

   asgn = 0;
   switch (op) {

      case ASSIGN:
         TokInc(ASSIGN);
         name = "asgn";
         break;

      case CARETASGN:
         TokInc(CARETASGN);
         TokDec(CARET);
         asgn++;
      case CARET:
         TokInc(CARET);
         name = "power";
         break;

      case CONCATASGN:
         TokInc(CONCATASGN);
         TokDec(CONCAT);
         asgn++;
      case CONCAT:
         TokInc(CONCAT);
         name = "cat";
         break;

      case DIFFASGN:
         TokInc(DIFFASGN);
         TokDec(DIFF);
         asgn++;
      case DIFF:
         TokInc(DIFF);
         name = "diff";
         break;

      case AUGEQV:
         TokInc(AUGEQV);
         TokDec(EQUIV);
         asgn++;
      case EQUIV:
         TokInc(EQUIV);
         name = "eqv";
         break;

      case INTERASGN:
         TokInc(INTERASGN);
         TokDec(INTER);
         asgn++;
      case INTER:
         TokInc(INTER);
         name = "inter";
         break;

      case LBRACK:
         TokInc(LBRACK);
         name = "subsc";
         break;

      case LCONCATASGN:
         TokInc(LCONCATASGN);
         TokDec(LCONCAT);
         asgn++;
      case LCONCAT:
         TokInc(LCONCAT);
         name = "lconcat";
         break;

      case AUGSEQ:
         TokInc(AUGSEQ);
         TokDec(LEXEQ);
         asgn++;
      case LEXEQ:
         TokInc(LEXEQ);
         name = "lexeq";
         break;

      case AUGSGE:
         TokInc(AUGSGE);
         TokDec(LEXGE);
         asgn++;
      case LEXGE:
         TokInc(LEXGE);
         name = "lexge";
         break;

      case AUGSGT:
         TokInc(AUGSGT);
         TokDec(LEXGT);
         asgn++;
      case LEXGT:
         TokInc(LEXGT);
         name = "lexgt";
         break;

      case AUGSLE:
         TokInc(AUGSLE);
         TokDec(LEXLE);
         asgn++;
      case LEXLE:
         TokInc(LEXLE);
         name = "lexle";
         break;

      case AUGSLT:
         TokInc(AUGSLT);
         TokDec(LEXLT);
         asgn++;
      case LEXLT:
         TokInc(LEXLT);
         name = "lexlt";
         break;

      case AUGSNE:
         TokInc(AUGSNE);
         TokDec(LEXNE);
         asgn++;
      case LEXNE:
         TokInc(LEXNE);
         name = "lexne";
         break;

      case MINUSASGN:
         TokInc(MINUSASGN);
         TokDec(MINUS);
         asgn++;
      case MINUS:
         TokInc(MINUS);
         name = "minus";
         break;

      case MODASGN:
         TokInc(MODASGN);
         TokDec(MOD);
         asgn++;
      case MOD:
         TokInc(MOD);
         name = "mod";
         break;

      case AUGNEQV:
         TokInc(AUGNEQV);
         TokDec(NOTEQUIV);
         asgn++;
      case NOTEQUIV:
         TokInc(NOTEQUIV);
         name = "neqv";
         break;

      case AUGEQ:
         TokInc(AUGEQ);
         TokDec(NUMEQ);
         asgn++;
      case NUMEQ:
         TokInc(NUMEQ);
         name = "numeq";
         break;

      case AUGGE:
         TokInc(AUGGE);
         TokDec(NUMGE);
         asgn++;
      case NUMGE:
         TokInc(NUMGE);
         name = "numge";
         break;

      case AUGGT:
         TokInc(AUGGT);
         TokDec(NUMGT);
         asgn++;
      case NUMGT:
         TokInc(NUMGT);
         name = "numgt";
         break;

      case AUGLE:
         TokInc(AUGLE);
         TokDec(NUMLE);
         asgn++;
      case NUMLE:
         TokInc(NUMLE);
         name = "numle";
         break;

      case AUGLT:
         TokInc(AUGLT);
         TokDec(NUMLT);
         asgn++;
      case NUMLT:
         TokInc(NUMLT);
         name = "numlt";
         break;

      case AUGNE:
         TokInc(AUGNE);
         TokDec(NUMNE);
         asgn++;
      case NUMNE:
         TokInc(NUMNE);
         name = "numne";
         break;

      case PLUSASGN:
         TokInc(PLUSASGN);
         TokDec(PLUS);
         asgn++;
      case PLUS:
         TokInc(PLUS);
         name = "plus";
         break;

      case REVASSIGN:
         TokInc(REVASSIGN);
         name = "rasgn";
         break;

      case REVSWAP:
         TokInc(REVSWAP);
         name = "rswap";
         break;

      case SLASHASGN:
         TokInc(SLASHASGN);
         TokDec(SLASH);
         asgn++;
      case SLASH:
         TokInc(SLASH);
         name = "div";
         break;

      case STARASGN:
         TokInc(STARASGN);
         TokDec(STAR);
         asgn++;
      case STAR:
         TokInc(STAR);
         name = "mult";
         break;

      case SWAP:
         TokInc(SWAP);
         name = "swap";
         break;

      case UNIONASGN:
         TokInc(UNIONASGN);
         TokDec(UNION);
         asgn++;
      case UNION:
         TokInc(UNION);
         name = "unions";
         break;

      default:
         emitn("?binop", op);
         syserr("binop: undefined binary operator");
      }
   emit(name);
   if (asgn)
      emit("asgn");
   return;
   }
/*
 * unopa and unopb handle code emission for unary operators. unary operator
 *  sequences that are the same as binary operator sequences are recognized
 *  by the lexical analyzer as binary operators.  For example, ~===x means to
 *  do three tab(match(...)) operations and then a cset complement, but the
 *  lexical analyzer sees the operator sequence as the "neqv" binary
 *  operation.  unopa and unopb unravel tokens of this form.
 *
 * When a N_Unop node is encountered, unopa is called to emit the necessary
 *  number of "pnull" operations to receive the intermediate results.  This
 *  amounts to a pnull for each operation.
 */
unopa(op)
int op;
   {
   switch (op) {
      case NOTEQUIV:		/* unary ~ and three = operators */
         emit("pnull");
      case LEXNE:		/* unary ~ and two = operators */
      case EQUIV:		/* three unary = operators */
         emit("pnull");
      case NUMNE:		/* unary ~ and = operators */
      case UNION:		/* two unary + operators */
      case DIFF:		/* two unary - operators */
      case LEXEQ:		/* two unary = operators */
      case INTER:		/* two unary * operators */
         emit("pnull");
      case DOT:			/* unary . operator */
      case BACKSLASH:		/* unary \ operator */
      case BANG:		/* unary ! operator */
      case CARET:		/* unary ^ operator */
      case PLUS:		/* unary + operator */
      case TILDE:		/* unary ~ operator */
      case MINUS:		/* unary - operator */
      case NUMEQ:		/* unary = operator */
      case STAR:		/* unary * operator */
      case QMARK:		/* unary ? operator */
      case SLASH:		/* unary / operator */
         emit("pnull");
         break;
      default:
         syserr("unopa: undefined unary operator");
      }
   return;
   }
/*
 * unopb is the back-end code emitter for unary operators.  It emits
 *  the operations represented by the token op.  For tokens representing
 *  a single operator, the name of the operator is emitted.  For tokens
 *  representing a sequence of operators, recursive calls are used.  In
 *  such a case, the operator sequence is "scanned" from right to left
 *  and unopb is called with the token for the appropriate operation.
 *
 * For example, consider the sequence of calls and code emission for "~===":
 *	unopb(NOTEQUIV)		~===
 *	    unopb(NUMEQ)	=
 * 		emits "tabmat"
 *	    unopb(NUMEQ)	=
 *		emits "tabmat"
 *	    unopb(NUMEQ)	=
 *		emits "tabmat"
 *	    emits "compl"
 */
unopb(op)
int op;
   {
   register char *name;

   switch (op) {

      case DOT:			/* unary . operator */
         TokInc(UDOT);
         name = "value";
         break;

      case BACKSLASH:		/* unary \ operator */
         TokInc(UBACK);
         name = "nonnull";
         break;

      case BANG:		/* unary ! operator */
         TokInc(BANG);
         name = "bang";
         break;

      case CARET:		/* unary ^ operator */
         TokInc(UCARET);
         name = "refresh";
         break;

      case UNION:		/* two unary + operators */
         TokInc(UPLUS);
         unopb(PLUS);
      case PLUS:		/* unary + operator */
         TokInc(UPLUS);
         name = "number";
         break;

      case NOTEQUIV:		/* unary ~ and three = operators */
         TokInc(UEQUAL);
         unopb(NUMEQ);
         TokInc(UEQUAL);
      case LEXNE:		/* unary ~ and two = operators */
         TokInc(UEQUAL);
         unopb(NUMEQ);
      case NUMNE:		/* unary ~ and = operators */
         TokInc(UEQUAL);
         unopb(NUMEQ);
      case TILDE:		/* unary ~ operator (cset compl) */
         TokInc(TILDE);
         name = "compl";
         break;

      case DIFF:		/* two unary - operators */
         TokInc(UMINUS);
         unopb(MINUS);
      case MINUS:		/* unary - operator */
         TokInc(UMINUS);
         name = "neg";
         break;

      case EQUIV:		/* three unary = operators */
         TokInc(UEQUAL);
         unopb(NUMEQ);
      case LEXEQ:		/* two unary = operators */
         TokInc(UEQUAL);
         unopb(NUMEQ);
      case NUMEQ:		/* unary = operator */
         TokInc(UEQUAL);
         name = "tabmat";
         break;

      case INTER:		/* two unary * operators */
         TokInc(USTAR);
         unopb(STAR);
      case STAR:		/* unary * operator */
         TokInc(USTAR);
         name = "size";
         break;

      case QMARK: 		/* unary ? operator */
         TokInc(UQUES);
         name = "random";
         break;

      case SLASH:		/* unary / operator */
         TokInc(USLASH);
         name = "null";
         break;

      default:
         emitn("?unop", op);
         syserr("unopb: undefined unary operator");
      }
   emit(name);
   return;
   }

/*
 * setline emits a "line" instruction for line n.  A "line" instruction is not
 *  emitted if the last "line" instruction was also for line n.
 */
setline(n)
int n;
   {
   static lastline = 0;

   if (n != lastline) {
      lastline = n;
      if (n > 0)
         emitn("line", n);
      }
   }
/*
 * The emit* routines output ucode to codefile.  The various routines are:
 *
 *  emitlab(l) - emit "lab" instruction for label l.
 *  emit(s) - emit instruction s.
 *  emitl(s,a) - emit instruction s with reference to label a.
 *  emitn(s,n) - emit instruction s with numeric operand a.
 *  emitnl(s,a,b) - emit instruction s with numeric operand a and label b.
 *  emits(s,a) - emit instruction s with string operand a.
 */
emitlab(l)
int l;
   {
   fprintf(codefile, "lab L%d\n", l);
   }

emit(s)
char *s;
   {
   fprintf(codefile, "\t%s\n", s);
   }

emitl(s, a)
char *s;
int a;
   {
   fprintf(codefile, "\t%s\tL%d\n", s, a);
   }

emitn(s, a)
char *s;
int a;
   {
   fprintf(codefile, "\t%s\t%d\n", s, a);
   }

emitnl(s, a, b)
char *s;
int a, b;
   {
   fprintf(codefile, "\t%s\t%d,L%d\n", s, a, b);
   }

emits(s, a)
char *s, *a;
   {
   fprintf(codefile, "\t%s\t%s\n", s, a);
   }
/*
 * alclab allocates n labels and returns the first.  For the interpreter,
 *  labels are restarted at 1 for each procedure, while in the compiler,
 *  they start at 1 and increase throughout the entire compilation.
 */
alclab(n)
int n;
   {
   register int lab;

   lab = nextlab;
   nextlab += n;
   return lab;
   }

#ifdef TranStats
tokdump()
   {
   int i;

   fprintf(stderr,"begin tokdump\n");
   for (i = 0; i < TokSize; i++)
      fprintf(stderr,"%d\n",TokCount[i]);
   fprintf(stderr,"end tokdump\n");
   fflush(stderr);
   }
#endif TranStats
