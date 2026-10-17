#
#include "e.h"
#
int	fromflg; /* removed by stan */
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
# define LPILE 269
# define PILE 270
# define CPILE 271
# define RPILE 272
# define ABOVE 273
# define DEFINE 274
# define TDEFINE 275
# define NDEFINE 276
# define DELIM 277
# define GSIZE 278
# define GFONT 279
# define INCLUDE 280
# define FROM 281
# define TO 282
# define OVER 283
# define SQRT 284
# define SUP 285
# define SUB 286
# define SIZE 287
# define FONT 288
# define ROMAN 289
# define ITALIC 290
# define BOLD 291
# define FAT 292
# define UP 293
# define DOWN 294
# define BACK 295
# define FWD 296
# define LEFT 297
# define RIGHT 298
# define DOT 299
# define DOTDOT 300
# define HAT 301
# define TILDE 302
# define BAR 303
# define UNDER 304
# define VEC 305
# define DYAD 306
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar, yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#ifndef YYSTYPE
#define YYSTYPE int
#endif
#ifndef YYVCOPY
#define YYVCOPY(x,y) x=y
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

int yyexca[] {
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 86
#
extern int yychar;

int yylast 534;
yyact[]{

  32,  54,  55,  46, 141,  53,  52, 140, 139, 138,
 122, 121, 120,  53,  52, 118,  80,  81, 124,  61,
  63,  56,  62,  59,  60,  57,  58,  61,  63,  56,
  62,  59,  60,  57,  58, 109, 110, 111, 112, 130,
 129, 128,  32, 127, 113,  61,  63,  56,  62,  59,
  60,  57,  58,  78, 126,  72,  71,  70,  69,  79,
  48,  20,  84,  97,   2,  19,  18,  17,  51,  49,
  96,  16, 101,  15,  47,  14,  13,  12,  11,  73,
 104,   8,   7, 116,  32, 114,  82, 108, 107, 106,
  50,  83, 105, 103,  21,  44,  85,   1,  86,  87,
  88,  89,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  32,   0,   0,   0,   0,
   0,   0,   0,   0,  34,  33,  35,  36,  37,  42,
   0,  98,  99, 100,   6,  45,  27,  30,  28,  29,
  80,  81, 119, 115,   0, 119, 119, 119, 119, 119,
 119,  10, 123, 119,  22,  26,  23,  24,  25,   9,
  39,  41,  40,  38,  31, 102,  34,  33,  35,  36,
  37,  42,  32, 133, 125, 117,   6,  45,  27,  30,
  28,  29, 109, 110, 111, 112,  80,  81, 134, 135,
 136, 137,   0,  10,   0,   0,  22,  26,  23,  24,
  25,   9,  39,  41,  40,  38,  31,   3,  34,  33,
  35,  36,  37,  42,  32,   0,   0,   0,   6,   5,
  27,  30,  28,  29,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  10,   0,   0,  22,  26,
  23,  24,  25,   9,  39,  41,  40,  38,  31,  34,
  33,  35,  36,  37,  42,   0,   0,   0,   0,   6,
  45,  27,  30,  28,  29,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  10,   0,   0,  22,
  26,  23,  24,  25,   9,  39,  41,  40,  38,  31,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  34,  33,  35,  36,
  37,  42,   0,   0,   0,   0,   6,   5,  27,  30,
  28,  29,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,  10,   0,   0,  22,  26,  23,  24,
  25,   9,  39,  41,  40,  38,  31,   0,  34,  33,
  35,  36,  37,  42,   0,   0,   0,   0,   6,   0,
  27,  30,  28,  29,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  10,   0,   0,  22,  26,
  23,  24,  25,   9,  39,  41,  40,  38,  31,   4,
   0,   0,  43,   0,   0,   0,  64,  65,  66,  67,
  68,   0,   0,   0,   0,   0,   0,  74,  75,  76,
  77,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  90,  91,  92,  93,  94,
  95,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,  43,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  43,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,  43,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0, 131,   0, 132,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,  43 };
yypact[]{

 -39,-1000,   2,-1000,-280,-1000, 101, 101, 101, 101,
 101, -65, -66, -67, -68,  59,-1000, 101, 101, 101,
 101, -70,-241,-1000,-1000,-1000,-241,-1000,-1000,-1000,
-1000, -61,  59,-1000,-1000,-1000,-1000,-1000,-241,-241,
-241,-241,-1000,-280,-1000, 101, 101, 101, 101, 101,
 101,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-280,-254,-254,-254,-272,  59,
  59,  59,  59,-123,-254,-254,-254,-254,-228,-1000,
-1000,-1000,-1000,-1000,-1000, -81,-1000,-1000,-1000,-1000,
-280,-272,-272,-272,-280,-280,-110,   2,-113,-114,
-115,-1000,-107, -71,-1000, -80, -82, -83, -84,-1000,
-1000,-1000,-1000,-1000,-1000, 101,-1000, 101,-1000,  59,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,  59,  59,  59,
  59,-272,-280,   2,-116,-117,-118,-121,-1000,-1000,
-1000,-1000 };
yypgo[]{

   0,  97,  63, 399,  95,  94,  93,  80,  92,  70,
  89,  88,  87,  85,  60,  83,  90,  82,  81,  78,
  77,  76,  75,  74,  73,  72,  71,  69,  68,  67,
  66,  65,  61,  59 };
yyr1[]{

   0,   1,   1,   1,   2,   2,   2,   2,   4,   5,
   6,   6,   7,   7,   7,   7,   8,  10,  11,  12,
  13,  15,  15,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,  29,  30,  31,  32,
  28,  28,  28,  28,  28,  28,  28,  28,  27,  16,
  24,  24,  25,  25,  25,   9,   9,  19,  20,  22,
  21,  17,  18,  18,  18,  18,  23,  14,  26,  26,
  26,  26,  26,  26,  33,  33 };
yyr2[]{

   0,   1,   1,   0,   1,   2,   2,   1,   2,   1,
   1,   2,   4,   4,   4,   4,   1,   1,   1,   1,
   2,   2,   0,   3,   2,   2,   2,   2,   2,   4,
   4,   4,   4,   4,   3,   3,   3,   1,   4,   3,
   2,   2,   2,   2,   2,   4,   2,   2,   2,   2,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   2,   2,   2,   2,   0,   1,   3,   1,   1,   1,
   1,   2,   1,   1,   1,   2,   1,   1,   3,   1,
   1,   1,   1,   1,   1,   1 };
yychk[]{

-1000,  -1,  -2, 256,  -3, 268, 267, -17, -18, 292,
 284, -19, -20, -21, -22, -24, -26, -29, -30, -31,
 -32,  -5, 287, 289, 290, 291, 288, 269, 271, 272,
 270, 297, 123, 258, 257, 259, 260, 261, 296, 293,
 295, 294, 262,  -3,  -4, 268, 283, -23, -14, -27,
 -16, -28, 286, 285, 281, 282, 301, 305, 306, 303,
 304, 299, 302, 300,  -3,  -3,  -3,  -3,  -3, 123,
 123, 123, 123,  -2,  -3,  -3,  -3,  -3, 123, -33,
 257, 258, -33, -33, 123,  -2, -33, -33, -33, -33,
  -3,  -3,  -3,  -3,  -3,  -3,  -9,  -2,  -9,  -9,
  -9, -25, 298,  -6,  -7,  -8, -10, -11, -12, 263,
 264, 265, 266, 125, -13, -14, -15, -16, 125, 273,
 125, 125, 125, -33, 125,  -7, 125, 123, 123, 123,
 123,  -3,  -3,  -2,  -9,  -9,  -9,  -9, 125, 125,
 125, 125 };
yydef[]{

   3,  -2,   1,   2,   4,   7,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  37,   0,   0,   0,
   0,   0,   0,  72,  73,  74,   0,  67,  68,  70,
  69,   0,   0,  79,  80,  81,  82,  83,   0,   0,
   0,   0,   9,   5,   6,   0,   0,   0,   0,   0,
   0,  40,  76,  77,  58,  59,  50,  51,  52,  53,
  54,  55,  56,  57,  24,  25,  26,  27,  28,   0,
   0,   0,   0,  64,  41,  42,  43,  44,   0,  71,
  84,  85,  75,  60,  61,   0,  46,  47,  48,  49,
   8,  23,  34,  35,  22,  39,   0,  65,   0,   0,
   0,  36,   0,   0,  10,   0,   0,   0,   0,  16,
  17,  18,  19,  78,  33,   0,  38,   0,  29,   0,
  30,  31,  32,  62,  63,  11,  45,   0,   0,   0,
   0,  20,  21,  66,   0,   0,   0,   0,  12,  13,
  14,  15 };
#
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

int yydebug 0; /* 1 for debugging */
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar -1; /* current input token number */
int yynerrs 0;  /* number of errors */
int yyerrflag 0;  /* error recovery flag */

yyparse() {

	int yys[YYMAXDEPTH];
	int yyj, yym;
	register YYSTYPE *yypvt;
	register yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

 yystack:    /* put a state and value onto the stack */

	if( yydebug  ) printf( "state %d, value %d, char %d\n",yystate,yyval,yychar );
		if( ++yyps> &yys[YYMAXDEPTH] ) yyerror( "yacc stack overflow" );
		*yyps = yystate;
		++yypv;
		YYVCOPY(*yypv,yyval);

 yynewstate:

	yyn = yypact[yystate];

	if( yyn<= YYFLAG ) goto yydefault; /* simple state */

	if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
	if( (yyn =+ yychar)<0 || yyn >= yylast ) goto yydefault;

	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		YYVCOPY(yyval,yylval);
		yystate = yyn;
		if( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
		}

 yydefault:
	/* default state action */

	if( (yyn=yydef[yystate]) == -2 ) {
		if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
		/* look through exception table */

		for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi =+ 2 ) ; /* VOID */

		while( *(yyxi=+2) >= 0 ){
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
			   if( yyn>= 0 && yyn < yylast && yychk[yyact[yyn]] == YYERRCODE ){
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
		yyps =- yyr2[yyn];
		yypvt = yypv;
		yypv =- yyr2[yyn];
		YYVCOPY(yyval,yypv[1]);
		yym=yyn;
			/* consult goto table to find next state */
		yyn = yyr1[yyn];
		yyj = yypgo[yyn] + *yyps + 1;
		if( yyj>=yylast || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
		switch(yym){
			
case 1:{ putout(yypvt[-0]); } break;
case 2:{ error(!FATAL, "syntax error"); } break;
case 3:{ eqnreg = 0; } break;
case 5:{ eqnbox(yypvt[-1], yypvt[-0], 0); } break;
case 6:{ eqnbox(yypvt[-1], yypvt[-0], 1); } break;
case 7:{ lineup(0); } break;
case 8:{ yyval = yypvt[-0]; lineup(1); } break;
case 9:{ yyval = ct; } break;
case 12:{ column('L', yypvt[-3]); } break;
case 13:{ column('C', yypvt[-3]); } break;
case 14:{ column('R', yypvt[-3]); } break;
case 15:{ column('-', yypvt[-3]); } break;
case 16:{ yyval = ct++; } break;
case 17:{ yyval = ct++; } break;
case 18:{ yyval = ct++; } break;
case 19:{ yyval = ct++; } break;
case 20:{ yyval = yypvt[-0]; } break;
case 21:{ yyval = yypvt[-0]; } break;
case 22:{ yyval = 0; } break;
case 23:{ boverb(yypvt[-2], yypvt[-0]); } break;
case 24:{ mark(yypvt[-0]); } break;
case 25:{ size(yypvt[-1], yypvt[-0]); } break;
case 26:{ font(yypvt[-1], yypvt[-0]); } break;
case 27:{ fatbox(yypvt[-0]); } break;
case 28:{ sqrt(yypvt[-0]); } break;
case 29:{ lpile('L', yypvt[-3], ct); ct = yypvt[-3]; } break;
case 30:{ lpile('C', yypvt[-3], ct); ct = yypvt[-3]; } break;
case 31:{ lpile('R', yypvt[-3], ct); ct = yypvt[-3]; } break;
case 32:{ lpile('-', yypvt[-3], ct); ct = yypvt[-3]; } break;
case 33:{ shift2(yypvt[-3], yypvt[-1], yypvt[-0]); } break;
case 34:{ bshiftb(yypvt[-2], yypvt[-1], yypvt[-0]); } break;
case 35:{ bshiftb(yypvt[-2], yypvt[-1], yypvt[-0]); } break;
case 36:{ paren(yypvt[-2], yypvt[-1], yypvt[-0]); } break;
case 38:{ fromto(yypvt[-3], yypvt[-1], yypvt[-0]); fromflg=0; } break;
case 39:{ fromto(yypvt[-2], 0, yypvt[-0]); } break;
case 40:{ diacrit(yypvt[-1], yypvt[-0]); } break;
case 41:{ move(FWD, yypvt[-1], yypvt[-0]); } break;
case 42:{ move(UP, yypvt[-1], yypvt[-0]); } break;
case 43:{ move(BACK, yypvt[-1], yypvt[-0]); } break;
case 44:{ move(DOWN, yypvt[-1], yypvt[-0]); } break;
case 45:{ matrix(yypvt[-3]); } break;
case 46:{ yyval = atoi((char *) yypvt[-1]); } break;
case 47:{ yyval = atoi((char *) yypvt[-1]); } break;
case 48:{ yyval = atoi((char *) yypvt[-1]); } break;
case 49:{ yyval = atoi((char *) yypvt[-1]); } break;
case 50:{ yyval = HAT; } break;
case 51:{ yyval = VEC; } break;
case 52:{ yyval = DYAD; } break;
case 53:{ yyval = BAR; } break;
case 54:{ yyval = UNDER; } break;
case 55:{ yyval = DOT; } break;
case 56:{ yyval = TILDE; } break;
case 57:{ yyval = DOTDOT; } break;
case 58:{ yyval=ps; ps -= 3; fromflg = 1;
		if(dbg)printf(".\tfrom: old ps %d, new ps %d, fflg %d\n", yyval, ps, fromflg);
		} break;
case 59:{ yyval=ps; if(fromflg==0)ps -= 3; 
			if(dbg)printf(".\tto: old ps %d, new ps %d\n", yyval, ps);
		} break;
case 60:{ yyval = ((char *)yypvt[-0])[0]; } break;
case 61:{ yyval = '{'; } break;
case 62:{ yyval = ((char *)yypvt[-0])[0]; } break;
case 63:{ yyval = '}'; } break;
case 64:{ yyval = 0; } break;
case 65:{ lp[ct++] = yypvt[-0]; } break;
case 66:{ lp[ct++] = yypvt[-0]; } break;
case 67:{ yyval = ct; } break;
case 68:{ yyval = ct; } break;
case 69:{ yyval = ct; } break;
case 70:{ yyval = ct; } break;
case 71:{ yyval = ps; setsize((char *) yypvt[-0]); } break;
case 72:{ setfont(ROM); } break;
case 73:{ setfont(ITAL); } break;
case 74:{ setfont(BLD); } break;
case 75:{ setfont(((char *)yypvt[-0])[0]); } break;
case 76:{ shift(SUB); } break;
case 77:{ shift(SUP); } break;
case 78:{ yyval = yypvt[-1]; } break;
case 79:{ text(QTEXT, (char *) yypvt[-0]); } break;
case 80:{ text(CONTIG, (char *) yypvt[-0]); } break;
case 81:{ text('~', (char *) yypvt[-0]); } break;
case 82:{ text(THIN, 0); } break;
case 83:{ text(TAB, 0); } break;
		}
		goto yystack;  /* stack new state and value */

	}
