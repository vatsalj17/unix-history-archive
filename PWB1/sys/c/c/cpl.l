%{
# define input getch
# define unput pushback
int ifdef 0;
%}
%%
%{
	char *p, *s;
	struct symtab {
		char name[8];
		char *value;
		} *sp;
%}
"\\\n"	;
"\n"	return(stop);
"=="	return(EQ);
"!="	return(NE);
"<="	return(LE);
">="	return(GE);
"<<"	return(LS);
">>"	return(RS);
"&&"	return(ANDAND);
"||"	return(OROR);
"*"	|
"/"	|
"%"	|
"+"	|
"-"	|
"<"	|
">"	|
"&"	|
"^"	|
"|"	|
"?"	|
":"	|
"!"	|
"~"	|
"("	|
")"	return(yytext[0]);
[ \t]	;
[a-zA-Z0-9_]+	{
	switch(yytext[0])
		{
		case '0':
			yylval = yytext[1]=='x'?
				tobinary(yytext+2, 16) :
				tobinary(yytext+1, 8);
			break;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			yylval = tobinary(yytext,10);
			break;
		default:
			if (strcmp(yytext,"defined")==0)
				{
				ifdef=1;
				return(DEFINED);
				}
			sp = lookup(yytext,-1);
			if (ifdef)
				{
				ifdef=0;
				yylval = (sp->name[0]==0) ? 0 : 1;
				break;
				}
			if (sp->name[0]==0)
				yylval=0;
			else
			if (strcmp(p=sp->value,yytext)==0)
				yylval=1;
			else
				{
				for(s=p; *s; s++);
				while (s>p)
					unput(*--s);
				continue;
				}
		}
	return(number);
	}
.	error("Illegal character %c in preprocessor if", yytext[0]);
%%
tobinary(st, b)
	char *st;
{
int n, c, t;
char *s;
n=0;
s=st;
while (c = *s++)
	{
	switch(c)
		{
		case '0': case '1': case '2': case '3': case '4': 
		case '5': case '6': case '7': case '8': case '9': 
			t = c-'0'; break;
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': 
			t = c-'a'; if (b>10) break;
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': 
			t = c - 'A'; if (b>10) break;
		default:
			t = -1;
			if ( c=='l' || c=='L')
				if (*s==0)
					break;
			error("Illegal number %s", st);
		}
	if (t<0) break;
	n = n*b+t;
	}
return(n);
}
