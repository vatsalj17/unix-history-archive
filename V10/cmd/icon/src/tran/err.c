/*
 * Routines for producing error messages.
 */

#include "itran.h"
#include "token.h"
#include "tree.h"
#include "lex.h"

struct errmsg {
   int	e_state;		/* parser state number */
   char *e_mesg;		/* message text */
   } errtab[] = {
/*
 * Initialization of table that maps error states to messages.
 */

     1, "end of file expected",
     2, "global, record, or procedure declaration expected",
     9, "link list expected",
    11, "missing semicolon",
    14, "global, record, or procedure declaration expected",
    19, "missing record name",
    22, "invalid global declaration",
    23, "missing procedure name",
    24, "missing procedure name", /* ? */
    25, "missing link file name",
    26, "missing field list in record declaration",
    28, "missing end",
    29, "missing semicolon or operator",
    44, "invalid operand for unary operator",
    45, "invalid operand for unary operator",
    46, "invalid operand for unary operator",
    47, "invalid operand for unary operator",
    48, "invalid operand for unary operator",
    49, "invalid operand for unary operator",
    59, "invalid create expression",
    66, "invalid keyword construction",
    93, "invalid if control expression",
    94, "invalid case control expression",
    95, "invalid while control expression",
    96, "invalid until control expression",
    97, "invalid every control expression",
    98, "invalid repeat expression",
   101, "missing parameter list in procedure declaration",
   105, "invalid local declaration",
   106, "invalid initial expression",
   112, "invalid operand",
   113, "invalid operand",
   114, "invalid operand in assignment",
   115, "invalid operand in augmented assignment",
   116, "invalid operand in augmented assignment",
   117, "invalid operand in augmented assignment",
   118, "invalid operand in augmented assignment",
   119, "invalid to clause",
   149, "invalid operand in alternation",
   150, "invalid operand",
   165, "invalid operand",
   168, "invalid operand",
   173, "invalid operand",
   178, "invalid operand",
   179, "invalid operand",
   180, "invalid operand",
   183, "invalid reference or subscript", /* ? */
   184, "invalid field name",
   193, "missing right parenthesis", /* ? */
   194, "missing right brace",
   200, "missing right bracket", /* ? */
   201, "missing then",
   202, "missing of",
   207, "missing identifier",
   209, "missing right parenthesis",
   211, "missing end",
   212, "invalid declaration",
   213, "missing semicolon or operator",
   231, "missing right bracket",
   241, "missing right parenthesis", /* ? */
   242, "invalid then clause",
   243, "missing left brace",
   244, "invalid do clause",
   245, "invalid do clause",
   246, "invalid do clause",
   248, "invalid argument list",
   253, "invalid by clause",
   255, "invalid section",
   261, "missing right parenthesis",
   264, "missing right bracket",
   266, "invalid case clause",
   272, "missing right bracket or ampersand",
   273, "missing right brace",
   274, "missing right parenthesis",
   278, "invalid else clause",
   279, "missing right brace or semicolon",
   281, "missing colon",
   282, "missing colon or ampersand",
   288, "invalid case clause",
   289, "invalid default clause",
   290, "invalid case clause",
    -1,  "syntax error"
   };

/*
 * yyerror produces syntax error messages.  tok is the offending token
 *  (yychar), lval is yylval, and state is the parser's state.
 *
 * errtab is searched for the state, if it is found, the associated
 *  message is produced; if the state isn't found, "syntax error"
 *  is produced.
 */
yyerror(tok, lval, state)
int tok, state;
nodeptr lval;
   {
   register struct errmsg *p;
   char *mapterm();

   if (*filep)
      fprintf(stderr, "%s, ", *filep);
   if (tok == EOFX)   /* special case end of file */
      fprintf(stderr, "unexpected end of file\n");
   else {
      fprintf(stderr, "line %d: ", Line(lval));
      if (Col(lval))
         fprintf(stderr, "\"%s\": ", mapterm(tok,lval));
      for (p = errtab; p->e_state != state && p->e_state >= 0; p++) ;
      fprintf(stderr, "%s\n", p->e_mesg);
      }
   fatalerrs++;
   nocode++;
   }
/*
 * err produces the error messages s1 and s2 (if nonnull).  The
 *  current line number is found in tline.
 */
err(s1, s2)
char *s1, *s2;
   {
   if (*filep)
      fprintf(stderr, "%s, ", *filep);
   fprintf(stderr, "line %d: ", tline);
   if (s2)
      fprintf(stderr, "\"%s\": ", s2);
   fprintf(stderr, "%s\n", s1);
   fatalerrs++;
   nocode++;
   }

/*
 * lerr produces the error message s and associates it with line l.
 */
lerr(l, s)
int l;
char *s;
   {
   if (*filep)
      fprintf(stderr, "%s, ", *filep);
   fprintf(stderr, "line %d: ", l);
   fprintf(stderr, "%s\n", s);
   fatalerrs++;
   nocode++;
   }

/*
 * warn produces s1 and s2 (if nonnull) as warning messages.  The current
 *  line is in tline.
 */
warn(s1, s2)
char *s1, *s2;
   {
   if (*filep)
      fprintf(stderr, "%s, ", *filep);
   fprintf(stderr, "line %d: ", tline);
   if (s2)
      fprintf(stderr, "\"%s\": ", s2);
   fprintf(stderr, "%s\n", s1);
   warnings++;
   }

/*
 * syserr is called for fatal errors.  The message s is produced and the
 *  translator exits.
 */
syserr(s)
char *s;
   {
   if (*filep)
      fprintf(stderr, "%s, ", *filep);
   fprintf(stderr, "line %d: %s\n", inline, s);
   exit(ErrorExit);
   }

/*
 * mapterm finds a printable string for the given token type
 *  and value.
 */
char *mapterm(typ,val)
int typ;
nodeptr val;
   {
   register struct toktab *t;
   register i;

   i = typ;
   if (i == IDENT || i == INTLIT || i == REALLIT || i == STRINGLIT || i == CSETLIT)
      return Str0(val);
   for (t = toktab; t->t_type != i; t++)
      if (t->t_type == 0)
         return "???";
   return (t->t_word);
   }
