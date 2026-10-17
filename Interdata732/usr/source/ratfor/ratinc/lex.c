# include "r.h"

char *keyword []{
	"do", "DO",	/* have to be first */
	"if", "IF",
	"else", "ELSE",
	"for", "FOR",
	"repeat", "REPEAT",
	"until", "UNTIL",
	"while", "WHILE",
	"break", "BREAK",
	"next", "NEXT",
	"define", "DEFINE",
	"include", "INCLUDE",
	0};

#include "y.tab.c"

int keytran[]{
	0, 0,
	XIF, XIF,
	XELSE, XELSE,
	XFOR, XFOR,
	REPEAT, REPEAT,
	UNTIL, UNTIL,
	XWHILE, XWHILE,
	XBREAK, XBREAK,
	NEXT, NEXT,
	XDEFINE, XDEFINE,
	XINCLUDE, XINCLUDE,
	0};

int	svargc;
char	**svargv;
int	infile	0;
int	fd	0;
int	ninclude	0;
int	filestack[10];
int	linect[10];
int	errorflag	0;
#define	MAXNAMES	300

char	*names[MAXNAMES];
char	*nameptr[MAXNAMES];
int	nnames	0;

main(argc,argv) int argc; char **argv; {
	contfld = errorflag = 0;
	if(argc>1 && argv[1][0]=='-'){
		if(argv[1][1]=='6')
			contfld=6;
		argc--;
		argv++;
	}
	svargc = argc;
	svargv = argv;
	filestack[0] = infile = fd = ninclude = linect[0] = 0;
	if(--svargc>0)
		if( (fd = filestack[0] = copen(svargv[++infile],'r')) < 0 ) {
			error("can't open %s", svargv[infile]);
			cexit(1);
		}
	yyparse();
	cexit(errorflag);
}

int	peek	-1;
int	nextchar	'\n';

getc(){
	nextchar = (peek<0) ? gchar(): peek;
	peek = -1;
	return(nextchar);
}

int	gcp	0;
char	gcbuf[300];
int	apos	-1;

gchar(){
	register int c,i,atype,t;
	register char **hp;
	if( c=gcbuf[gcp++] )
		return(c);
   loop:
	for(gcp=0; (c=gcbuf[gcp]=cgetc(fd))!='\0' ; gcp++ ){
		if( gcbuf[0]== '%' ){
			while(putchar(cgetc(fd))!='\n');
			gcp = -1;
			++linect[ninclude];
			continue;
		}
		if( (atype=alphanum(c)) && apos < 0 ){
			apos = gcp;
			continue;
		}
		if( !atype )
			if( apos >= 0 ){
				gcbuf[gcp] = '\0';
				if (nnames>0 && *(hp=dlookup(&gcbuf[apos])) != 0 ) {
					hp = hp - names + nameptr;
					for (i=0; gcbuf[apos++] = hp[0][i]; i++);
					gcp = apos-1;
				}
				apos = -1;
				gcbuf[gcp] = c;
			}
		if( c < ' ' && (c!='\n' && c!='\t') )	/* strip crap */
			c = gcbuf[gcp] = ' ';
		if( c=='#' ){
			gcbuf[gcp] = '\n';
			while( (c=cgetc(fd))!='\n' && c!='\0');
		}
		if( c=='"' || c=='\'' ){
			while( (gcbuf[++gcp]=t=cgetc(fd)) != c )
				if( t=='\n' ) {
					error("unbalanced quote");
					gcbuf[gcp] = c;
					gcbuf[++gcp] = c = '\n';
					goto newline;
				}
			continue;
		}
	newline:
		if( c=='\n' ){
			gcbuf[gcp+1] = '\0';
			gcp = 1;
			++linect[ninclude];
			return(gcbuf[0]);
		}
	}
	if(ninclude){
		cclose(filestack[ninclude--]);
		fd = filestack[ninclude];
		goto loop;
	}
	cclose(filestack[ninclude]);
	if(--svargc>0){
		if( (fd = filestack[ninclude] = copen(svargv[++infile],'r')) < 0) {
			error("can't open %s", svargv[infile]);
			cexit(1);
		}
		linect[0] = 0;
		goto loop;
	}
	return(0);
}

inclstat(){
	register int  i,c;
	static char fname[50]	"/usr/rat/";
	while( (c=getc())==' ' || c=='\t' );
	peek = c;
	for(i=9; (fname[i]=c=getc())!='\n' && c!=';' && c!=' ' && c!='\t'; i++);
	fname[i] = '\0';
	if( (fd = copen(&fname[9],'r')) < 0 ) {
		if( (fd = copen(fname,'r')) < 0 ) {
			error("can't open %s", fname);	
			cexit(1);
		}
	}
		filestack[++ninclude] = fd;
		linect[ninclude] = 0;
}

lookup(string,tbl)
	register char *string;
	register char *tbl[]; 
{
	register i,j, r;
	for(i=0; tbl[i]!=0; i++){ 
		for( j=0; (r=tbl[i][j])==string[j] && r!='\0'; j++);
		if( r == string[j] )
			return(i);
	}
	return( -1 );
}

char	str[200];
yylex(){
	register int c, type;
	char **dlookup();
  top:
	while( (c=getc())==' ' || c=='\n' || c=='\t' );
	yylval = c;
	switch(c){

	case '\0':
		return('\0');
	case ';':
		return(SCOL);
	case'{':
		return(LCURL);
	case '}':
		return(RCURL);
	}
	peek = c;
	getstr(str);
	yylval = &str[0];
	if( alldigits(str) )
		return(DIGITS);
	type = lookup(str,keyword);
	if( keytran[type]==XDEFINE ) {
		defstat();
		goto top;
	} else if( keytran[type]==XINCLUDE ) {
		inclstat();
		goto top;
	} else if( type > 1 )
		return(keytran[type]);
	else if( type < 0 )
		return(XGOK);
	while( (c=getc())==' ' || c=='\t' || c=='\n' );
	peek = c;
	if( c>='a' && c<='z' || c>='A' && c<='Z' )
		return(NEWDO);
	else
		return(OLDDO);
}

getstr(s) register char *s; {
	register int  c, sp;
	for (sp=0; (c=s[sp++]=getc())!=' ' && c!='\t' && c!='\n' && c!='{' && c!='}'
		&& c!=';' && c!='(' && c!=')' ; )
			if( c=='\'' || c=='"' )
				while( (s[sp++]=getc())!=c );
	peek = c;
	s[--sp]='\0';
	return(sp);
}

alldigits(s) register char *s; {
	register int c;
	if( *s == '\0' )
		return(0);
	while( (c = *s++) != '\0' )
		if( c<'0' || c>'9' )
			return(0);
	return(1);
}

int	dbg	0;

yyerror(){;}

alphanum(c) register int c; {
	if(c>='0' && c<='9') return(1);
	if(c>='a' && c<='z') return(1);
	if(c>='A' && c<='Z') return(1);
	return(0);
}


defstat(){
	register int c,i,index;
	register char **hp;
	char *alloc();
	char **dlookup();
	if (++nnames >= MAXNAMES-2 ) {
		error("too many defined names");
		cexit(1);
	}
	while( (c=getc())==' ' || c=='\t' || c=='(' );
	peek = c;
	for(i=0; c=getc(); i++ ){
		if(c==' ' || c=='\t' || c=='\n' || c==',' ) break;
		str[i] = c;
	}
	peek = c;
	str[i] = '\0';
	if ( *(hp = dlookup(str)) == 0 ) {
		*hp = alloc(i+1);
		for (i=0; hp[0][i] = str[i]; i++) ;
	}
	while( (c=getc())==' ' || c=='\t' || c==',' );
	peek = c;
	for( i=0; (c=getc())!='\n' && c!='\0' && c!=')'; i++ )
		str[i] = c;
	str[i] = '\0';
	hp = hp - names + nameptr;
	*hp = alloc(i+1);
	for (i=0; hp[0][i] = str[i]; i++);
}
char **
dlookup(s)
	register char *s;
{
	register int i;
	register char **hp;
	register char *cp,*cp1;

	i=0;
	for (cp=s; *cp;)
		i = (i<<1) + *cp++;
	for (hp =  &names[(i&077777)%MAXNAMES]; *hp!=0;) {
		cp1 = *hp;
		for (cp=s; *cp == *cp1++;)
			if (*cp++ == '\0')
				return(hp);
		if (++hp >= &names[MAXNAMES] )
			hp = names;
	}
	return(hp);
}
