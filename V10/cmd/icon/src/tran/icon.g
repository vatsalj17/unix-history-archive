/*	Grammar for Icon Version 5.10.	*/

/* primitive tokens */

%token	CSETLIT
	EOFX
	IDENT
	INTLIT
	REALLIT
	STRINGLIT

/* reserved words */

%token	BREAK		/* break */
	BY		/* by */
	CASE		/* case */
	CREATE		/* create */
	DEFAULT		/* default */
	DO		/* do */
	DYNAMIC		/* dynamic */
	ELSE		/* else */
	END		/* end */
	EVERY		/* every */
	FAIL		/* fail */
	GLOBAL		/* global */
	IF		/* if */
	INITIAL		/* initial */
	LINK		/* link */
	LOCAL		/* link */
	NEXT		/* next */
	NOT		/* not */
	OF		/* of */
	PROCEDURE	/* procedure */
	RECORD		/* record */
	REPEAT		/* repeat */
	RETURN		/* return */
	STATIC		/* static */
	SUSPEND		/* suspend */
	THEN		/* then */
	TO		/* to */
	UNTIL		/* until */
	WHILE		/* while */

/* operators */

%token	ASSIGN		/* := */
	AT		/* @ */
	AUGACT		/* @:= */
	AUGAND		/* &:= */
	AUGEQ		/* =:= */
	AUGEQV		/* ===:= */
	AUGGE		/* >=:= */
	AUGGT		/* >:= */
	AUGLE		/* <=:= */
	AUGLT		/* <:= */
	AUGNE		/* ~=:= */
	AUGNEQV		/* ~===:= */
	AUGSEQ		/* ==:= */
	AUGSGE		/* >>=:= */
	AUGSGT		/* >>:= */
	AUGSLE		/* <<=:= */
	AUGSLT		/* <<:= */
	AUGSNE		/* ~==:= */
	BACKSLASH	/* \ */
	BANG		/* ! */
	BAR		/* | */
	CARET		/* ^ */
	CARETASGN	/* ^:= */
	COLON		/* : */
	COMMA		/* , */
	CONCAT		/* || */
	CONCATASGN	/* ||:= */
	CONJUNC		/* & */
	DIFF		/* -- */
	DIFFASGN	/* --:= */
	DOT		/* . */
	EQUIV		/* === */
	INTER		/* ** */
	INTERASGN	/* **:= */
	LBRACE		/* { */
	LBRACK		/* [ */
	LCONCAT		/* ||| */
	LCONCATASGN	/* |||:= */
	LEXEQ		/* == */
	LEXGE		/* >>= */
	LEXGT		/* >> */
	LEXLE		/* <<= */
	LEXLT		/* << */
	LEXNE		/* ~== */
	LPAREN		/* ( */
	MCOLON		/* -: */
	MINUS		/* - */
	MINUSASGN	/* -:= */
	MOD		/* % */
	MODASGN		/* %:= */
	NOTEQUIV	/* ~=== */
	NUMEQ		/* = */
	NUMGE		/* >= */
	NUMGT		/* > */
	NUMLE		/* <= */
	NUMLT		/* < */
	NUMNE		/* ~= */
	PCOLON		/* +: */
	PLUS		/* + */
	PLUSASGN	/* +:= */
	QMARK		/* ? */
	RBRACE		/* } */
	RBRACK		/* ] */
	REVASSIGN	/* <- */
	REVSWAP		/* <-> */
	RPAREN		/* ) */
	SCANASGN	/* ?:= */
	SEMICOL		/* ; */
	SLASH		/* / */
	SLASHASGN	/* /:= */
	STAR		/* * */
	STARASGN	/* *:= */
	SWAP		/* :=: */
	TILDE		/* ~ */
	UNION		/* ++ */
	UNIONASGN	/* ++:= */
%{
#include "itran.h"
#include "sym.h"
#include "tree.h"
#include "../h/keyword.h"
#define YYSTYPE nodeptr
#define YYMAXDEPTH 500
%}

%%

%{
int argcnt;
int idflag;
int i;
int cstack[50];			/* context stack expression lists */
int stacktop = 0;		/* stack top */
nodeptr cswitch();
%}

program	: decls EOFX {gout(globfile);} ;

decls	: ;	
	| decls decl ;

decl	: record {
		if (!nocode)
			rout(globfile, Str0($1));
		nocode = 0;
		loc_init();
		} ;
	| proc	{
		if (!nocode)
			codegen($1);
		nocode = 0;
		treeinit();
		loc_init();
		} ;
	| global ;
	| LINK lnklist ;

lnklist	: lnkfile	;
	| lnklist COMMA lnkfile;

lnkfile	: IDENT {addlfile(Str0($1));} ;
	| STRINGLIT {addlfile(Str0($1));} ;

global	: GLOBAL {idflag = F_Global;} idlist ;

record	: RECORD {idflag = F_Argument;} IDENT LPAREN arglist RPAREN {
		install(Str0($3),F_Record|F_Global,(int)$5);
		$$ = $3;
		} ;

proc	: prochead SEMICOL locals initial procbody END {
		$$ = (nodeptr)ProcNode($1,$4,$5,$6);
		} ;

prochead: PROCEDURE {idflag = F_Argument;} IDENT LPAREN arglist RPAREN {
		$$ = $3;
		install(Str0($3),F_Proc|F_Global,(int)$5);
		} ;

arglist	: {$$ = (int)0;} ;
	| idlist {$$ = (nodeptr)$1;} ;


idlist	: IDENT {
		install(Str0($1),idflag,0);
		$$ = (nodeptr)1;
		} ;
	| idlist COMMA IDENT {
		install(Str0($3),idflag,0);
		$$ = (nodeptr)((int)$1 + 1);
		} ;

locals	: ;
	| locals retention idlist SEMICOL ;

retention: LOCAL {idflag = F_Dynamic;} ;
	|	STATIC {idflag = F_Static;} ;
	|	DYNAMIC {idflag = F_Dynamic;} ;

initial	: {$$ = EmptyNode;} ;
	| INITIAL expr SEMICOL {$$ = $2;} ;

procbody: {$$ = EmptyNode;} ;
	| nexpr SEMICOL procbody {$$ = SListNode($2, $1, $3);} ;

nexpr	: {$$ = EmptyNode;} ;
	| expr ;

expr	: expr1a	;
	| expr CONJUNC expr1a	{$$ = ConjNode($2,$1,$3);} ;

expr1a	: expr1	;
	| expr1a QMARK expr1	{$$ = ScanNode($2,$1,$3);} ;

expr1	: expr2 ;
	| expr2 op1 expr1 {binop: $$ = BinopNode($2,$1,$3);} ;
	| expr2 op1a expr1 {$$ = AugopNode($2,$1,$3);} ;
	| expr2 SCANASGN expr1 {$$ = ScanNode($2,$1,$3);} ;
	| expr2 AUGAND expr1 {$$ = ConjNode($2,$1,$3);} ;
	| expr2 AUGACT expr1 {$$ = ActivNode($2,$3,$1);} ;

op1	: SWAP ;
	| ASSIGN ;
	| REVSWAP ;
	| REVASSIGN ;

op1a	: CONCATASGN ;
	| LCONCATASGN ;
	| DIFFASGN ;
	| UNIONASGN ;
	| PLUSASGN ;
	| MINUSASGN ;
	| STARASGN ;
	| INTERASGN ;
	| SLASHASGN ;
	| MODASGN ;
	| CARETASGN ;
	| AUGEQ ;
	| AUGEQV ;
	| AUGGE ;
	| AUGGT ;
	| AUGLE ;
	| AUGLT ;
	| AUGNE ;
	| AUGNEQV ;
	| AUGSEQ ;
	| AUGSGE ;
	| AUGSGT ;
	| AUGSLE ;
	| AUGSLT ;
	| AUGSNE ;

expr2	: expr3 ;
	| expr2 TO expr3 {$$ = ToNode($2,$1,$3);} ;
	| expr2 TO expr3 BY expr3 {$$ = ToByNode($2,$1,$3,$5);} ;

expr3	: expr4 ;
	| expr4 BAR expr3 {$$ = AltNode($2,$1,$3);} ;

expr4	: expr5 ;
	| expr4 op4 expr5 {goto binop;} ;

op4	: LEXEQ ;
	| LEXGE ;
	| LEXGT ;
	| LEXLE ;
	| LEXLT ;
	| LEXNE ;
	| NUMEQ ;
	| NUMGE ;
	| NUMGT ;
	| NUMLE ;
	| NUMLT ;
	| NUMNE ;
	| EQUIV ;
	| NOTEQUIV ;

expr5	: expr6 ;
	| expr5 op5 expr6 {goto binop;} ;

op5	: CONCAT ;
	| LCONCAT ;

expr6	: expr7 ;
	| expr6 op6 expr7 {goto binop;} ;

op6	: PLUS ;
	| DIFF ;
	| UNION ;
	| MINUS ;

expr7	: expr8 ;
	| expr7 op7 expr8 {goto binop;} ;

op7	: STAR ;
	| INTER ;
	| SLASH ;
	| MOD ;

expr8	: expr9 ;
	| expr9 CARET expr8 {goto binop;} ;

expr9	: expr10 ;
	| expr9 BACKSLASH expr10 {$$ = LimitNode($1,$3);} ;
	| expr9 AT expr10 {$$ = ActivNode($2, $3, $1);};

expr10	: expr11 ;
	| AT expr10 {$$ = ActivNode($1, $2, EmptyNode);} ;
	| NOT expr10 {$$ = NotNode($2);} ;
	| BAR expr10 {$$ = BarNode($2);} ;
	| CONCAT expr10 {$$ = BarNode($2);} ;
	| LCONCAT expr10 {$$ = BarNode($2);} ;
	| op10 expr10 {$$ = UnopNode($1,$2);} ;

op10	: DOT ;
	| BANG ;
	| DIFF ;
	| PLUS ;
	| STAR ;
	| SLASH ;
	| CARET ;
	| INTER ;
	| TILDE ;
	| MINUS ;
	| NUMEQ ;
	| NUMNE ;
	| LEXEQ ;
	| LEXNE ;
	| EQUIV ;
	| UNION ;
	| QMARK ;
	| NOTEQUIV ;
	| BACKSLASH ;

expr11	: literal ;
	| section ;
	| return ;
	| if ;
	| case ;
	| while ;
	| until ;
	| every ;
	| repeat ;
	| CREATE expr {$$ = CreateNode($1,$2);} ;
	| IDENT {Val0($1) = putloc(Str0($1),0);} ;
	| NEXT {$$ = NextNode($1);} ;
	| BREAK nexpr {$$ = BreakNode($1,$2);} ;
	| LPAREN {
		pushcs(0);
		} exprlist {
		popcs();
		} RPAREN {
			if (($3)->n_type == N_Elist)
			$$ = InvokNode($1,EmptyNode,$3);
			else
			$$ = $3; } ;
	| LBRACE compound RBRACE {$$ = $2;} ;
	| LBRACK {
		pushcs(0);
		} exprlist {
		popcs();
		} RBRACK {
		$$ = ListNode($1,$3);} ;
	| expr11 LBRACK nexpr RBRACK {$$ = BinopNode($2,$1,$3);} ;
	| expr11 LBRACE RBRACE {$$ = InvokNode($2,$1,ListNode($2,EmptyNode));} ;
	| expr11 LBRACE {
		pushcs(1);
		} exprlist {
		popcs();
		} RBRACE {
		$$ = InvokNode($2,$1,ListNode($2,$4));
		} ;
	| expr11 LPAREN {
		pushcs(0);
		} exprlist {
		popcs();
		} RPAREN {
		$$ = InvokNode($2,$1,$4);
		} ;
	| expr11 DOT IDENT {$$ = FieldNode($2,$1,$3);} ;
	| CONJUNC FAIL {$$ = KeyNode($1, K_FAIL);} ;
	| CONJUNC IDENT {
		if ((i = klocate(Str0($2))) == NULL)
			err("invalid keyword",Str0($2));
		$$ = KeyNode($1, i);
		} ;

while	: WHILE expr {$$ = LoopNode($1,$2,EmptyNode);} ;
	| WHILE expr DO expr {$$ = LoopNode($1,$2,$4);} ;

until	: UNTIL expr {$$ = LoopNode($1,$2,EmptyNode);} ;
	| UNTIL expr DO expr {$$ = LoopNode($1,$2,$4);} ;

every	: EVERY expr {$$ = LoopNode($1,$2,EmptyNode);} ;
	| EVERY expr DO expr {$$ = LoopNode($1,$2,$4);} ;

repeat	: REPEAT expr {$$ = LoopNode($1,$2,EmptyNode);} ;

return	: FAIL {$$ = RetNode($1,EmptyNode);} ;
	| RETURN nexpr {$$ = RetNode($1,$2);} ;
	| SUSPEND nexpr {$$ = SuspNode($1,$2);} ;

if	: IF expr THEN expr {$$ = IfNode($1,$2,$4,EmptyNode);} ;
	| IF expr THEN expr ELSE expr {$$ = IfNode($1,$2,$4,$6);} ;

case	: CASE expr OF LBRACE caselist RBRACE {$$ = CaseNode($1,$2,$5);} ;

caselist: cclause ;
	| caselist SEMICOL cclause {$$ = ClistNode($2,$1,$3);} ;

cclause	: DEFAULT COLON expr {$$ = CclsNode($2,$1,$3);} ;
	| expr COLON expr {$$ = CclsNode($2,$1,$3);} ;

exprlist: nexpr {
		$$ = cswitch($1,$1);
		} ;
	| exprlist COMMA nexpr {
		$$ = ElistNode($2,$1,cswitch($3,$2));
		} ;

literal	: INTLIT {Val0($1) = putlit(Str0($1),F_IntLit,0);} ;
	| REALLIT {Val0($1) = putlit(Str0($1),F_RealLit,0);} ;
	| STRINGLIT {Val0($1) = putlit(Str0($1),F_StrLit,Val1($1));} ;
	| CSETLIT {Val0($1) = putlit(Str0($1),F_CsetLit,Val1($1));} ;

section	: expr11 LBRACK expr sectop expr RBRACK {$$ = (nodeptr)SectNode($4,$1,$3,$5);} ;

sectop	: COLON ;
	| PCOLON ;
	| MCOLON ;

compound: nexpr ;
	| nexpr SEMICOL compound {$$ = SListNode($2, $1, $3);} ;

program	: error decls EOFX ;
proc	: prochead error procbody END ;
expr	: error ;
%%

pushcs(val)
int val;
{
	stacktop = stacktop + 1;
	cstack[stacktop] = val;
}

popcs()
{
	stacktop = stacktop - 1;
}

nodeptr cswitch(x,y)
	nodeptr x, y;
	{
	if (cstack[stacktop]) return(CreateNode(y,x));
	else return(x);
}
