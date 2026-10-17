#define	in(s, c)	(((s)[((int)c)>>3]) & (1<<(((int)c)&07)))
#define	insert(s, c)	(((s)[((int)c)>>3]) |= (1<<(((int)c)&07)))
#define	delete(s,c)	(((s)[((int)c)>>3]) &= ~(1<<(((int)c)&07)))
#define	loop		for(;;)
#define	STABZ		(32)
#define	MAX_INDEX	(1024)
#define	talloc(x)	((x *)scalloc(sizeof(x)))
#define	get_token	(token)yylex
#define	ENUM_ID		(01)
#define	ENUM_ELEMENT	(02)
#define	ENUM_TYPE	(04)

typedef	enum
{
	sy_eof,
	sy_char,
	sy_comma,
	sy_dot_dot,
	sy_enum,
	sy_id,
	sy_int,
	sy_off_curly,
	sy_off_squares,
	sy_on_curly,
	sy_on_squares,
	sy_set,
	sy_typedef,
}
	token;

typedef	enum
{
	got_eid,
	got_enum,
	got_etypedef,
	got_on_squares,
	got_tenum,
	got_set,
	got_typedef,
	in_source,
	want_setid,
}
	state;

extern int	yylval;
extern int	yylineno;
extern char	*myname;
extern char	yytext[];
