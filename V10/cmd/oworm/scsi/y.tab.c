
# line 4 "gram.y"

#include	"scsi.h"


# line 17 "gram.y"
typedef union  {
	char *str;
	int num;	/* 32 bits */
} YYSTYPE;
# define DISK 257
# define ID 258
# define RELEASE 259
# define HELP 260
# define SET 261
# define EJECT 262
# define INQUIRY 263
# define READ 264
# define CAPACITY 265
# define SENSE 266
# define NUMBER 267
# define EXT 268
# define SIDEA 269
# define SIDEB 270
# define ALL 271
# define RESET 272
# define SLEEP 273
# define START 274
# define STOP 275
# define STATUS 276
# define ECHO 277
# define ERROR 278
# define MEDIA 279
# define WORM 280
# define WRITE 281
# define DIAGNOSTIC 282
# define STRING 283
# define COPY 284
# define CONFIG 285
# define TEST 286
# define INTERNAL 287
# define ALTERNATE 288
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 100 "gram.y"


yywrap()
{
	return(1);
}

yyerror(s1, s2)
	char *s1, *s2;
{
	fprint(2, s1, s2);
	fprint(2, "\n");
}
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 6,
	};
# define YYNPROD 47
# define YYLAST 257
short yyact[]={

   7,  10,  81,   9,  73,  74,  11,  15,   4,  17,
  26,  14,  43,   2,  83,  16,  18,  19,  21,  20,
   8,  82,  13,  23,  24,  42,  45,   6,   5,  22,
  12,   3,  32,  77,  76,  32,  75,  61,  70,  69,
  68,  66,  63,  29,  56,  47,  40,  37,  34,  35,
  31,  36,  33,  72,  54,  60,   1,   0,  28,  27,
   0,  38,  39,  30,  41,   0,  44,   0,  46,   0,
  48,  49,  50,  51,  52,  53,   0,   0,   0,   0,
   0,   0,   0,   0,  57,  58,  59,   0,   0,   0,
   0,   0,   0,  64,  65,   0,  67,   0,   0,  62,
   0,   0,   0,  55,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  71,   0,   0,   0,   0,
   0,  78,   0,  80,   0,  79,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  25 };
short yypact[]={

-1000,-257,   0,-224,-224,-1000,-235,-210,-220,-1000,
-235,-235,-221,-235,-254,-232,-1000,-235,-222,-235,
-235,-235,-235,-235,-235,  44,-1000,-1000,-1000,-1000,
-1000,-223,-1000,-235,-235,-235,-230,-1000,-1000,-1000,
-224,-225,-235,-235,-226,-235,-1000,-1000,-1000,-1000,
-1000,-1000,-227,-228,-1000,-1000,-229,-1000,-1000,-230,
-265,-1000,-1000,-231,-233,-1000,-1000,-1000,-1000,-234,
-235,-265,-235,-1000,-1000,-281,-246,-1000,-253,-1000,
-1000,-1000,-1000,-1000 };
short yypgo[]={

   0,  56,  50,  53,  55,  58,  13 };
short yyr1[]={

   0,   1,   1,   1,   1,   1,   6,   6,   6,   6,
   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
   6,   2,   5,   5,   4,   3,   3 };
short yyr2[]={

   0,   0,   4,   3,   3,   4,   0,   2,   2,   1,
   6,   3,   3,   5,   3,   5,   2,   1,   2,   1,
   2,   1,   3,   4,   5,   5,   3,   3,   1,   2,
   3,   2,   2,   2,   1,   2,   2,   2,   3,   3,
   4,   1,   1,   0,   1,   1,   1 };
short yychk[]={

-1000,  -1,  -6, 288, 265, 285, 284, 257, 277, 260,
 258, 263, 287, 279, 268, 264, 272, 266, 273, 274,
 276, 275, 286, 280, 281, 256,  10,  59,  -5, 267,
  -5,  -2, 267, 262, 258, 259, 261, 267,  -2,  -2,
 267,  -2, 279, 266,  -2, 258,  -2, 267,  -2,  -2,
  -2,  -2,  -2,  -2,  10,  59, 267,  -2,  -2,  -2,
  -4, 267,  -5, 267,  -2,  -2, 267,  -2, 267, 267,
 267,  -4,  -3, 269, 270, 267, 267, 267,  -2,  -3,
  -2, 283, 267, 267 };
short yydef[]={

   1,  -2,   0,  43,  43,   9,   0,   0,   0,  17,
   0,  19,  21,   0,   0,   0,  28,   0,   0,   0,
  34,   0,   0,   0,   0,   0,   3,   4,   7,  42,
   8,   0,  41,   0,   0,   0,   0,  16,  18,  20,
  43,   0,   0,   0,   0,   0,  29,  31,  32,  33,
  35,  36,  37,   0,   2,   5,   0,  11,  12,  14,
   0,  44,  22,   0,   0,  30,  26,  27,  38,  39,
   0,   0,   0,  45,  46,  23,   0,  40,   0,  13,
  15,  24,  25,  10 };
# ifdef YYDEBUG
# include "y.debug"
# endif

# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

#ifdef YYDEBUG
int yydebug = 0; /* 1 for debugging */
#endif
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse()
{	short yys[YYMAXDEPTH];
	int yyj, yym;
	register YYSTYPE *yypvt;
	register int yystate, yyn;
	register short *yyps;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

yystack:    /* put a state and value onto the stack */
#ifdef YYDEBUG
	if(yydebug >= 3)
		if(yychar < 0 || yytoknames[yychar] == 0)
			print("char %d in %s", yychar, yystates[yystate]);
		else
			print("%s in %s", yytoknames[yychar], yystates[yystate]);
#endif
	if( ++yyps >= &yys[YYMAXDEPTH] ) { 
		yyerror( "yacc stack overflow" ); 
		return(1); 
	}
	*yyps = yystate;
	++yypv;
	*yypv = yyval;
yynewstate:
	yyn = yypact[yystate];
	if(yyn <= YYFLAG) goto yydefault; /* simple state */
	if(yychar<0) {
		yychar = yylex();
#ifdef YYDEBUG
		if(yydebug >= 2) {
			if(yychar <= 0)
				print("lex EOF\n");
			else if(yytoknames[yychar])
				print("lex %s\n", yytoknames[yychar]);
			else
				print("lex (%c)\n", yychar);
		}
#endif
		if(yychar < 0)
			yychar = 0;
	}
	if((yyn += yychar) < 0 || yyn >= YYLAST)
		goto yydefault;
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
		if(yychar < 0) {
			yychar = yylex();
#ifdef YYDEBUG
			if(yydebug >= 2)
				if(yychar < 0)
					print("lex EOF\n");
				else
					print("lex %s\n", yytoknames[yychar]);
#endif
			if(yychar < 0)
				yychar = 0;
		}
		/* look through exception table */
		for(yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate);
			yyxi += 2 ) ; /* VOID */
		while( *(yyxi+=2) >= 0 ){
			if( *yyxi == yychar ) break;
		}
		if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
	}
	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */
		switch( yyerrflag ){
		case 0:   /* brand new error */
#ifdef YYDEBUG
			yyerror("syntax error\n%s", yystates[yystate]);
			if(yytoknames[yychar])
				yyerror("saw %s\n", yytoknames[yychar]);
			else if(yychar >= ' ' && yychar < '\177')
				yyerror("saw `%c'\n", yychar);
			else if(yychar == 0)
				yyerror("saw EOF\n");
			else
				yyerror("saw char 0%o\n", yychar);
#else
			yyerror( "syntax error" );
#endif
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
#ifdef YYDEBUG
				if( yydebug ) print( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
#endif
				--yyps;
				--yypv;
			}
			/* there is no state on the stack with an error shift ... abort */
yyabort:
			return(1);
		case 3:  /* no shift yet; clobber input char */
#ifdef YYDEBUG
			if( yydebug ) {
				print("error recovery discards ");
				if(yytoknames[yychar])
					print("%s\n", yytoknames[yychar]);
				else if(yychar >= ' ' && yychar < '\177')
					print("`%c'\n", yychar);
				else if(yychar == 0)
					print("EOF\n");
				else
					print("char 0%o\n", yychar);
			}
#endif
			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */
		}
	}
	/* reduction by production yyn */
#ifdef YYDEBUG
	if(yydebug) {	char *s;
		print("reduce %d in:\n\t", yyn);
		for(s = yystates[yystate]; *s; s++) {
			putchar(*s);
			if(*s == '\n' && *(s+1))
				putchar('\t');
		}
	}
#endif
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
		
case 7:
# line 35 "gram.y"
{ s_alternate(yypvt[-0].num); } break;
case 8:
# line 36 "gram.y"
{ s_capacity(yypvt[-0].num, (long *)0, (long *)0); } break;
case 9:
# line 37 "gram.y"
{ s_config(); } break;
case 10:
# line 38 "gram.y"
{/*:COPY sdrive sstart nblocks ddrive dstart:: */
			s_copy(yypvt[-4].num, yypvt[-3].num, yypvt[-2].num, yypvt[-1].num, yypvt[-0].num);
		} break;
case 11:
# line 41 "gram.y"
{ s_diskeject(yypvt[-0].num); } break;
case 12:
# line 42 "gram.y"
{ s_diskid(yypvt[-0].num); } break;
case 13:
# line 43 "gram.y"
{ s_diskrelease(yypvt[-1].num, yypvt[-0].num, yypvt[-2].num, 1); } break;
case 14:
# line 44 "gram.y"
{ s_diskrelease(-1, A, yypvt[-0].num, 1); } break;
case 15:
# line 45 "gram.y"
{ s_diskset(yypvt[-2].num, yypvt[-1].num, yypvt[-0].num, 1); } break;
case 16:
# line 46 "gram.y"
{ print(">> %d <<\n", yypvt[-0].num); } break;
case 17:
# line 47 "gram.y"
{ help(); } break;
case 18:
# line 48 "gram.y"
{ scsiid(yypvt[-0].num); } break;
case 19:
# line 49 "gram.y"
{
			int i;
			for(i = 0; i<8; i++)
				s_inquiry(scsi_id, i);
		} break;
case 20:
# line 54 "gram.y"
{ s_inquiry(scsi_id, yypvt[-0].num); } break;
case 21:
# line 55 "gram.y"
{ s_internal(-1, 0); } break;
case 22:
# line 56 "gram.y"
{ s_internal(yypvt[-1].num, yypvt[-0].num); } break;
case 23:
# line 57 "gram.y"
{ s_media(yypvt[-2].num, yypvt[-1].num, yypvt[-0].num, 0); } break;
case 24:
# line 58 "gram.y"
{ s_mediaf(yypvt[-3].num, yypvt[-2].num, yypvt[-1].num, yypvt[-0].str); } break;
case 25:
# line 59 "gram.y"
{ s_media(yypvt[-2].num, yypvt[-1].num, yypvt[-0].num, 1); } break;
case 26:
# line 60 "gram.y"
{
			struct scsi_o output;
			s_read(yypvt[-1].num, yypvt[-0].num, 1, &output);
			scsiodump(output.data, 1024);
		} break;
case 27:
# line 65 "gram.y"
{
			char name[256];
;
			s_readid(yypvt[-0].num, name);
			print("id='%s'\n", name);
		} break;
case 28:
# line 71 "gram.y"
{ s_reset(); } break;
case 29:
# line 72 "gram.y"
{ s_sense(yypvt[-0].num, 0); } break;
case 30:
# line 73 "gram.y"
{ s_sense(yypvt[-0].num, 1); } break;
case 31:
# line 74 "gram.y"
{ sleep(yypvt[-0].num); } break;
case 32:
# line 75 "gram.y"
{ s_start(yypvt[-0].num); } break;
case 33:
# line 76 "gram.y"
{ s_status(yypvt[-0].num, (struct scsi_o *)0); } break;
case 34:
# line 77 "gram.y"
{ s_status(0, (struct scsi_o *)0); } break;
case 35:
# line 78 "gram.y"
{ s_stop(yypvt[-0].num); } break;
case 36:
# line 79 "gram.y"
{ s_testunit(scsi_id, yypvt[-0].num); } break;
case 37:
# line 80 "gram.y"
{ s_worm(yypvt[-0].num, 1); } break;
case 38:
# line 81 "gram.y"
{ s_worm(yypvt[-1].num, yypvt[-0].num); } break;
case 39:
# line 82 "gram.y"
{ s_write(yypvt[-1].num, yypvt[-0].num, 1); } break;
case 40:
# line 83 "gram.y"
{ s_write(yypvt[-2].num, yypvt[-1].num, yypvt[-0].num); } break;
case 43:
# line 90 "gram.y"
{ yyval.num = 0; } break;
case 45:
# line 96 "gram.y"
{ yyval.num = 0; } break;
case 46:
# line 97 "gram.y"
{ yyval.num = 1; } break;
	}
	goto yystack;  /* stack new state and value */
}
