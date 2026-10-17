/*
 * Structure of a tree node.
 */

typedef	struct node	*nodeptr;

struct node {
   int n_type;			/* node type */
   int n_line;			/* line number in source program */
   int n_col;			/* column number in source program */
   union {
      int n_val;		/* integer-valued fields */
      char *n_str;		/* string-valued fields */
      nodeptr n_ptr;   	     	/* subtree pointers */
      } n_field[4];
   };

/*
 * Macros to access fields of parse tree nodes.
 */

#define Type(t)		t->n_type
#define Line(t)		t->n_line
#define Col(t)		t->n_col
#define Tree0(t)	t->n_field[0].n_ptr
#define Tree1(t)	t->n_field[1].n_ptr
#define Tree2(t)	t->n_field[2].n_ptr
#define Tree3(t)	t->n_field[3].n_ptr
#define Val0(t)		t->n_field[0].n_val
#define Val1(t)		t->n_field[1].n_val
#define Val2(t)		t->n_field[2].n_val
#define Val3(t)		t->n_field[3].n_val
#define Str0(t)		t->n_field[0].n_str
#define Str1(t)		t->n_field[1].n_str
#define Str2(t)		t->n_field[2].n_str
#define Str3(t)		t->n_field[3].n_str

/*
 * External declarations.
 */

extern nodeptr tree;		/* parse tree space */
extern nodeptr tfree;		/* free pointer for tree space */
extern nodeptr tend;		/* end of tree space */
extern int tsize;		/* parse tree size (integers) */
extern nodeptr tree1();		/* tree node allocator routines */
extern nodeptr tree3();
extern nodeptr tree4();
extern nodeptr tree5();
extern nodeptr tree6();
extern nodeptr tree7();
extern nodeptr yylval;		/* parser's current token value */

/*
 * Node types.
 */

#define N_Activat	 1		/* activation control structure */
#define N_Alt		 2		/* alternation operator */
#define N_Augop		 3		/* augmented operator */
#define N_Bar		 4		/* generator control structure */
#define N_Binop		 5		/* other binary operator */
#define N_Break		 6		/* break statement */
#define N_Case		 7		/* case statement */
#define N_Ccls		 8		/* case clause */
#define N_Clist		 9		/* list of case clauses */
#define N_Conj		10		/* conjunction operator */
#define N_Create	11		/* create control structure */
#define N_Cset		12		/* cset literal */
#define N_Elist		14		/* list of expressions */
#define N_Empty		15		/* empty expression or statement */
#define N_Field		16		/* record field reference */
#define N_Id		17		/* identifier token */
#define N_If		18		/* if-then-else statement */
#define N_Int		19		/* integer literal */
#define N_Invok		20		/* procedure call */
#define N_Key		21		/* keyword */
#define N_Limit		22		/* LIMIT control structure */
#define N_List		23		/* [ ... ] style list */
#define N_Loop		24		/* while, until, every, or repeat */
#define N_Not		25		/* not prefix control structure */
#define N_Next		26		/* next statement */
#define N_Op		27		/* operator token */
#define N_Proc		28		/* procedure */
#define N_Real		29		/* real literal */
#define N_Res		30		/* reserved word token */
#define N_Ret		31		/* fail, return, or succeed */
#define N_Scan		32		/* scan-using statement */
#define N_Sect		33		/* s[i:j] (section) */
#define N_Slist		34		/* list of statements */
#define N_Str		35		/* string literal */
#define N_Susp		36		/* suspend statement */
#define N_To		37		/* TO operator */
#define N_ToBy		38		/* TO-BY operator */
#define N_Unop		39		/* unary operator */

/*
 * Node constructor macros.
 */

#define ActivNode(a,b,c)	tree6(N_Activat,Line(a),Col(a),a,b,c) 
#define AltNode(a,b,c)		tree5(N_Alt,Line(a),Col(a),b,c) 
#define AugopNode(a,b,c)	tree6(N_Augop,Line(a),Col(a),a,b,c) 
#define BarNode(a)		tree4(N_Bar,Line(a),Col(a),a) 
#define BinopNode(a,b,c)	tree6(N_Binop,Line(a),Col(a),a,b,c) 
#define BreakNode(a,b)		tree4(N_Break,Line(a),Col(a),b) 
#define CaseNode(a,b,c)		tree5(N_Case,Line(a),Col(a),b,c) 
#define CclsNode(a,b,c)		tree5(N_Ccls,Line(a),Col(a),b,c) 
#define ClistNode(a,b,c)	tree5(N_Clist,Line(a),Col(a),b,c) 
#define ConjNode(a,b,c)		tree6(N_Conj,Line(a),Col(a),a,b,c) 
#define CreateNode(a,b)		tree4(N_Create,Line(a),Col(a),b) 
#define CsetNode(a,b)		tree5(N_Cset,tline,tcol,a,b) 
#define ElistNode(a,b,c)	tree5(N_Elist,Line(a),Col(a),b,c) 
#define EmptyNode		tree1(N_Empty) 
#define FieldNode(a,b,c)	tree5(N_Field,Line(a),Col(a),b,c) 
#define IdNode(a)		tree4(N_Id,tline,tcol,a) 
#define IfNode(a,b,c,d)		tree6(N_If,Line(a),Col(a),b,c,d) 
#define IntNode(a)		tree4(N_Int,tline,tcol,a) 
#define InvokNode(a,b,c)	tree5(N_Invok,Line(a),Col(a),b,c) 
#define KeyNode(a,b)		tree4(N_Key,Line(a),Col(a),b) 
#define LimitNode(a,b)		tree5(N_Limit,Line(a),Col(a),a,b) 
#define ListNode(a,b)		tree4(N_List,Line(a),Col(a),b) 
#define LoopNode(a,b,c)		tree6(N_Loop,Line(a),Col(a),a,b,c) 
#define NotNode(a)		tree4(N_Not,Line(a),Col(a),a) 
#define NextNode(a)		tree3(N_Next,Line(a),Col(a)) 
#define OpNode(a)		tree4(N_Op,tline,tcol,a) 
#define ProcNode(a,b,c,d)	tree7(N_Proc,Line(a),Col(a),a,b,c,d) 
#define RealNode(a)		tree4(N_Real,tline,tcol,a) 
#define ResNode(a)		tree4(N_Res,tline,tcol,a) 
#define RetNode(a,b)		tree5(N_Ret,Line(a),Col(a),a,b) 
#define ScanNode(a,b,c)		tree6(N_Scan,Line(a),Col(a),a,b,c) 
#define SectNode(a,b,c,d)	tree7(N_Sect,Line(a),Col(a),a,b,c,d) 
#define SListNode(a,b,c)	tree5(N_Slist,Line(a),Col(a),b,c) 
#define StrNode(a,b)		tree5(N_Str,tline,tcol,a,b) 
#define SuspNode(a,b)		tree4(N_Susp,Line(a),Col(a),b) 
#define ToByNode(a,b,c,d)	tree6(N_ToBy,Line(a),Col(a),b,c,d) 
#define ToNode(a,b,c)		tree5(N_To,Line(a),Col(a),b,c) 
#define UnopNode(a,b)		tree5(N_Unop,Line(a),Col(a),a,b) 
