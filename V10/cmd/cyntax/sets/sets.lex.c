# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
# define SOURCE 2
# define COMMENT 4
# define STRING 6
# define CHAR 8
# define DEFN 10
# define SETDEFN 12
# define INIT 14
# define C_DEFN 16
# define C_SETDEFN 18
# define C_INIT 20
#include	"sets.h"

#define	RETURN		return (int)
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
	{
				ECHO;
				BEGIN COMMENT;
			}
break;
case 2:
	{
				ECHO;
				BEGIN STRING;
			}
break;
case 3:
	{
				ECHO;
				BEGIN CHAR;
			}
break;
case 4:
	{
				BEGIN SETDEFN;
				RETURN sy_set;
			}
break;
case 5:
	{
				ECHO;
				BEGIN DEFN;
				RETURN sy_typedef;
			}
break;
case 6:
	{
				ECHO;
				BEGIN DEFN;
				RETURN sy_enum;
			}
break;
case 7:
ECHO;
break;
case 8:
	{
				BEGIN INIT;
				RETURN sy_on_squares;
			}
break;
case 9:
	putchar(*yytext);
break;
case 10:
	ECHO;
break;
case 11:
	{
				ECHO;
				BEGIN SOURCE;
			}
break;
case 12:
	ECHO;
break;
case 13:
	ECHO;
break;
case 14:
	ECHO;
break;
case 15:
	{
				ECHO;
				BEGIN SOURCE;
			}
break;
case 16:
	ECHO;
break;
case 17:
		ECHO;
break;
case 18:
	{
				ECHO;
				BEGIN SOURCE;
			}
break;
case 19:
	ECHO;
break;
case 20:
	{
				ECHO;
				BEGIN C_DEFN;
			}
break;
case 21:
	ECHO;
break;
case 22:
	{
				ECHO;
				RETURN sy_enum;
			}
break;
case 23:
	{
				BEGIN SETDEFN;
				RETURN sy_set;
			}
break;
case 24:
	{
				ECHO;
				RETURN sy_id;
			}
break;
case 25:
	{
				ECHO;
				RETURN sy_on_curly;
			}
break;
case 26:
	{
				ECHO;
				RETURN sy_off_curly;
			}
break;
case 27:
	{
				ECHO;
				RETURN sy_comma;
			}
break;
case 28:
		{
				ECHO;
				fprintf(stderr, "%s: line %d, ignored unknown character (%s) in definition\n", myname, yylineno, print_char());
			}
break;
case 29:
	{
				ECHO;
				BEGIN DEFN;
			}
break;
case 30:
	ECHO;
break;
case 31:
	{
				ECHO;
				BEGIN C_SETDEFN;
			}
break;
case 32:
	ECHO;
break;
case 33:
	RETURN sy_char;
break;
case 34:
RETURN sy_id;
break;
case 35:
	fprintf(stderr, "%s: line %d, ignored unknown character (%s) in set definition\n", myname, yylineno, print_char());
break;
case 36:
	{
				ECHO;
				BEGIN SETDEFN;
			}
break;
case 37:
	ECHO;
break;
case 38:
	{
				ECHO;
				BEGIN C_INIT;
			}
break;
case 39:
	ECHO;
break;
case 40:
	{
				BEGIN SOURCE;
				RETURN sy_off_squares;
			}
break;
case 41:
	RETURN sy_comma;
break;
case 42:
	RETURN sy_dot_dot;
break;
case 43:
	RETURN sy_id;
break;
case 44:
	{
				sscanf(yytext + 2, "%x", &yylval);
				RETURN sy_int;
			}
break;
case 45:
	{
				sscanf(yytext, "%o", &yylval);
				RETURN sy_int;
			}
break;
case 46:
	{
				sscanf(yytext, "%d", &yylval);
				RETURN sy_int;
			}
break;
case 47:
	{
				yylval = yytext[1];
				RETURN sy_int;
			}
break;
case 48:
{
				sscanf(yytext + 2, "%o", &yylval);
				RETURN sy_int;
			}
break;
case 49:
	{
				switch (yytext[2])
				{
				case 'n':
					yylval = '\n';
					break;

				case 't':
					yylval = '\t';
					break;

				case 'b':
					yylval = '\b';
					break;

				case 'r':
					yylval = '\r';
					break;

				case 'f':
					yylval = '\f';
					break;

				default:
					yylval = yytext[2];
					break;
				}
				RETURN sy_int;
			}
break;
case 50:
		fprintf(stderr, "%s: line %d, ignored unknown character (%s) in set initialisation\n", myname, yylineno, print_char());
break;
case 51:
	{
				ECHO;
				BEGIN INIT;
			}
break;
case 52:
	ECHO;
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */

begin_source()
{
	BEGIN SOURCE;
}

begin_setdefn()
{
	BEGIN SETDEFN;
}
int yyvstop[] = {
0,

9,
0,

2,
9,
0,

3,
9,
0,

9,
0,

7,
9,
0,

9,
0,

7,
9,
0,

7,
9,
0,

7,
9,
0,

19,
0,

19,
0,

13,
0,

11,
13,
0,

13,
0,

17,
0,

15,
17,
0,

17,
0,

28,
0,

21,
28,
0,

21,
0,

27,
28,
0,

28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

25,
28,
0,

26,
28,
0,

35,
0,

32,
35,
0,

32,
0,

35,
0,

34,
35,
0,

34,
35,
0,

50,
0,

39,
50,
0,

39,
0,

50,
0,

41,
50,
0,

50,
0,

50,
0,

46,
50,
0,

46,
50,
0,

43,
50,
0,

50,
0,

30,
0,

30,
0,

37,
0,

37,
0,

52,
0,

52,
0,

1,
0,

7,
0,

8,
0,

7,
0,

7,
0,

7,
0,

18,
0,

10,
0,

12,
0,

14,
0,

16,
0,

20,
0,

24,
0,

24,
0,

24,
0,

31,
0,

34,
0,

34,
0,

42,
0,

38,
0,

45,
46,
0,

46,
0,

43,
0,

40,
0,

29,
0,

36,
0,

51,
0,

7,
0,

4,
7,
0,

7,
0,

24,
0,

23,
24,
0,

34,
0,

47,
0,

47,
0,

44,
0,

6,
7,
0,

7,
0,

22,
24,
0,

33,
34,
0,

49,
0,

7,
0,

7,
0,

48,
0,

5,
7,
0,
0};
# define YYTYPE char
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	3,23,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	5,32,	3,23,	3,0,	
6,0,	7,34,	4,0,	8,0,	
0,0,	5,32,	5,0,	0,0,	
9,37,	7,34,	7,0,	18,0,	
10,0,	0,0,	0,0,	20,0,	
9,37,	9,0,	0,0,	22,0,	
0,0,	0,0,	0,0,	3,24,	
0,0,	0,0,	4,24,	8,35,	
3,25,	0,0,	0,0,	4,25,	
6,33,	26,73,	7,35,	44,84,	
3,26,	3,23,	5,33,	4,26,	
33,79,	10,38,	53,88,	18,68,	
5,32,	3,23,	9,38,	20,70,	
7,34,	61,93,	14,53,	22,72,	
5,32,	36,80,	3,27,	9,37,	
7,34,	62,94,	68,100,	70,101,	
3,27,	5,32,	72,102,	9,37,	
91,109,	7,34,	110,118,	5,32,	
3,27,	39,82,	121,123,	7,34,	
9,37,	11,40,	0,0,	5,32,	
0,0,	3,27,	9,37,	7,34,	
3,28,	11,41,	11,42,	28,75,	
5,32,	8,36,	9,37,	5,32,	
7,34,	0,0,	3,29,	7,34,	
7,36,	4,29,	10,39,	9,37,	
30,77,	12,43,	9,37,	9,39,	
12,44,	13,50,	14,55,	29,76,	
3,30,	3,31,	46,86,	4,30,	
4,31,	13,51,	13,52,	36,81,	
3,23,	31,78,	3,23,	4,23,	
11,43,	4,23,	47,87,	11,44,	
11,40,	55,90,	39,83,	41,42,	
41,42,	66,99,	41,42,	76,103,	
11,40,	51,52,	51,52,	77,104,	
51,52,	78,105,	86,106,	87,107,	
90,108,	11,45,	57,58,	57,58,	
103,114,	57,58,	105,115,	11,45,	
106,116,	108,117,	41,42,	13,53,	
13,50,	115,120,	119,121,	11,45,	
51,52,	120,122,	12,46,	122,124,	
13,50,	0,0,	15,56,	0,0,	
11,45,	57,58,	0,0,	11,40,	
0,0,	13,54,	15,57,	15,58,	
12,47,	0,0,	0,0,	13,54,	
0,0,	11,46,	0,0,	0,0,	
12,48,	0,0,	12,49,	13,54,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	11,47,	
13,54,	0,0,	0,0,	13,50,	
0,0,	0,0,	0,0,	11,48,	
15,59,	11,49,	0,0,	13,55,	
0,0,	15,60,	0,0,	15,61,	
15,62,	15,63,	15,64,	15,64,	
15,64,	15,64,	15,64,	15,64,	
15,64,	15,64,	0,0,	0,0,	
0,0,	0,0,	0,0,	16,59,	
112,118,	0,0,	15,65,	0,0,	
16,60,	0,0,	16,61,	16,62,	
15,65,	16,64,	16,64,	16,64,	
16,64,	16,64,	16,64,	16,64,	
15,65,	0,0,	17,67,	0,0,	
0,0,	0,0,	0,0,	19,69,	
0,0,	15,65,	17,67,	17,0,	
15,56,	0,0,	15,66,	19,69,	
19,0,	0,0,	21,71,	0,0,	
0,0,	0,0,	0,0,	0,0,	
112,119,	0,0,	21,71,	21,0,	
95,95,	95,95,	95,95,	95,95,	
95,95,	95,95,	95,95,	95,95,	
112,119,	16,66,	0,0,	0,0,	
0,0,	0,0,	0,0,	17,68,	
0,0,	0,0,	0,0,	0,0,	
19,70,	17,67,	0,0,	0,0,	
0,0,	0,0,	19,69,	0,0,	
0,0,	17,67,	0,0,	21,72,	
0,0,	0,0,	19,69,	0,0,	
0,0,	21,71,	17,67,	0,0,	
112,119,	0,0,	112,119,	19,69,	
17,67,	21,71,	0,0,	0,0,	
0,0,	19,69,	0,0,	0,0,	
17,67,	0,0,	21,71,	0,0,	
0,0,	19,69,	0,0,	0,0,	
21,71,	17,67,	0,0,	0,0,	
17,67,	0,0,	19,69,	0,0,	
21,71,	19,69,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	21,71,	0,0,	0,0,	
21,71,	27,74,	27,74,	27,74,	
27,74,	27,74,	27,74,	27,74,	
27,74,	27,74,	27,74,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	27,74,	27,74,	
27,74,	27,74,	27,74,	27,74,	
27,74,	27,74,	27,74,	27,74,	
27,74,	27,74,	27,74,	27,74,	
27,74,	27,74,	27,74,	27,74,	
27,74,	27,74,	27,74,	27,74,	
27,74,	27,74,	27,74,	27,74,	
0,0,	0,0,	0,0,	0,0,	
27,74,	0,0,	27,74,	27,74,	
27,74,	27,74,	27,74,	27,74,	
27,74,	27,74,	27,74,	27,74,	
27,74,	27,74,	27,74,	27,74,	
27,74,	27,74,	27,74,	27,74,	
27,74,	27,74,	27,74,	27,74,	
27,74,	27,74,	27,74,	27,74,	
45,85,	45,85,	45,85,	45,85,	
45,85,	45,85,	45,85,	45,85,	
45,85,	45,85,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	45,85,	45,85,	45,85,	
45,85,	45,85,	45,85,	45,85,	
45,85,	45,85,	45,85,	45,85,	
45,85,	45,85,	45,85,	45,85,	
45,85,	45,85,	45,85,	45,85,	
45,85,	45,85,	45,85,	45,85,	
45,85,	45,85,	45,85,	0,0,	
0,0,	0,0,	0,0,	45,85,	
0,0,	45,85,	45,85,	45,85,	
45,85,	45,85,	45,85,	45,85,	
45,85,	45,85,	45,85,	45,85,	
45,85,	45,85,	45,85,	45,85,	
45,85,	45,85,	45,85,	45,85,	
45,85,	45,85,	45,85,	45,85,	
45,85,	45,85,	45,85,	54,89,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	54,89,	54,89,	
54,89,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	0,0,	0,0,	
0,0,	0,0,	54,89,	0,0,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	59,91,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	59,91,	59,0,	
63,95,	63,95,	63,95,	63,95,	
63,95,	63,95,	63,95,	63,95,	
63,96,	63,96,	64,96,	64,96,	
64,96,	64,96,	64,96,	64,96,	
64,96,	64,96,	64,96,	64,96,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	59,91,	0,0,	0,0,	
63,97,	0,0,	0,0,	0,0,	
0,0,	59,91,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	59,91,	0,0,	
0,0,	0,0,	0,0,	0,0,	
59,91,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
59,91,	0,0,	0,0,	0,0,	
63,97,	0,0,	0,0,	0,0,	
0,0,	59,91,	0,0,	0,0,	
59,91,	59,92,	65,98,	65,98,	
65,98,	65,98,	65,98,	65,98,	
65,98,	65,98,	65,98,	65,98,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	65,98,	
65,98,	65,98,	65,98,	65,98,	
65,98,	65,98,	65,98,	65,98,	
65,98,	65,98,	65,98,	65,98,	
65,98,	65,98,	65,98,	65,98,	
65,98,	65,98,	65,98,	65,98,	
65,98,	65,98,	65,98,	65,98,	
65,98,	0,0,	0,0,	0,0,	
0,0,	65,98,	0,0,	65,98,	
65,98,	65,98,	65,98,	65,98,	
65,98,	65,98,	65,98,	65,98,	
65,98,	65,98,	65,98,	65,98,	
65,98,	65,98,	65,98,	65,98,	
65,98,	65,98,	65,98,	65,98,	
65,98,	65,98,	65,98,	65,98,	
65,98,	92,110,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	92,110,	92,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	97,113,	
97,113,	97,113,	97,113,	97,113,	
97,113,	97,113,	97,113,	97,113,	
97,113,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	92,111,	
97,113,	97,113,	97,113,	97,113,	
97,113,	97,113,	0,0,	0,0,	
92,112,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
92,110,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	92,110,	0,0,	0,0,	
0,0,	0,0,	0,0,	92,110,	
97,113,	97,113,	97,113,	97,113,	
97,113,	97,113,	0,0,	92,110,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
92,110,	0,0,	0,0,	92,110,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+0,	0,		0,	
yycrank+0,	0,		0,	
yycrank+-1,	0,		0,	
yycrank+-4,	yysvec+3,	0,	
yycrank+-8,	0,		0,	
yycrank+-2,	yysvec+5,	0,	
yycrank+-12,	0,		0,	
yycrank+-5,	yysvec+7,	0,	
yycrank+-19,	0,		0,	
yycrank+-14,	yysvec+9,	0,	
yycrank+-84,	0,		0,	
yycrank+-65,	yysvec+11,	0,	
yycrank+-112,	0,		0,	
yycrank+-15,	yysvec+13,	0,	
yycrank+-169,	0,		0,	
yycrank+-192,	yysvec+15,	0,	
yycrank+-249,	0,		0,	
yycrank+-13,	yysvec+17,	0,	
yycrank+-254,	0,		0,	
yycrank+-17,	yysvec+19,	0,	
yycrank+-265,	0,		0,	
yycrank+-21,	yysvec+21,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+0,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+3,	0,		yyvstop+9,
yycrank+309,	0,		yyvstop+11,
yycrank+4,	0,		yyvstop+14,
yycrank+5,	yysvec+27,	yyvstop+16,
yycrank+7,	yysvec+27,	yyvstop+19,
yycrank+4,	yysvec+27,	yyvstop+22,
yycrank+0,	0,		yyvstop+25,
yycrank+5,	0,		yyvstop+27,
yycrank+0,	0,		yyvstop+29,
yycrank+0,	0,		yyvstop+31,
yycrank+31,	0,		yyvstop+34,
yycrank+0,	0,		yyvstop+36,
yycrank+0,	0,		yyvstop+38,
yycrank+42,	0,		yyvstop+41,
yycrank+0,	0,		yyvstop+43,
yycrank+126,	0,		yyvstop+45,
yycrank+0,	yysvec+41,	yyvstop+48,
yycrank+0,	0,		yyvstop+50,
yycrank+5,	0,		yyvstop+53,
yycrank+384,	0,		yyvstop+55,
yycrank+8,	yysvec+45,	yyvstop+58,
yycrank+29,	yysvec+45,	yyvstop+61,
yycrank+0,	0,		yyvstop+64,
yycrank+0,	0,		yyvstop+67,
yycrank+0,	0,		yyvstop+70,
yycrank+132,	0,		yyvstop+72,
yycrank+0,	yysvec+51,	yyvstop+75,
yycrank+12,	0,		yyvstop+77,
yycrank+459,	0,		yyvstop+79,
yycrank+29,	yysvec+54,	yyvstop+82,
yycrank+0,	0,		yyvstop+85,
yycrank+141,	0,		yyvstop+87,
yycrank+0,	yysvec+57,	yyvstop+90,
yycrank+-581,	0,		yyvstop+92,
yycrank+0,	0,		yyvstop+94,
yycrank+15,	0,		yyvstop+97,
yycrank+27,	0,		yyvstop+99,
yycrank+544,	0,		yyvstop+101,
yycrank+554,	0,		yyvstop+104,
yycrank+626,	0,		yyvstop+107,
yycrank+44,	0,		yyvstop+110,
yycrank+0,	0,		yyvstop+112,
yycrank+23,	0,		yyvstop+114,
yycrank+0,	0,		yyvstop+116,
yycrank+24,	0,		yyvstop+118,
yycrank+0,	0,		yyvstop+120,
yycrank+27,	0,		yyvstop+122,
yycrank+0,	0,		yyvstop+124,
yycrank+0,	yysvec+27,	yyvstop+126,
yycrank+0,	0,		yyvstop+128,
yycrank+22,	yysvec+27,	yyvstop+130,
yycrank+27,	yysvec+27,	yyvstop+132,
yycrank+33,	yysvec+27,	yyvstop+134,
yycrank+0,	0,		yyvstop+136,
yycrank+0,	0,		yyvstop+138,
yycrank+0,	0,		yyvstop+140,
yycrank+0,	0,		yyvstop+142,
yycrank+0,	0,		yyvstop+144,
yycrank+0,	0,		yyvstop+146,
yycrank+0,	yysvec+45,	yyvstop+148,
yycrank+29,	yysvec+45,	yyvstop+150,
yycrank+31,	yysvec+45,	yyvstop+152,
yycrank+0,	0,		yyvstop+154,
yycrank+0,	yysvec+54,	yyvstop+156,
yycrank+51,	yysvec+54,	yyvstop+158,
yycrank+37,	0,		0,	
yycrank+-748,	0,		0,	
yycrank+0,	0,		yyvstop+160,
yycrank+0,	0,		yyvstop+162,
yycrank+228,	yysvec+64,	yyvstop+164,
yycrank+0,	yysvec+64,	yyvstop+167,
yycrank+723,	0,		0,	
yycrank+0,	yysvec+65,	yyvstop+169,
yycrank+0,	0,		yyvstop+171,
yycrank+0,	0,		yyvstop+173,
yycrank+0,	0,		yyvstop+175,
yycrank+0,	0,		yyvstop+177,
yycrank+43,	yysvec+27,	yyvstop+179,
yycrank+0,	yysvec+27,	yyvstop+181,
yycrank+53,	yysvec+27,	yyvstop+184,
yycrank+47,	yysvec+45,	yyvstop+186,
yycrank+0,	yysvec+45,	yyvstop+188,
yycrank+43,	yysvec+54,	yyvstop+191,
yycrank+0,	0,		yyvstop+193,
yycrank+39,	0,		0,	
yycrank+0,	yysvec+110,	yyvstop+195,
yycrank+193,	0,		0,	
yycrank+0,	yysvec+97,	yyvstop+197,
yycrank+0,	yysvec+27,	yyvstop+199,
yycrank+61,	yysvec+27,	yyvstop+202,
yycrank+0,	yysvec+45,	yyvstop+204,
yycrank+0,	yysvec+54,	yyvstop+207,
yycrank+0,	0,		yyvstop+210,
yycrank+69,	0,		0,	
yycrank+64,	yysvec+27,	yyvstop+212,
yycrank+43,	0,		0,	
yycrank+65,	yysvec+27,	yyvstop+214,
yycrank+0,	0,		yyvstop+216,
yycrank+0,	yysvec+27,	yyvstop+218,
0,	0,	0};
struct yywork *yytop = yycrank+839;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,011 ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'8' ,'8' ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'O' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'X' ,'G' ,'G' ,'[' ,01  ,01  ,01  ,'G' ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'X' ,'G' ,'G' ,'[' ,01  ,'[' ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych;
	struct yywork *yyr;
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
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank){		/* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
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
				yyprevious = YYU(*yylastch);
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
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
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
