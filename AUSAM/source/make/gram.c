#
#include "defs"
# define NAME 257
# define SHELLINE 258
# define START 259
# define MACRODEF 260
# define COLON 261
# define DOUBLECOLON 262
# define ARCHIVE 263
# define ARCTERM 264
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar, yyerrflag;

int yyval 0;
int *yypv;
int yylval 0;
yyactr(__np__){
struct depblock *pp;
FSTATIC struct shblock *prevshp;

FSTATIC struct nameblock *lefts[NLEFTS];
struct nameblock *leftp;
FSTATIC int nlefts;

struct lineblock *lp, *lpp;
FSTATIC struct depblock *prevdep;
FSTATIC int sepc;

struct nameblock *curarcnam;

extern int yylineno;


switch(__np__){

case 5: {
	    while( --nlefts >= 0)
		{
		leftp = lefts[nlefts];
		if(leftp->septype == 0)
			leftp->septype = sepc;
		else if(leftp->septype != sepc)
			fprintf(stderr, "Inconsistent rules lines for `%s'\n",
				leftp->namep);
		else if(sepc==ALLDEPS && *(leftp->namep)!='.' && yypv[4]!=0)
			{
			for(lp=leftp->linep; lp->nextp!=0; lp=lp->nextp)
			    if(lp->shp)
				fprintf(stderr, "Multiple rules lines for `%s'\n",
				    leftp->namep);
			}

		lp = intalloc(sizeof(*lp));
		lp->nextp = 0;
		lp->depp = yypv[3];
		lp->shp = yypv[4];

		if(equals(leftp->namep, ".SUFFIXES") && yypv[3]==0)
			leftp->linep = 0;
		else if(leftp->linep == 0)
			leftp->linep = lp;
		else	{
			for(lpp = leftp->linep; lpp->nextp!=0;
				lpp = lpp->nextp) ;
				if(sepc == ALLDEPS)
					lpp->shp = 0;
			lpp->nextp = lp;
			}
		}
	} break;
case 7: { lefts[0] = yypv[1]; nlefts = 1; } break;
case 8: { lefts[nlefts++] = yypv[2];
	    	if(nlefts>NLEFTS) yyerror("Too many lefts"); } break;
case 9: { yyerror("Must be a separator on rules line"); } break;
case 11: { prevdep = 0;  yyval = 0; } break;
case 12: {
			  if( yypv[1] )
				yyval = yypv[1];
			  else
				yyval = yypv[2];
			  } break;
case 13: {
			  pp = intalloc(sizeof(*pp));
			  pp->nextp = 0;
			  pp->depname = yypv[2];
			  if(prevdep == 0) yyval = pp;
			  else  prevdep->nextp = pp;
			  prevdep = pp;
			  } break;
case 14: {
				  yyval = yypv[2];  /* note archive not a dependancy */
				  } break;
case 15: {
				yyerror("No modules for archive");
			    } break;
case 16:	{
			pp = intalloc(sizeof(*pp));
			pp->nextp = 0;
			pp->depname = yypv[1];
			if(prevdep == 0) yyval = pp;
			else  prevdep->nextp = pp;
			prevdep = pp;
			} break;
case 17: {
			 pp = intalloc(sizeof(*pp));
			 pp->nextp = 0;
			 pp->depname = yypv[2];
			 if(prevdep == 0) yyval = pp;
			 else  prevdep->nextp = pp;
			 prevdep = pp;
			 } break;
case 18: { sepc = ALLDEPS; } break;
case 19: { sepc = SOMEDEPS; } break;
case 20: {yyval = 0; } break;
case 21: { yyval = yypv[1]; } break;
case 22: { yyval = yypv[1];  prevshp = yypv[1]; } break;
case 23: { yyval = yypv[1];
			prevshp->nextp = yypv[2];
			prevshp = yypv[2];
			} break;
}
}
int yyerrval 256;




# include "lex.c"

int yyact[] {0,12289,4096,16384,4352,8197,4355,8195,4356,8196
,0,12290,4353,8199,12291,12292,12294,4353,8201,4357
,8204,4358,8205,12297,12295,4354,8208,12308,12296,4353
,8210,4359,8211,12298,12299,12306,12307,12293,4354,8212
,12309,12310,12300,12301,4353,8215,4360,8214,0,12311
,4353,8217,4360,8216,0,12303,12304,12302,12305,-1};

int yypact[] {0,1,2,11,12,15,16,17,24,25
,28,29,34,35,36,37,38,41,42,43
,44,49,50,55,56,57,58,-1};

int yyr1[] {0,1,1,2,2,2,2,3,3,4
,4,6,6,6,8,8,9,9,7,7
,5,5,10,10,-1};

int yyr2[] {0,0,2,1,1,4,1,1,2,0
,1,1,2,2,3,2,1,2,1,1
,0,1,1,2,-1};

int yygo[] {0,-1,1,-1,2,-1,6,-1,8,-1
,14,-1,10,-1,11,-1,17,-1,21,-1
,15,-1};

int yypgo[] {0,1,3,5,7,9,11,13,15,17
,19,-1};

int nterms 10;
int nnonter 10;
int nstate 26;
char *yysterm[] {
"error",
"NAME",
"SHELLINE",
"START",
"MACRODEF",
"COLON",
"DOUBLECOLON",
"ARCHIVE",
"ARCTERM",
0 };

char *yysnter[] {
"$accept",
"file",
"comline",
"namelist",
"deplist",
"shellist",
"dlist",
"sepchar",
"arname",
"modulist",
"shlist" };
