#include "itran.h"
#include "lex.h"
#include "token.h"

/*
 * Token table - contains an entry for each token type
 * with printable name of token, token type, and flags
 * for semicolon insertion.
 */

struct toktab toktab[] = {
/*  token		token type	flags */

   /* primitives */
   "identifier",      IDENT,         Beginner+Ender,    /*   0 */
   "integer-literal", INTLIT,        Beginner+Ender,    /*   1 */
   "real-literal",    REALLIT,       Beginner+Ender,    /*   2 */
   "string-literal",  STRINGLIT,     Beginner+Ender,    /*   3 */
   "cset-literal",    CSETLIT,       Beginner+Ender,    /*   4 */
   "end-of-file",     EOFX,          0,                 /*   5 */

   /* reserved words */
   "break",           BREAK,         Beginner+Ender,    /*   6 */
   "by",              BY,            0,                 /*   7 */
   "case",            CASE,          Beginner,          /*   8 */
   "create",          CREATE,        Beginner,          /*   9 */
   "default",         DEFAULT,       Beginner,          /*  10 */
   "do",              DO,            0,                 /*  11 */
   "dynamic",         DYNAMIC,       Beginner,          /*  12 */
   "else",            ELSE,          0,                 /*  13 */
   "end",             END,           Beginner,          /*  14 */
   "every",           EVERY,         Beginner,          /*  15 */
   "fail",            FAIL,          Beginner+Ender,    /*  16 */
   "global",          GLOBAL,        0,                 /*  17 */
   "if",              IF,            Beginner,          /*  18 */
   "initial",         INITIAL,       Beginner,          /*  19 */
   "link",            LINK,          0,                 /*  20 */
   "local",           LOCAL,         Beginner,          /*  21 */
   "next",            NEXT,          Beginner+Ender,    /*  22 */
   "not",             NOT,           Beginner,          /*  23 */
   "of",              OF,            0,                 /*  24 */
   "procedure",       PROCEDURE,     0,                 /*  25 */
   "record",          RECORD,        0,                 /*  26 */
   "repeat",          REPEAT,        Beginner,          /*  27 */
   "return",          RETURN,        Beginner+Ender,    /*  28 */
   "static",          STATIC,        Beginner,          /*  29 */
   "suspend",         SUSPEND,       Beginner+Ender,    /*  30 */
   "then",            THEN,          0,                 /*  31 */
   "to",              TO,            0,                 /*  32 */
   "until",           UNTIL,         Beginner,          /*  33 */
   "while",           WHILE,         Beginner,          /*  34 */

   /* operators */
   ":=",              ASSIGN,        0,                 /*  35 */
   "@",               AT,            Beginner,          /*  36 */
   "@:=",             AUGACT,        0,                 /*  37 */
   "&:=",             AUGAND,        0,                 /*  38 */
   "=:=",             AUGEQ,         0,                 /*  39 */
   "===:=",           AUGEQV,        0,                 /*  40 */
   ">=:=",            AUGGE,         0,                 /*  41 */
   ">:=",             AUGGT,         0,                 /*  42 */
   "<=:=",            AUGLE,         0,                 /*  43 */
   "<:=",             AUGLT,         0,                 /*  44 */
   "~=:=",            AUGNE,         0,                 /*  45 */
   "~===:=",          AUGNEQV,       0,                 /*  46 */
   "==:=",            AUGSEQ,        0,                 /*  47 */
   ">>=:=",           AUGSGE,        0,                 /*  48 */
   ">>:=",            AUGSGT,        0,                 /*  49 */
   "<<=:=",           AUGSLE,        0,                 /*  50 */
   "<<:=",            AUGSLT,        0,                 /*  51 */
   "~==:=",           AUGSNE,        0,                 /*  52 */
   "\\",              BACKSLASH,     Beginner,          /*  53 */
   "!",               BANG,          Beginner,          /*  54 */
   "|",               BAR,           Beginner,          /*  55 */
   "^",               CARET,         Beginner,          /*  56 */
   "^:=",             CARETASGN,     0,                 /*  57 */
   ":",               COLON,         0,                 /*  58 */
   ",",               COMMA,         0,                 /*  59 */
   "||",              CONCAT,        Beginner,          /*  60 */
   "||:=",            CONCATASGN,    0,                 /*  61 */
   "&",               CONJUNC,       Beginner,          /*  62 */
   ".",               DOT,           Beginner,          /*  63 */
   "--",              DIFF,          Beginner,          /*  64 */
   "--:=",            DIFFASGN,      0,                 /*  65 */
   "===",             EQUIV,         Beginner,          /*  66 */
   "**",              INTER,         Beginner,          /*  67 */
   "**:=",            INTERASGN,     0,                 /*  68 */
   "{",               LBRACE,        Beginner,          /*  69 */
   "[",               LBRACK,        Beginner,          /*  70 */
   "|||",             LCONCAT,       Beginner,          /*  71 */
   "|||:=",           LCONCATASGN,   Beginner,          /*  72 */
   "==",              LEXEQ,         Beginner,          /*  73 */
   ">>=",             LEXGE,         0,                 /*  74 */
   ">>",              LEXGT,         0,                 /*  75 */
   "<<=",             LEXLE,         0,                 /*  76 */
   "<<",              LEXLT,         0,                 /*  77 */
   "~==",             LEXNE,         Beginner,          /*  78 */
   "(",               LPAREN,        Beginner,          /*  79 */
   "-:",              MCOLON,        0,                 /*  80 */
   "-",               MINUS,         Beginner,          /*  81 */
   "-:=",             MINUSASGN,     0,                 /*  82 */
   "%",               MOD,           0,                 /*  83 */
   "%:=",             MODASGN,       0,                 /*  84 */
   "~===",            NOTEQUIV,      Beginner,          /*  85 */
   "=",               NUMEQ,         Beginner,          /*  86 */
   ">=",              NUMGE,         0,                 /*  87 */
   ">",               NUMGT,         0,                 /*  88 */
   "<=",              NUMLE,         0,                 /*  89 */
   "<",               NUMLT,         0,                 /*  90 */
   "~=",              NUMNE,         Beginner,          /*  91 */
   "+:",              PCOLON,        0,                 /*  92 */
   "+",               PLUS,          Beginner,          /*  93 */
   "+:=",             PLUSASGN,      0,                 /*  94 */
   "?",               QMARK,         Beginner,          /*  95 */
   "<-",              REVASSIGN,     0,                 /*  96 */
   "<->",             REVSWAP,       0,                 /*  97 */
   "}",               RBRACE,        Ender,             /*  98 */
   "]",               RBRACK,        Ender,             /*  99 */
   ")",               RPAREN,        Ender,             /* 100 */
   ";",               SEMICOL,       0,                 /* 101 */
   "?:=",             SCANASGN,      0,                 /* 102 */
   "/",               SLASH,         Beginner,          /* 103 */
   "/:=",             SLASHASGN,     0,                 /* 104 */
   "*",               STAR,          Beginner,          /* 105 */
   "*:=",             STARASGN,      0,                 /* 106 */
   ":=:",             SWAP,          0,                 /* 107 */
   "~",               TILDE,         Beginner,          /* 108 */
   "++",              UNION,         Beginner,          /* 109 */
   "++:=",            UNIONASGN,     0,                 /* 110 */
   "end-of-file",     0,             0,
   };

/*
 * restab[c] points to the first keyword in toktab which
 * begins with the letter c.
 */

struct toktab *restab[] = {
                             NULL       , NULL       , /*   _` */
   NULL,        &toktab[ 6], &toktab[ 8], &toktab[10], /* abcd */
   &toktab[13], &toktab[16], &toktab[17], NULL,        /* efgh */
   &toktab[18], NULL,        NULL,        &toktab[20], /* ijkl */
   NULL,        &toktab[22], &toktab[24], &toktab[25], /* mnop */
   NULL,        &toktab[26], &toktab[29], &toktab[31], /* qrst */
   &toktab[33], NULL,        &toktab[34], NULL,        /* uvwx */
   NULL,        NULL,                                  /* yz */
   };
