#
# define LCURL 257
# define RCURL 258
# define LPAR 259
# define RPAR 260
# define SCOL 261
# define DIGITS 262
# define XIF 263
# define XELSE 264
# define XFOR 265
# define XWHILE 266
# define XBREAK 267
# define NEXT 268
# define OLDDO 269
# define NEWDO 270
# define XGOK 271
# define XDEFINE 272
# define XINCLUDE 273
# define REPEAT 274
# define UNTIL 275
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar, yyerrflag;

int yyval 0;
int *yypv;
int yylval 0;
yyactr(__np__){

switch(__np__){

case 3:{ outcont(yypv[1]); } break;
case 4:{ outcont(yypv[1]+1); } break;
case 5:{ whilestat(yypv[1]); } break;
case 6:{ forstat(yypv[1]); } break;
case 7:{ nountil(yypv[1]); } break;
case 8:{ untils(yypv[1]); } break;
case 9:{ breakcode(yypv[1]); } break;
case 10:{ nextcode(yypv[1]); } break;
case 11:{ dostat(yypv[1]); } break;
case 12:{ docode(0,yypv[1]); } break;
case 13:{ gokcode(yypv[1]); } break;
case 17:{ errcode(yypv[1]); yyclearin; } break;
case 18:{ outcode(yypv[1]); outcode("\t"); } break;
case 19:{ ifcode(yypv[1]); } break;
case 20:{ outgoto(yypv[1]+1); outcont(yypv[1]); } break;
case 21:{ whilecode(yypv[1]); } break;
case 22:{ forcode(yypv[1]); } break;
case 23:{ repcode(yypv[1]); } break;
case 24:{ docode(1,yypv[1]); } break;
}
return(0);
}
int yyerrval 256;


int yyact[] {0,12290,4096,16384,4352,8208,4353,8206,4357,8205
,4358,8214,4359,8209,4361,8211,4362,8210,4363,8200
,4364,8201,4365,8203,4366,8213,4367,8204,4370,8212
,0,12289,4352,8208,4353,8206,4357,8205,4358,8214
,4359,8209,4361,8211,4362,8210,4363,8200,4364,8201
,4365,8203,4366,8213,4367,8204,4370,8212,0,12297
,12298,12300,12301,12302,12305,12307,12309,12310,12311,12312
,12306,4360,8223,12291,12292,12293,12294,4371,8224,12295
,12299,4352,8208,4353,8206,4354,8225,4357,8205,4358
,8214,4359,8209,4361,8211,4362,8210,4363,8200,4364
,8201,4365,8203,4366,8213,4367,8204,4370,8212,0
,12304,12308,12296,12303,-1};

int yypact[] {0,1,2,31,32,32,32,32,32,59
,60,32,61,62,63,1,32,64,65,66
,67,68,69,70,71,74,75,76,77,80
,81,110,111,112,113,-1};

int yyr1[] {0,1,1,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,9,3
,4,5,6,7,8,-1};

int yyr2[] {0,2,0,2,2,2,2,2,3,1
,1,2,1,1,1,3,2,1,1,1
,3,1,1,1,1,-1};

int yygo[] {0,14,29,-1,1,3,23,4,24,5
,25,6,26,7,27,10,28,15,30,-1
,2,-1,3,-1,4,-1,5,-1,6,-1
,7,-1,10,-1,15,-1};

int yypgo[] {0,1,5,21,23,25,27,29,31,33
,-1};

int nterms 21;
int nnonter 9;
int nstate 34;
char *yysterm[] {
"error",
"LCURL",
"RCURL",
"LPAR",
"RPAR",
"SCOL",
"DIGITS",
"XIF",
"XELSE",
"XFOR",
"XWHILE",
"XBREAK",
"NEXT",
"OLDDO",
"NEWDO",
"XGOK",
"XDEFINE",
"XINCLUDE",
"REPEAT",
"UNTIL",
0 };

char *yysnter[] {
"$accept",
"statl",
"stat",
"if",
"ifelse",
"while",
"for",
"repeat",
"newdo",
"label" };
