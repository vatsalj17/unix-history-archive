/* Yacc productions for "expr" command: */

%token A_STRING SUBSTR LENGTH INDEX NOARG

/* operators listed below in increasing precedence: */
%left '|'
%left '&'
%left '+' '-'
%left '*' '/' '%'
%left SUBSTR
%left LENGTH INDEX
%%

/* a single `expression' is evaluated and printed: */

expression:	expr NOARG = { printf("%s\n", $1); exit(0); }
	;


expr:	'(' expr ')' = { $$ = $2; }
	| expr '|' expr   = { $$ = binop('|', $1, $3); }
	| expr '&' expr   = { $$ = binop('&', $1, $3); }
	| expr '+' expr   = { $$ = binop('+', $1, $3); }
	| expr '-' expr   = { $$ = binop('-', $1, $3); }
	| expr '*' expr   = { $$ = binop('*', $1, $3); }
	| expr '/' expr   = { $$ = binop('/', $1, $3); }
	| expr '%' expr   = { $$ = binop('%', $1, $3); }
	| SUBSTR expr expr expr = { $$ = substr($2, $3, $4); }
	| LENGTH expr       = { $$ = length($2); }
	| INDEX expr expr = { $$ = index($2, $3); }
	| A_STRING
	;
%%
/*	expression command - 1.2 of 6/6/76 */
char	**av;
int	ac;
char	tmp[100][6];
int	tmpi;
int	argi;

int yyline 0;

main(argc, argv) char **argv; {
	ac = argc;
	argi = 1;
	av = argv;
	yyinit();
	yyparse();
}
yylex() {
register char *p;

	if(argi >= ac) return NOARG;

	p = av[argi++];

	switch(*p) {
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '&':
		case '|':
		case '(':
		case ')':
			if(*(p+1) == '\0') return *p;
	}

	if(eq(p, "substr")) return SUBSTR;

	if(eq(p, "length")) return LENGTH;
	if(eq(p, "index")) return INDEX;

	yylval = p;
	return A_STRING;
}

binop(op, r1, r2) char op, *r1, *r2; {
register i1, i2;

	i1 = atoi(r1);
	i2 = atoi(r2);

	switch(op) {
		case '+': i1 = i1 + i2; break;
		case '-': i1 = i1 - i2; break;
		case '*': i1 = i1 * i2; break;
		case '/': i1 = i1 / i2; break;
		case '%': i1 = i1 % i2; break;
		case '&': i1 = i1 & i2; break;
		case '|': i1 = i1 | i2; break;
	}
	copy(itoa(i1), tmp[tmpi]);
	return tmp[tmpi++];
}

substr(v, s, w) char *v, *s, *w; {
register si, wi;
register char *res;

	si = atoi(s);
	wi = atoi(w);
	while(--si) if(*v) ++v;

	res = v;

	while(wi--) if(*v) ++v;

	*v = '\0';
	return res;
}

length(s) register char *s; {
register i 0;

	while(*s++) ++i;

	copy(itoa(i), tmp[tmpi]);
	return tmp[tmpi++];
}

index(s, t) char *s, *t; {

register i, j;

	for(i = 0; s[i] ; ++i)
		for(j = 0; t[j] ; ++j)
			if(s[i]==t[j]) {
				copy(itoa(++i), tmp[tmpi]);
				return tmp[tmpi++];
			}
	return "0";
}

eq(a, b)
char *a, *b;
{
	register int i;

	i = 0;
l:
	if(a[i] != b[i])
		return(0);
	if(a[i++] == '\0')
		return(1);
	goto l;
}
atoi(s) register char *s; {

	register i 0, neg 0;


	if(*s == '-') {
		++neg;
		++s;
	}

	while(*s) {
		if(*s < '0' || *s > '9') {
			write(2, "non-numeric argument\n", 21);
			putchar('\n');
			exit(1);
		}
		i = i * 10 + (*s - '0');
		++s;
	}
	return neg? -i: i;
}

itoa(n) register n; {
	register char *cp;
	static char *str[10];
	register neg = 0;

	if(n < 0) {
		neg++;
		n= -n;
	}
	for(cp = &str[8]; ;--cp) {
		*cp = n % 10 + '0';
		n = n / 10;
		if (n == 0) {
			if(neg)
				*--cp = '-';
			return cp;
		}
	}
}
copy(source, sink) register char *source, *sink; {
	 while(*sink++ = *source++);
}
