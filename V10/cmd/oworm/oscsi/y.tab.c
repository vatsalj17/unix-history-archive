
# line 4 "gram.y"

#include	"scsi.h"


# line 16 "gram.y"
typedef union  {
	char *str;
	long num;
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
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 89 "gram.y"


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
# define YYNPROD 40
# define YYLAST 257
short yyact[]={

   6,   9,  72,   8,  64,  65,  10,  13,   3,  15,
  23,  12,  39,  37,  63,  14,  16,  17,  19,  18,
   7,  26,  11,  20,  21,   5,  36,   4,  74,  73,
  26,  68,  67,  66,  53,  61,  60,  59,  57,  54,
  49,  41,  32,  30,  25,  31,  29,  47,  52,  27,
  28,   2,   1,   0,  33,  34,  35,   0,  38,  24,
  40,   0,  42,  43,  44,  45,  46,   0,   0,   0,
   0,   0,   0,   0,  50,  51,   0,  70,   0,   0,
   0,  55,  56,   0,  58,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  48,   0,   0,   0,
  62,   0,   0,   0,   0,   0,  69,   0,  71,   0,
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
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  22 };
short yypact[]={

-1000,-257,   0,-237,-237,-237,-216,-225,-1000,-237,
-237,-237,-253,-246,-1000,-237,-226,-237,-237,-237,
-237,-237,  37,-1000,-1000,-1000,-1000,-227,-1000,-237,
-237,-233,-1000,-1000,-1000,-228,-237,-237,-229,-237,
-1000,-1000,-1000,-1000,-1000,-230,-231,-1000,-1000,-232,
-1000,-233,-265,-1000,-234,-235,-1000,-1000,-1000,-1000,
-236,-237,-265,-237,-1000,-1000,-281,-238,-1000,-239,
-1000,-1000,-1000,-1000,-1000 };
short yypgo[]={

   0,  52,  44,  14,  48,  51 };
short yyr1[]={

   0,   1,   1,   1,   1,   1,   5,   5,   5,   5,
   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
   5,   5,   5,   5,   5,   5,   2,   4,   3,   3 };
short yyr2[]={

   0,   0,   4,   3,   3,   4,   0,   2,   6,   2,
   3,   5,   3,   5,   2,   1,   2,   1,   2,   4,
   5,   5,   3,   3,   1,   2,   3,   2,   2,   2,
   1,   2,   2,   3,   3,   4,   1,   1,   1,   1 };
short yychk[]={

-1000,  -1,  -5, 265, 284, 282, 257, 277, 260, 258,
 263, 279, 268, 264, 272, 266, 273, 274, 276, 275,
 280, 281, 256,  10,  59,  -2, 267,  -2,  -2, 262,
 259, 261, 267,  -2,  -2,  -2, 279, 266,  -2, 258,
  -2, 267,  -2,  -2,  -2,  -2,  -2,  10,  59, 267,
  -2,  -2,  -4, 267, 267,  -2,  -2, 267,  -2, 267,
 267, 267,  -4,  -3, 269, 270, 267, 267, 267,  -2,
  -3,  -2, 283, 267, 267 };
short yydef[]={

   1,  -2,   0,   0,   0,   0,   0,   0,  15,   0,
  17,   0,   0,   0,  24,   0,   0,   0,  30,   0,
   0,   0,   0,   3,   4,   7,  36,   0,   9,   0,
   0,   0,  14,  16,  18,   0,   0,   0,   0,   0,
  25,  27,  28,  29,  31,  32,   0,   2,   5,   0,
  10,  12,   0,  37,   0,   0,  26,  22,  23,  33,
  34,   0,   0,   0,  38,  39,  19,   0,  35,   0,
  11,  13,  20,  21,   8 };
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
			printf("char %d in %s", yychar, yystates[yystate]);
		else
			printf("%s in %s", yytoknames[yychar], yystates[yystate]);
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
				printf("lex EOF\n");
			else if(yytoknames[yychar])
				printf("lex %s\n", yytoknames[yychar]);
			else
				printf("lex (%c)\n", yychar);
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
					printf("lex EOF\n");
				else
					printf("lex %s\n", yytoknames[yychar]);
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
				if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
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
				printf("error recovery discards ");
				if(yytoknames[yychar])
					printf("%s\n", yytoknames[yychar]);
				else if(yychar >= ' ' && yychar < '\177')
					printf("`%c'\n", yychar);
				else if(yychar == 0)
					printf("EOF\n");
				else
					printf("char 0%o\n", yychar);
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
		printf("reduce %d in:\n\t", yyn);
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
# line 34 "gram.y"
{ s_capacity(yypvt[-0].num, (long *)0, (long *)0); } break;
case 8:
# line 35 "gram.y"
{/*:COPY sdrive sstart nblocks ddrive dstart:: */
			s_copy(yypvt[-4].num, yypvt[-3].num, yypvt[-2].num, yypvt[-1].num, yypvt[-0].num);
		} break;
case 9:
# line 38 "gram.y"
{ s_drdiag(yypvt[-0].num); } break;
case 10:
# line 39 "gram.y"
{ s_diskeject(yypvt[-0].num); } break;
case 11:
# line 40 "gram.y"
{ s_diskrelease(yypvt[-1].num, yypvt[-0].num, yypvt[-2].num, 1); } break;
case 12:
# line 41 "gram.y"
{ s_diskrelease(-1, A, yypvt[-0].num, 1); } break;
case 13:
# line 42 "gram.y"
{ s_diskset(yypvt[-2].num, yypvt[-1].num, yypvt[-0].num, 1); } break;
case 14:
# line 43 "gram.y"
{ print(">> %d <<\n", yypvt[-0].num); } break;
case 15:
# line 44 "gram.y"
{ help(); } break;
case 16:
# line 45 "gram.y"
{ scsiid(yypvt[-0].num); } break;
case 17:
# line 46 "gram.y"
{
			int i;
			for(i = 0; i<8; i++)
				s_inquiry(scsi_id, i);
		} break;
case 18:
# line 51 "gram.y"
{ s_inquiry(scsi_id, yypvt[-0].num); } break;
case 19:
# line 52 "gram.y"
{ s_media(yypvt[-2].num, yypvt[-1].num, yypvt[-0].num, 0); } break;
case 20:
# line 53 "gram.y"
{ s_mediaf(yypvt[-3].num, yypvt[-2].num, yypvt[-1].num, yypvt[-0].str); } break;
case 21:
# line 54 "gram.y"
{ s_media(yypvt[-2].num, yypvt[-1].num, yypvt[-0].num, 1); } break;
case 22:
# line 55 "gram.y"
{
			struct scsi_o output;
			s_read(yypvt[-1].num, yypvt[-0].num, 1, &output);
			scsiodump(output.data, 1024);
		} break;
case 23:
# line 60 "gram.y"
{
			struct scsi_o output;
			s_read(yypvt[-0].num, 1, 1, &output);
			print("id='%s'\n", &output.data[42]);
		} break;
case 24:
# line 65 "gram.y"
{ s_reset(); } break;
case 25:
# line 66 "gram.y"
{ s_sense(yypvt[-0].num, 0); } break;
case 26:
# line 67 "gram.y"
{ s_sense(yypvt[-0].num, 1); } break;
case 27:
# line 68 "gram.y"
{ sleep(yypvt[-0].num); } break;
case 28:
# line 69 "gram.y"
{ s_start(yypvt[-0].num); } break;
case 29:
# line 70 "gram.y"
{ s_status(yypvt[-0].num, (struct scsi_o *)0); } break;
case 30:
# line 71 "gram.y"
{ s_status(0, (struct scsi_o *)0); } break;
case 31:
# line 72 "gram.y"
{ s_stop(yypvt[-0].num); } break;
case 32:
# line 73 "gram.y"
{ s_worm(yypvt[-0].num, 1); } break;
case 33:
# line 74 "gram.y"
{ s_worm(yypvt[-1].num, yypvt[-0].num); } break;
case 34:
# line 75 "gram.y"
{ s_write(yypvt[-1].num, yypvt[-0].num, 1); } break;
case 35:
# line 76 "gram.y"
{ s_write(yypvt[-2].num, yypvt[-1].num, yypvt[-0].num); } break;
case 38:
# line 85 "gram.y"
{ yyval.num = 0; } break;
case 39:
# line 86 "gram.y"
{ yyval.num = 1; } break;
	}
	goto yystack;  /* stack new state and value */
}
