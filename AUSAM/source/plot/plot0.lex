D	[0-9]
E	[Ee][-+]?{D}+
DELIM	[ \n\t,]
SP	[\t ,]
%%
{DELIM}+		;


[-+]?{D}+/[\n\t *,]			|
[-+]?{D}+"."{D}*({E})?/[\n\t ,*]	|
[-+]?{D}*"."{D}+({E})?/[\t \n*,]	|
[-+]?{D}+{E}/[\t \n*,]			{
						/* THIS RECOGNISES NUMBERS	*/
						p_token = yytext;
						f_token = atof(yytext);
						return( NUMBER );
					}

\"[^"\n]*\"/\"[^\n]			{
						/* THIS IS A STRING WITH THE CLOSING	*/
						/* FOLLOWED BY ANOTHER QUOTE. IT IS THUS ONLY	*/
						/* PART OF A STRING AND YYMORE() TACKS THE NEXT	*/
						/* TOKEN ON TO THE END OF THIS ONE. (WE KNOW THE	*/
						/* NEXT ONE SHOULD BE A STRING)			/*
						yyleng--;
						/* "" TURNS TO " SO WE BACK UP TO RID US OF THE EXTRA ONE	*/
						yymore();
					}

\"[^"\n]*\"/[^"]			{
						/* THIS IS A NORMAL STRING (NO "" )	*/
						p_token = yytext;
						f_token = 0.0;
						p_token++;
						yytext[yyleng-1] = 0;
						return( STRING );
					}

leg(end)?/{DELIM}			{
						p_token = yytext;
						f_token = 0.0;
						return( LEGEND );
					}

xaxis/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( XAXIS );
					}

yaxis/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( YAXIS );
					}


date/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( DATE );
					}


data/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( DATA );
					}

next/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return(NEXT);
					}


plot/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( PLOT );
					}

mark/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( MARK );
					}

key[\t ]*at/{DELIM}			{
						p_token = yytext;
						f_token = 0.0;
						return( KEY_AT );
					}

end/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( END );
					}

from/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( FROM );
					}

to/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( TO );
					}

at/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( AT );
					}

by/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( BY );
					}

form(at)?/{DELIM}			{
						p_token = yytext;
						f_token = 0.0;
						return(FORMAT);
					}
point[\t ]*=				{
						p_token = yytext;
						f_token = 0.0;
						return( POINT_EQ );
					}

line[\t ]*=				{
						p_token = yytext;
						f_token = 0.0;
						return( LINE_EQ );
					}

regress(ion)?/{DELIM}			{
						p_token = yytext;
						f_token = 0.0;
						return( REGRESSION );
					}
x[\t ]*=				{
						p_token = yytext;
						f_token = 0.0;
						return( X_EQ );
					}

y[\t ]*=				{
						p_token = yytext;
						f_token = 0.0;
						return( Y_EQ );
					}

"*"					{
						p_token = yytext;
						f_token = 0.0;
						return( TIMES );
					}

cross(es)?/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( CROSS );
					}

dot(s)?/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( DOT );
					}

circ(le)?(s)?/{DELIM}			{
						p_token = yytext;
						f_token = 0.0;
						return( CIRCLE );
					}

star(s)?/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( STAR );
					}

solid/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( SOLID );
					}

short/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( SHORT );
					}

long/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( LONG );
					}

short[\t -]*long/{DELIM}			|
long[\t -]*short/{DELIM}			{
						p_token = yytext;
						f_token = 0.0;
						return( SHORT_LONG );
					}

no(thing)?/{DELIM}			|
nil/{DELIM}				{
						p_token = yytext;
						f_token = 0.0;
						return( NOTHING );
					}
!.*$					|
com(ment)?.*$				|
^[\t ]*c[\t ]+.*$			;


.					{
						/*
						** I DID HAVE A MORE SOPHISTICATED
						** ERROR RECOVERY TECHNIQUE USING
						** LEX BUT IT MADE THE TABLES
						** VERY LARGE, SO I USE THIS METHOD INSTEAD
						*/
						p_token = yytext+1;
						while( ((*p_token = input()) != ' ') && (*p_token != '\t') && (*p_token != '\n'))
							if(*p_token++ < ' ')
							{
								p_token = yytext;
								return(BAD_CHAR);
							}
							/*
							** This code is here
							** so that a filename may
							** optionally not contain quotes
							*/
							if(*p_token == '\n')
							{
								unput('\n');
							}
							*p_token = 0;
							p_token = yytext;
							return(UNRECOGNISED);
					}
%%
