
# line 1 "e.y"
#
#include "e.h"
#
int	fromflg;
# define CONTIG 257
# define QTEXT 258
# define SPACE 259
# define THIN 260
# define TAB 261
# define MATRIX 262
# define LCOL 263
# define CCOL 264
# define RCOL 265
# define COL 266
# define MARK 267
# define LINEUP 268
# define SUM 269
# define INT 270
# define PROD 271
# define UNION 272
# define INTER 273
# define LPILE 274
# define PILE 275
# define CPILE 276
# define RPILE 277
# define ABOVE 278
# define DEFINE 279
# define TDEFINE 280
# define NDEFINE 281
# define DELIM 282
# define GSIZE 283
# define GFONT 284
# define INCLUDE 285
# define FROM 286
# define TO 287
# define OVER 288
# define SQRT 289
# define SUP 290
# define SUB 291
# define SIZE 292
# define FONT 293
# define ROMAN 294
# define ITALIC 295
# define BOLD 296
# define FAT 297
# define UP 298
# define DOWN 299
# define BACK 300
# define FWD 301
# define LEFT 302
# define RIGHT 303
# define DOT 304
# define DOTDOT 305
# define HAT 306
# define TILDE 307
# define BAR 308
# define UNDER 309
# define VEC 310
# define DYAD 311
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 166 "e.y"

short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 95
# define YYLAST 560
short yyact[]={

  34,  60,  61,  52, 152,  59,  58,  59,  58, 151,
 150, 149, 132,  59,  58, 131,  88,  89, 141,  67,
  69,  62,  68,  65,  66,  63,  64,  67,  69,  62,
  68,  65,  66,  63,  64,  67,  69,  62,  68,  65,
  66,  63,  64, 119, 120, 121, 122,  34,  53, 123,
 130, 128, 140, 139,  87, 138, 135,  86,  78,  77,
 105,   2,  76,  75,  79,  21,  20, 137,  19,  18,
  57,  55,  17, 111,  16,  92,  15,  81,  14,  13,
  12,  11,  90,   8,   7, 126, 118, 117,  91, 116,
 115, 113, 124,  22,  34,  93,  50, 114,   1,  94,
  95,  96,  97,   0,   0,   0,   0,   0,   0,  56,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,  36,  35,  37,  38,  39,  48,
  34,   0,   0,   0,   6,  51,  40,  32,  41,  42,
  43,  28,  31,  29,  30,   0,   0, 129, 104,   0,
   0,   0, 129, 129, 129, 129,  10, 134, 129,  23,
  27,  24,  25,  26,   9,  45,  47,  46,  44,  33,
 112,  36,  35,  37,  38,  39,  48,  34,  88,  89,
 144,   6,  51,  40,  32,  41,  42,  43,  28,  31,
  29,  30, 133, 129, 129, 119, 120, 121, 122,  88,
  89, 136, 127,  10,   0,   0,  23,  27,  24,  25,
  26,   9,  45,  47,  46,  44,  33,   3,  36,  35,
  37,  38,  39,  48,  34, 106, 107, 108,   6,   5,
  40,  32,  41,  42,  43,  28,  31,  29,  30,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  10,   0,   0,  23,  27,  24,  25,  26,   9,  45,
  47,  46,  44,  33,  36,  35,  37,  38,  39,  48,
   0,   0,   0,   0,   6,  51,  40,  32,  41,  42,
  43,  28,  31,  29,  30,   0,   0, 145, 146, 147,
 148,   0,   0,   0,   0,   0,  10,   0,   0,  23,
  27,  24,  25,  26,   9,  45,  47,  46,  44,  33,
   0,  36,  35,  37,  38,  39,  48,  54,   0,   0,
   0,   6,   5,  40,  32,  41,  42,  43,  28,  31,
  29,  30,   0,  80,   0,   0,   0,   0,   0,   0,
   0,   0,   0,  10,   0,   0,  23,  27,  24,  25,
  26,   9,  45,  47,  46,  44,  33,   0,  36,  35,
  37,  38,  39,  48,   0,   0,   0,   0,   6,   0,
  40,  32,  41,  42,  43,  28,  31,  29,  30,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  10,   0,   0,  23,  27,  24,  25,  26,   9,  45,
  47,  46,  44,  33,   4,   0,   0,  49,   0,   0,
   0,  70,  71,  72,  73,  74,   0,   0, 125,   0,
   0,   0,   0,  82,  83,  84,  85, 125,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  98,  99, 100, 101,
 102, 103,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0, 109, 110,  49,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,  49,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  49,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 142,   0, 143,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,  49 };
short yypact[]={

 -29,-1000,  17,-1000,-285,-1000, 111, 111, 111, 111,
 111, -60, -61, -64, -65,-283,  64,-1000, 111, 111,
 111, 111, -66,-241,-1000,-1000,-1000,-241,-1000,-1000,
-1000,-1000,-1000, -48,  64,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-241,-241,-241,-241,-1000,-285,
-1000, 111, 111, 111, 111, 111, 111,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-285,-269,-269,-269,-277,  64,  64,  64,  64, 111,
 111,-123,-269,-269,-269,-269,-220,-1000,-1000,-1000,
-1000,-1000,-1000, -76,-1000,-1000,-1000,-1000,-285,-277,
-277,-277,-285,-285, -74,  17, -75,-110,-113,-277,
-277,-1000, -69, -58,-1000, -68, -70, -71,-105,-1000,
-1000,-1000,-1000,-1000,-1000, 111,-1000, 111,-1000,  64,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,  64,  64,
  64,  64,-277,-285,  17,-114,-115,-116,-121,-1000,
-1000,-1000,-1000 };
short yypgo[]={

   0,  98,  60, 414,  96,  93,  91,  97,  90, 158,
  89,  87,  86,  92, 327,  85, 109,  84,  83,  81,
  80,  79,  78,  48,  76,  74,  73,  72,  71,  70,
  69,  68,  66,  65,  54 };
short yyr1[]={

   0,   1,   1,   1,   2,   2,   2,   2,   4,   5,
   6,   6,   7,   7,   7,   7,   8,  10,  11,  12,
  13,  15,  15,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
  24,  30,  31,  32,  33,  29,  29,  29,  29,  29,
  29,  29,  29,  28,  16,  25,  25,  26,  26,  26,
   9,   9,  19,  20,  22,  21,  17,  18,  18,  18,
  18,  23,  14,  27,  27,  27,  27,  27,  27,  27,
  27,  27,  27,  34,  34 };
short yyr2[]={

   0,   1,   1,   0,   1,   2,   2,   1,   2,   1,
   1,   2,   4,   4,   4,   4,   1,   1,   1,   1,
   2,   2,   0,   3,   2,   2,   2,   2,   2,   4,
   4,   4,   4,   4,   3,   3,   4,   3,   3,   1,
   3,   1,   4,   3,   2,   2,   2,   2,   2,   4,
   1,   2,   2,   2,   2,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   2,   2,   2,   2,   0,
   1,   3,   1,   1,   1,   1,   2,   1,   1,   1,
   2,   1,   1,   3,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1 };
short yychk[]={

-1000,  -1,  -2, 256,  -3, 268, 267, -17, -18, 297,
 289, -19, -20, -21, -22, -24, -25, -27, -30, -31,
 -32, -33,  -5, 292, 294, 295, 296, 293, 274, 276,
 277, 275, 270, 302, 123, 258, 257, 259, 260, 261,
 269, 271, 272, 273, 301, 298, 300, 299, 262,  -3,
  -4, 268, 288, -23, -14, -28, -16, -29, 291, 290,
 286, 287, 306, 310, 311, 308, 309, 304, 307, 305,
  -3,  -3,  -3,  -3,  -3, 123, 123, 123, 123, -23,
 -14,  -2,  -3,  -3,  -3,  -3, 123, -34, 257, 258,
 -34, -34, 123,  -2, -34, -34, -34, -34,  -3,  -3,
  -3,  -3,  -3,  -3,  -9,  -2,  -9,  -9,  -9,  -3,
  -3, -26, 303,  -6,  -7,  -8, -10, -11, -12, 263,
 264, 265, 266, 125, -13, -14, -15, -16, 125, 278,
 125, 125, 125, -13, -34, 125,  -7, 125, 123, 123,
 123, 123,  -3,  -3,  -2,  -9,  -9,  -9,  -9, 125,
 125, 125, 125 };
short yydef[]={

   3,  -2,   1,   2,   4,   7,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  39,   0,  41,   0,   0,
   0,   0,   0,   0,  77,  78,  79,   0,  72,  73,
  75,  74,  50,   0,   0,  84,  85,  86,  87,  88,
  89,  90,  91,  92,   0,   0,   0,   0,   9,   5,
   6,   0,   0,   0,   0,   0,   0,  44,  81,  82,
  63,  64,  55,  56,  57,  58,  59,  60,  61,  62,
  24,  25,  26,  27,  28,   0,   0,   0,   0,   0,
   0,  69,  45,  46,  47,  48,   0,  76,  93,  94,
  80,  65,  66,   0,  51,  52,  53,  54,   8,  23,
  34,  35,  22,  43,   0,  70,   0,   0,   0,  37,
  38,  40,   0,   0,  10,   0,   0,   0,   0,  16,
  17,  18,  19,  83,  33,   0,  42,   0,  29,   0,
  30,  31,  32,  36,  67,  68,  11,  49,   0,   0,
   0,   0,  20,  21,  71,   0,   0,   0,   0,  12,
  13,  14,  15 };
#
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

int yydebug = 0; /* 1 for debugging */
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse() {

	short yys[YYMAXDEPTH];
	short yyj, yym;
	register YYSTYPE *yypvt;
	register short yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

 yystack:    /* put a state and value onto the stack */

	if( yydebug  ) printf( "state %d, char 0%o\n", yystate, yychar );
		if( ++yyps> &yys[YYMAXDEPTH] ) { yyerror( "yacc stack overflow" ); return(1); }
		*yyps = yystate;
		++yypv;
		*yypv = yyval;

 yynewstate:

	yyn = yypact[yystate];

	if( yyn<= YYFLAG ) goto yydefault; /* simple state */

	if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
	if( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
		}

 yydefault:
	/* default state action */

	if( (yyn=yydef[yystate]) == -2 ) {
		if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
		/* look through exception table */

		for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

		while( *(yyxi+=2) >= 0 ){
			if( *yyxi == yychar ) break;
			}
		if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
		}

	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */

		switch( yyerrflag ){

		case 0:   /* brand new error */

			yyerror( "syntax error" );
		yyerrlab:
			++yynerrs;

		case 1:
		case 2: /* incompletely recovered error ... try again */

			yyerrflag = 3;

			/* find a state where "error" is a legal shift action */

			while ( yyps >= yys ) {
			   yyn = yypact[*yyps] + YYERRCODE;
			   if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
			      yystate = yyact[yyn];  /* simulate a shift of "error" */
			      goto yystack;
			      }
			   yyn = yypact[*yyps];

			   /* the current yyps has no shift onn "error", pop stack */

			   if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
			   --yyps;
			   --yypv;
			   }

			/* there is no state on the stack with an error shift ... abort */

	yyabort:
			return(1);


		case 3:  /* no shift yet; clobber input char */

			if( yydebug ) printf( "error recovery discards char %d\n", yychar );

			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

		if( yydebug ) printf("reduce %d\n",yyn);
		yyps -= yyr2[yyn];
		yypvt = yypv;
		yypv -= yyr2[yyn];
		yyval = yypv[1];
		yym=yyn;
			/* consult goto table to find next state */
		yyn = yyr1[yyn];
		yyj = yypgo[yyn] + *yyps + 1;
		if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
		switch(yym){
			
case 1:
# line 22 "e.y"
{ putout(yypvt[-0]); } break;
case 2:
# line 23 "e.y"
{ error(!FATAL, "syntax error"); } break;
case 3:
# line 24 "e.y"
{ eqnreg = 0; } break;
case 5:
# line 28 "e.y"
{ eqnbox(yypvt[-1], yypvt[-0], 0); } break;
case 6:
# line 29 "e.y"
{ eqnbox(yypvt[-1], yypvt[-0], 1); } break;
case 7:
# line 30 "e.y"
{ lineup(0); } break;
case 8:
# line 33 "e.y"
{ yyval = yypvt[-0]; lineup(1); } break;
case 9:
# line 36 "e.y"
{ yyval = ct; } break;
case 12:
# line 42 "e.y"
{ column('L', yypvt[-3]); } break;
case 13:
# line 43 "e.y"
{ column('C', yypvt[-3]); } break;
case 14:
# line 44 "e.y"
{ column('R', yypvt[-3]); } break;
case 15:
# line 45 "e.y"
{ column('-', yypvt[-3]); } break;
case 16:
# line 48 "e.y"
{ yyval = ct++; } break;
case 17:
# line 49 "e.y"
{ yyval = ct++; } break;
case 18:
# line 50 "e.y"
{ yyval = ct++; } break;
case 19:
# line 51 "e.y"
{ yyval = ct++; } break;
case 20:
# line 53 "e.y"
{ yyval = yypvt[-0]; } break;
case 21:
# line 56 "e.y"
{ yyval = yypvt[-0]; } break;
case 22:
# line 57 "e.y"
{ yyval = 0; } break;
case 23:
# line 60 "e.y"
{ boverb(yypvt[-2], yypvt[-0]); } break;
case 24:
# line 61 "e.y"
{ mark(yypvt[-0]); } break;
case 25:
# line 62 "e.y"
{ size(yypvt[-1], yypvt[-0]); } break;
case 26:
# line 63 "e.y"
{ font(yypvt[-1], yypvt[-0]); } break;
case 27:
# line 64 "e.y"
{ fatbox(yypvt[-0]); } break;
case 28:
# line 65 "e.y"
{ sqrt(yypvt[-0]); } break;
case 29:
# line 66 "e.y"
{ lpile('L', yypvt[-3], ct); ct = yypvt[-3]; } break;
case 30:
# line 67 "e.y"
{ lpile('C', yypvt[-3], ct); ct = yypvt[-3]; } break;
case 31:
# line 68 "e.y"
{ lpile('R', yypvt[-3], ct); ct = yypvt[-3]; } break;
case 32:
# line 69 "e.y"
{ lpile('-', yypvt[-3], ct); ct = yypvt[-3]; } break;
case 33:
# line 70 "e.y"
{ shift2(yypvt[-3], yypvt[-1], yypvt[-0]); } break;
case 34:
# line 71 "e.y"
{ bshiftb(yypvt[-2], yypvt[-1], yypvt[-0]); } break;
case 35:
# line 72 "e.y"
{ bshiftb(yypvt[-2], yypvt[-1], yypvt[-0]); } break;
case 36:
# line 73 "e.y"
{ integral(yypvt[-3], yypvt[-1], yypvt[-0]); } break;
case 37:
# line 74 "e.y"
{ integral(yypvt[-2], yypvt[-0], 0); } break;
case 38:
# line 75 "e.y"
{ integral(yypvt[-2], 0, yypvt[-0]); } break;
case 39:
# line 76 "e.y"
{ integral(yypvt[-0], 0, 0); } break;
case 40:
# line 77 "e.y"
{ paren(yypvt[-2], yypvt[-1], yypvt[-0]); } break;
case 42:
# line 79 "e.y"
{ fromto(yypvt[-3], yypvt[-1], yypvt[-0]); fromflg=0; } break;
case 43:
# line 80 "e.y"
{ fromto(yypvt[-2], 0, yypvt[-0]); } break;
case 44:
# line 81 "e.y"
{ diacrit(yypvt[-1], yypvt[-0]); } break;
case 45:
# line 82 "e.y"
{ move(FWD, yypvt[-1], yypvt[-0]); } break;
case 46:
# line 83 "e.y"
{ move(UP, yypvt[-1], yypvt[-0]); } break;
case 47:
# line 84 "e.y"
{ move(BACK, yypvt[-1], yypvt[-0]); } break;
case 48:
# line 85 "e.y"
{ move(DOWN, yypvt[-1], yypvt[-0]); } break;
case 49:
# line 86 "e.y"
{ matrix(yypvt[-3]); } break;
case 50:
# line 89 "e.y"
{ setintegral(); } break;
case 51:
# line 92 "e.y"
{ yyval = atoi((char *) yypvt[-1]); } break;
case 52:
# line 93 "e.y"
{ yyval = atoi((char *) yypvt[-1]); } break;
case 53:
# line 94 "e.y"
{ yyval = atoi((char *) yypvt[-1]); } break;
case 54:
# line 95 "e.y"
{ yyval = atoi((char *) yypvt[-1]); } break;
case 55:
# line 97 "e.y"
{ yyval = HAT; } break;
case 56:
# line 98 "e.y"
{ yyval = VEC; } break;
case 57:
# line 99 "e.y"
{ yyval = DYAD; } break;
case 58:
# line 100 "e.y"
{ yyval = BAR; } break;
case 59:
# line 101 "e.y"
{ yyval = UNDER; } break;
case 60:
# line 102 "e.y"
{ yyval = DOT; } break;
case 61:
# line 103 "e.y"
{ yyval = TILDE; } break;
case 62:
# line 104 "e.y"
{ yyval = DOTDOT; } break;
case 63:
# line 107 "e.y"
{ yyval=ps; ps -= 3; fromflg = 1;
		if(dbg)printf(".\tfrom: old ps %d, new ps %d, fflg %d\n", yyval, ps, fromflg);
		} break;
case 64:
# line 112 "e.y"
{ yyval=ps; if(fromflg==0)ps -= 3; 
			if(dbg)printf(".\tto: old ps %d, new ps %d\n", yyval, ps);
		} break;
case 65:
# line 117 "e.y"
{ yyval = ((char *)yypvt[-0])[0]; } break;
case 66:
# line 118 "e.y"
{ yyval = '{'; } break;
case 67:
# line 121 "e.y"
{ yyval = ((char *)yypvt[-0])[0]; } break;
case 68:
# line 122 "e.y"
{ yyval = '}'; } break;
case 69:
# line 123 "e.y"
{ yyval = 0; } break;
case 70:
# line 126 "e.y"
{ lp[ct++] = yypvt[-0]; } break;
case 71:
# line 127 "e.y"
{ lp[ct++] = yypvt[-0]; } break;
case 72:
# line 130 "e.y"
{ yyval = ct; } break;
case 73:
# line 131 "e.y"
{ yyval = ct; } break;
case 74:
# line 132 "e.y"
{ yyval = ct; } break;
case 75:
# line 133 "e.y"
{ yyval = ct; } break;
case 76:
# line 135 "e.y"
{ yyval = ps; setsize((char *) yypvt[-0]); } break;
case 77:
# line 138 "e.y"
{ setfont(ROM); } break;
case 78:
# line 139 "e.y"
{ setfont(ITAL); } break;
case 79:
# line 140 "e.y"
{ setfont(BLD); } break;
case 80:
# line 141 "e.y"
{ setfont(((char *)yypvt[-0])[0]); } break;
case 81:
# line 144 "e.y"
{ shift(SUB); } break;
case 82:
# line 147 "e.y"
{ shift(SUP); } break;
case 83:
# line 150 "e.y"
{ yyval = yypvt[-1]; } break;
case 84:
# line 151 "e.y"
{ text(QTEXT, (char *) yypvt[-0]); } break;
case 85:
# line 152 "e.y"
{ text(CONTIG, (char *) yypvt[-0]); } break;
case 86:
# line 153 "e.y"
{ text(SPACE, 0); } break;
case 87:
# line 154 "e.y"
{ text(THIN, 0); } break;
case 88:
# line 155 "e.y"
{ text(TAB, 0); } break;
case 89:
# line 156 "e.y"
{ funny(SUM); } break;
case 90:
# line 157 "e.y"
{ funny(PROD); } break;
case 91:
# line 158 "e.y"
{ funny(UNION); } break;
case 92:
# line 159 "e.y"
{ funny(INTER); } break;
		}
		goto yystack;  /* stack new state and value */

	}
