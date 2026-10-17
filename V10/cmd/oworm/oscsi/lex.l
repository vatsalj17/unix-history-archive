%{
#undef	ECHO
#include	"x.tab.h"
#include	<string.h>
#undef	getc
#define	getc(x)		Fgetc(0)
#undef	output
#define	output(x)	(void)(x)
%}
%%
a		return(SIDEA);
all		return(ALL);
b		return(SIDEB);
capacity	return(CAPACITY);
copy		return(COPY);
diag		return(DIAGNOSTIC);
diagnostic	return(DIAGNOSTIC);
disk		return(DISK);
echo		return(ECHO);
eject		return(EJECT);
err		return(ERROR);
error		return(ERROR);
ext		return(EXT);
help		return(HELP);
id		return(ID);
inq		return(INQUIRY);
inquiry		return(INQUIRY);
media		return(MEDIA);
read		return(READ);
rel		return(RELEASE);
release		return(RELEASE);
reset		return(RESET);
sense		return(SENSE);
set		return(SET);
sleep		return(SLEEP);
start		return(START);
status		return(STATUS);
stop		return(STOP);
worm		return(WORM);
write		return(WRITE);
[0123456789]+	{ yylval.num = atoi(yytext); return(NUMBER); }
\n		{ yylineno++; return('\n'); }
\"[^"]*\"	{ yytext[yyleng-1] = 0; yylval.str = strdup(yytext+1); return(STRING); }
#.*		{ ; /* comment */ }
[ \t]+		{ ; /* white space */ }
^!.*		{ system(yytext+1); print("!\n"); }
.		{ return(yytext[0]); }

%%
