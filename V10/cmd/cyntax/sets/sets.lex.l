%start	SOURCE COMMENT STRING CHAR DEFN SETDEFN INIT C_DEFN C_SETDEFN C_INIT

%{
#include	"sets.h"

#define	RETURN		return (int)
%}

A	[A-Za-z_]
AN	[A-Za-z_0-9]
X	[0-9A-Fa-f]
D	[0-9]
O	[0-7]
WS	[ \t\n\f]+

%%

<SOURCE>"/*"		{
				ECHO;
				BEGIN COMMENT;
			}

<SOURCE>\"		{
				ECHO;
				BEGIN STRING;
			}

<SOURCE>"'"		{
				ECHO;
				BEGIN CHAR;
			}

<SOURCE>set		{
				BEGIN SETDEFN;
				RETURN sy_set;
			}

<SOURCE>typedef		{
				ECHO;
				BEGIN DEFN;
				RETURN sy_typedef;
			}

<SOURCE>enum		{
				ECHO;
				BEGIN DEFN;
				RETURN sy_enum;
			}

<SOURCE>{A}{AN}*	ECHO;

<SOURCE>"[["		{
				BEGIN INIT;
				RETURN sy_on_squares;
			}

<SOURCE>.		putchar(*yytext);

<STRING>\\\"		ECHO;

<STRING>\"		{
				ECHO;
				BEGIN SOURCE;
			}

<STRING>\\\\		ECHO;

<STRING>.		ECHO;

<CHAR>\\"'"		ECHO;

<CHAR>"'"		{
				ECHO;
				BEGIN SOURCE;
			}

<CHAR>\\\\		ECHO;

<CHAR>.			ECHO;

<COMMENT>"*/"		{
				ECHO;
				BEGIN SOURCE;
			}

<COMMENT>.		ECHO;

<DEFN>"/*"		{
				ECHO;
				BEGIN C_DEFN;
			}

<DEFN>{WS}		ECHO;

<DEFN>enum		{
				ECHO;
				RETURN sy_enum;
			}

<DEFN>set		{
				BEGIN SETDEFN;
				RETURN sy_set;
			}

<DEFN>{A}{AN}*		{
				ECHO;
				RETURN sy_id;
			}

<DEFN>"{"		{
				ECHO;
				RETURN sy_on_curly;
			}

<DEFN>"}"		{
				ECHO;
				RETURN sy_off_curly;
			}

<DEFN>","		{
				ECHO;
				RETURN sy_comma;
			}

<DEFN>.			{
				ECHO;
				fprintf(stderr, "%s: line %d, ignored unknown character (%s) in definition\n", myname, yylineno, print_char());
			}

<C_DEFN>"*/"		{
				ECHO;
				BEGIN DEFN;
			}

<C_DEFN>.		ECHO;

<SETDEFN>"/*"		{
				ECHO;
				BEGIN C_SETDEFN;
			}

<SETDEFN>{WS}		ECHO;

<SETDEFN>char		RETURN sy_char;

<SETDEFN>{A}{AN}*	RETURN sy_id;

<SETDEFN>.		fprintf(stderr, "%s: line %d, ignored unknown character (%s) in set definition\n", myname, yylineno, print_char());

<C_SETDEFN>"*/"		{
				ECHO;
				BEGIN SETDEFN;
			}

<C_SETDEFN>.		ECHO;

<INIT>"/*"		{
				ECHO;
				BEGIN C_INIT;
			}

<INIT>{WS}		ECHO;

<INIT>"]]"		{
				BEGIN SOURCE;
				RETURN sy_off_squares;
			}

<INIT>","		RETURN sy_comma;

<INIT>".."		RETURN sy_dot_dot;

<INIT>{A}{AN}*		RETURN sy_id;

<INIT>0[xX]{X}+		{
				sscanf(yytext + 2, "%x", &yylval);
				RETURN sy_int;
			}

<INIT>0{O}+		{
				sscanf(yytext, "%o", &yylval);
				RETURN sy_int;
			}

<INIT>{D}+		{
				sscanf(yytext, "%d", &yylval);
				RETURN sy_int;
			}

<INIT>'.'		{
				yylval = yytext[1];
				RETURN sy_int;
			}

<INIT>'\\{O}[{O}[{O}]]'	{
				sscanf(yytext + 2, "%o", &yylval);
				RETURN sy_int;
			}

<INIT>'\\.'		{
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

<INIT>.			fprintf(stderr, "%s: line %d, ignored unknown character (%s) in set initialisation\n", myname, yylineno, print_char());

<C_INIT>"*/"		{
				ECHO;
				BEGIN INIT;
			}

<C_INIT>.		ECHO;

%%

begin_source()
{
	BEGIN SOURCE;
}

begin_setdefn()
{
	BEGIN SETDEFN;
}
