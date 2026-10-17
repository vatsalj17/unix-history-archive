# include <stdio.h>
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYERROR yysvec
# define YYSTATE (yystate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?*--yysptr:getc(yyin))=='\n'?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO printf("%s",yytext);
# define REJECT { nstr = yyreject(); goto fussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin {stdin}, *yyout {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yystate;
extern struct yysvf yysvec[], *yybgin;
	int cm,nf,jc,ec;
# define YYNEWLINE 10
yylex(){
int nstr;
char c;
int i;
while((nstr = yylook()) >= 0)
fussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
{
		/* end of sentence */
		c = yytext[yyleng-1];
		if('a' <= c && c <= 'z')
			yytext[yyleng-1] = c^040;
		ECHO;
		}
break;
case 2:
{
		/* capt letter */
		c = yytext[yyleng-1];
		printf("%c",'a'<=c&&c<='z'?c^040:c);
		}
break;
case 3:
{
		/* capt all letters */
		for(i=0;i<yyleng;i++){
			c = yytext[i];
			yytext[i] = 'a' <= c && c <= 'z' ? c^040 : c;
			}
		if(yytext[yyleng-1] == '.'){
			unput(yytext[yyleng-1]);
			yytext[yyleng-1] = 0;
			}
		printf("%s",yytext+1);
		}
break;
case 4:
{printf(".ls 2\n");}
break;
case 5:
{printf(".ls 1\n"); }
break;
case 6:
{printf(".in 0\n"); }
break;
case 7:
{printf(".ex\n");}
break;
case 8:
;
break;
case 9:
	;
break;
case 10:
	;
break;
case 11:
	;
break;
case 12:
{printf(".ti -%s\n",&yytext[3]); }
break;
case 13:
;
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
int yyvstop[]{
0,

3,
0,

9,
0,

13,
0,

3,
0,

2,
0,

1,
0,

10,
0,

11,
0,

4,
0,

7,
0,

6,
0,

5,
0,

12,
0,

8,
0,
0};
# define YYTYPE char
struct yywork { YYTYPE verify, advance; } yycrank[] {
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	13,12,	
23,35,	2,8,	24,36,	30,40,	
14,0,	3,12,	26,12,	31,41,	
4,14,	32,42,	33,43,	45,49,	
0,0,	0,0,	0,0,	0,0,	
0,0,	4,4,	0,0,	0,0,	
0,0,	13,26,	1,3,	2,9,	
1,4,	1,5,	14,0,	3,13,	
26,26,	3,3,	14,0,	38,47,	
39,48,	47,50,	3,3,	1,3,	
3,3,	2,10,	2,11,	4,4,	
4,14,	4,14,	6,16,	4,0,	
11,23,	34,44,	50,37,	4,14,	
4,14,	11,24,	51,52,	52,53,	
1,3,	53,54,	54,55,	4,14,	
0,0,	0,0,	0,0,	0,0,	
0,0,	34,45,	34,45,	34,45,	
34,45,	34,45,	34,45,	34,45,	
34,45,	34,45,	34,45,	0,0,	
4,14,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	5,5,	
0,0,	0,0,	0,0,	1,6,	
46,6,	0,0,	0,0,	9,17,	
0,0,	0,0,	0,0,	19,31,	
0,0,	0,0,	17,29,	28,38,	
48,51,	0,0,	0,0,	20,32,	
22,34,	5,5,	27,37,	29,39,	
16,27,	16,28,	18,30,	21,33,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	1,7,	
0,0,	0,0,	0,0,	0,0,	
14,0,	44,45,	44,45,	44,45,	
44,45,	44,45,	44,45,	44,45,	
44,45,	44,45,	44,45,	0,0,	
0,0,	4,0,	5,15,	5,15,	
5,15,	5,15,	5,15,	5,15,	
5,15,	5,15,	5,15,	5,15,	
5,15,	5,15,	5,15,	5,15,	
5,15,	5,15,	5,15,	5,15,	
5,15,	5,15,	5,15,	5,15,	
5,15,	5,15,	5,15,	5,15,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	5,15,	5,15,	
5,15,	5,15,	5,15,	5,15,	
5,15,	5,15,	5,15,	5,15,	
5,15,	5,15,	5,15,	5,15,	
5,15,	5,15,	5,15,	5,15,	
5,15,	5,15,	5,15,	5,15,	
5,15,	5,15,	5,15,	5,15,	
10,12,	12,12,	0,0,	0,0,	
0,0,	0,0,	12,12,	12,12,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	10,13,	0,0,	
10,3,	0,0,	0,0,	0,0,	
0,0,	10,3,	0,0,	10,3,	
12,25,	12,25,	12,25,	12,25,	
12,25,	12,25,	12,25,	12,25,	
12,25,	12,25,	12,25,	12,25,	
12,25,	12,25,	12,25,	12,25,	
12,25,	12,25,	12,25,	12,25,	
12,25,	12,25,	12,25,	12,25,	
12,25,	12,25,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
12,25,	12,25,	12,25,	12,25,	
12,25,	12,25,	12,25,	12,25,	
12,25,	12,25,	12,25,	12,25,	
12,25,	12,25,	12,25,	12,25,	
12,25,	12,25,	12,25,	12,25,	
12,25,	12,25,	12,25,	12,25,	
12,25,	12,25,	10,18,	10,19,	
37,46,	0,0,	0,0,	10,20,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	10,21,	0,0,	10,22,	
0,0,	55,55,	0,0,	0,0,	
0,0,	0,0,	37,37,	0,0,	
0,0,	0,0,	55,56,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	37,37,	37,37,	
37,37,	37,37,	37,37,	37,37,	
37,37,	37,37,	37,37,	37,37,	
55,55,	55,55,	55,55,	0,0,	
55,55,	0,0,	0,0,	0,0,	
55,55,	55,55,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
55,55,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	55,55,	0,0,	0,0,	
0,0};
struct yysvf yysvec[] {
0,	0,	0,
yycrank+1,	0,		0,	
yycrank+3,	yysvec+1,	0,	
yycrank+7,	0,		0,	
yycrank+-19,	0,		yyvstop+1,
yycrank+81,	0,		0,	
yycrank+8,	0,		0,	
yycrank+0,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+5,
yycrank+1,	0,		0,	
yycrank+194,	0,		0,	
yycrank+14,	0,		0,	
yycrank+171,	0,		0,	
yycrank+1,	0,		0,	
yycrank+-6,	yysvec+4,	yyvstop+7,
yycrank+0,	0,		yyvstop+9,
yycrank+1,	0,		0,	
yycrank+1,	0,		0,	
yycrank+3,	0,		0,	
yycrank+1,	0,		0,	
yycrank+1,	0,		0,	
yycrank+4,	0,		0,	
yycrank+2,	0,		0,	
yycrank+2,	0,		0,	
yycrank+4,	0,		0,	
yycrank+0,	0,		yyvstop+11,
yycrank+8,	yysvec+12,	0,	
yycrank+2,	0,		0,	
yycrank+2,	0,		0,	
yycrank+5,	0,		0,	
yycrank+5,	0,		0,	
yycrank+9,	0,		0,	
yycrank+11,	0,		0,	
yycrank+12,	0,		0,	
yycrank+25,	0,		0,	
yycrank+0,	0,		yyvstop+13,
yycrank+0,	0,		yyvstop+15,
yycrank+286,	0,		0,	
yycrank+11,	0,		0,	
yycrank+12,	0,		0,	
yycrank+0,	0,		yyvstop+17,
yycrank+0,	0,		yyvstop+19,
yycrank+0,	0,		yyvstop+21,
yycrank+0,	0,		yyvstop+23,
yycrank+85,	0,		0,	
yycrank+13,	yysvec+44,	0,	
yycrank+2,	yysvec+12,	0,	
yycrank+2,	0,		0,	
yycrank+2,	0,		0,	
yycrank+0,	0,		yyvstop+25,
yycrank+5,	0,		0,	
yycrank+13,	0,		0,	
yycrank+14,	0,		0,	
yycrank+33,	0,		0,	
yycrank+34,	0,		0,	
yycrank+-312,	0,		0,	
yycrank+0,	0,		yyvstop+27,
0,	0,	0};
struct yywork *yytop yycrank+377;
struct yysvf *yybgin yysvec+1;
char yymatch[]{
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
040 ,'!' ,'"' ,01  ,'$' ,01  ,01  ,'"' ,
'(' ,')' ,01  ,01  ,01  ,01  ,'!' ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,01  ,01  ,01  ,01  ,01  ,'!' ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,01  ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,'$' ,01  ,
0};
char yyextra[] {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
int yylineno 1;
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr yysbuf;
int *yyfnd;
struct yysvf *yystate;
int yyprevious YYNEWLINE;
yylook(){
	register struct yysvf *state, **lsp;
	register struct yywork *t;
	struct yysvf *z;
	int ch;
	struct yywork *r;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng-1;
		}
	for(;;){
		lsp = yylstate;
		yystate = state = yybgin;
		if (yyprevious==YYNEWLINE) state++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",state-yysvec-1);
# endif
			t = state->yystoff;
			if(t == yycrank){		/* may not be any transitions */
				z = state->yyother;
				if(z == 0)break;
				if(z->yystoff == yycrank)break;
				}
			*yylastch++ = ch = input();
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(ch);
				putchar('\n');
				}
# endif
			r = t;
			if(t > yycrank){
				t = r + ch;
				if(t <= yytop && t->verify+yysvec == state){
					if(t->advance+yysvec == YYERROR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = state = t->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if(t < yycrank) {		/* r < yycrank */
				t = r = yycrank+(yycrank-t);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				t = t + ch;
				if(t <= yytop && t->verify+yysvec == state){
					if(t->advance+yysvec == YYERROR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = state = t->advance+yysvec;
					goto contin;
					}
				t = r + yymatch[ch];
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(yymatch[ch]);
					putchar('\n');
					}
# endif
				if(t <= yytop && t->verify+yysvec == state){
					if(t->advance+yysvec == YYERROR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = state = t->advance+yysvec;
					goto contin;
					}
				}
			if ((state = state->yyother) && (t= state->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",state-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",state-yysvec-1);
				allprint(ch);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(ch);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = *yylastch;
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0 && feof(yyin))
			return(0);
		output(yyprevious = input());
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
