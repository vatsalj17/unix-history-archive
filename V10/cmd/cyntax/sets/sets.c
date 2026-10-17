#include	"local-system"
#include	<stdio.h>
#include	<ctype.h>
#include	"sets.h"

typedef struct _symbol	symbol;
struct	_symbol
{
	int	value;
	int	range;
	short	flags;
	symbol	*great;
	symbol	*less;
	char	*name;
};

char	*myname;
FILE	*dump_f;
FILE	*type_f;
int	yylval;
int	want_table;
state	s	= in_source;
token	t;
char	iset[(MAX_INDEX / 8) + 1];

extern char	*malloc();
extern char	*strcpy();
extern char	*strrchr();

char	*
salloc(n)
register int	n;
{
	register char	*p;

	if ((p = malloc(n)) != NULL)
		return p;
	fprintf(stderr, "%s: ran out of memory\n", myname);
	exit(1);
}

char	*
scalloc(n)
register int	n;
{
	register char	*p;

	if ((p = malloc(n)) == NULL)
		fprintf(stderr, "%s: ran out of memory\n", myname);
	while (n--)
		p[n] = '\0';
	return p;
}

/*
 *	Find symbol with name s.
 */
symbol	*
find(s)
register char	*s;
{
	register symbol	**n;
	register int	i;
	static symbol	*stab[STABZ];

	{
		register char	*p;

		for (p = s, i = 0; *p != '\0'; i += i ^ *p++)
			;
		n = &stab[i & (STABZ - 1)];
	}
	while (*n != NULL)
	{
		register int	i;

		if ((i = strcmp(s, (*n)->name)) == 0)
			return *n;
		n = i < 0 ? &((*n)->less) : &((*n)->great);
	}
	(*n = talloc(symbol))->name = strcpy(salloc(strlen(s) + 1), s);
	(*n)->flags = 0;
	(*n)->range = 0;
	return *n;
}

gather_enum()
{
	int	value	= 0;
	symbol	*id;

	while ((t = get_token()) == sy_id || t == sy_comma)
	{
		if (t != sy_comma)
		{
			if (want_table)
			{
				fprintf
				(
					dump_f,
					"%s%c%c%c",
					yytext,
					'\0',
					(value >> 8) & 0xFF,
					value & 0xFF
				);
			}
			id = find(yytext);
			id->flags |= ENUM_ELEMENT;
			id->value = value++;
		}
	}
	return t == sy_off_curly ? value - 1 : 0;
}

id_to_yylval()
{
	register symbol	*id;

	if (!((id = find(yytext))->flags & ENUM_ELEMENT))
		fprintf(stderr, "%s: line %d, '%s' is not a member of any known enum\n", myname, yylineno, id->name);
	yylval = id->value;
}

read_enum(silent)
int	silent;
{
	register int	i;
	register int	c;
	register FILE	*f;
	register symbol	*p;
	char		buff[BUFSIZ];

	if ((f = fopen("enum_tab", "r")) == NULL)
	{
		if (silent)
			goto fpd;
		fprintf(stderr, "%s: cannot open `enum_tab'.\n", myname);
		exit(1);
	}

	while ((c = getc(f)) != EOF)
	{
		i = 0;
		do
		{
			if (i < BUFSIZ - 1)
				buff[i++] = c;
			c = getc(f);
		}
		while (c != '\0' && c != EOF);

		buff[i] = '\0';

		if
		(
			c == EOF
			||
			(c = getc(f)) == EOF
			||
			(i = getc(f)) == EOF
		)
		{
			fprintf(stderr, "%s: bad `enum_tab'.\n", myname);
			exit(1);
		}

		if ((p = find(buff))->flags != 0)
		{
			fprintf(stderr, "%s: redefinition of %s in  `enum_tab'.\n", myname, buff);
			exit(1);
		}

		i &= 0xFF;
		p->value = i | ((c & 0xFF) << 8);
		p->flags = ENUM_ELEMENT;
	}

	fclose(f);

fpd:
	if ((f = fopen("type_file", "r")) == NULL)
	{
		if (silent)
			return;
		fprintf(stderr, "%s: cannot open `type_file'.\n", myname);
		exit(1);
	}

	while ((c = getc(f)) != EOF)
	{
		i = 0;
		do
		{
			if (i < BUFSIZ - 1)
				buff[i++] = c;
			c = getc(f);
		}
		while (c != '\0' && c != EOF);

		buff[i] = '\0';

		if
		(
			c == EOF
			||
			(c = getc(f)) == EOF
			||
			(i = getc(f)) == EOF
		)
		{
			fprintf(stderr, "%s: bad `type_file'.\n", myname);
			exit(1);
		}

		p = find(buff);

		i &= 0xFF;
		p->range = i | ((c & 0xFF) << 8);
		p->flags |= ENUM_TYPE;
	}

	fclose(f);
}

gather_set()
{
	register int	largest_index	= 0;
	register int	from;
	register int	i;

	if (t != sy_int && t != sy_id)
		return 0;
	for (i = 0; i < (MAX_INDEX / 8) + 1; i++)
		iset[i] = 0;
	do
	{
		if (t == sy_id)
			id_to_yylval();
		from = yylval;
		if ((t = get_token()) == sy_dot_dot)
		{
			if ((t = get_token()) != sy_int && t != sy_id)
			{
				fprintf(stderr, "%s: line %d, constant expected after '..'\n", myname, yylineno);
				return 0;
			}
			if (t == sy_id)
				id_to_yylval();
			t = get_token();
		}
		if (!(t == sy_comma || t == sy_off_squares))
			fprintf(stderr, "%s: line %d, ',' or ']]' expected\n", myname, yylineno);
		if (yylval > MAX_INDEX)
		{
			fprintf(stderr, "%s: line %d, set element to large\n", myname, yylineno);
			return 0;
		}
		if (from > yylval)
		{
			fprintf(stderr, "%s: line %d, left operand of '..' greater than right\n", myname, yylineno);
			return 0;
		}
		while (from <= yylval)
		{
			insert(iset, from);
			from++;
		}
		if (yylval > largest_index)
			largest_index = yylval;
	} while (t != sy_off_squares && ((t = get_token()) == sy_int || t == sy_id));
	return t == sy_off_squares ? largest_index + 1 : 0;
}

main(argc, argv)
int	argc;
char	*argv[];
{
	register symbol	*enum_id;
	register int	i;

	if ((myname = strrchr(argv[0], '/')) == NULL || *++myname == '\0')
		myname = argv[0];

	switch (argc)
	{
	case 3:
		if (strcmp(argv[2], "-") != 0)
		{
			if (freopen(argv[2], "w", stdout) == NULL)
			{
				fprintf(stderr, "%s: Cannot open %s.\n", myname, argv[2]);
				return 1;
			}
		}
	case 2:
		if (strcmp(argv[1], "-") != 0)
		{
			if (freopen(argv[1], "r", stdin) == NULL)
			{
				fprintf(stderr, "%s: Cannot open %s.\n", myname, argv[1]);
				return 1;
			}
		}
	case 1:
		break;

	default:
		fprintf(stderr, "usage: %s [infile [outfile]]\n", myname);
		return 1;
	}

	if (strcmp(myname, "set_expand") == 0)
		read_enum(0);
	else if (strcmp(myname, "dump_enum") == 0)
	{
		read_enum(1);
		want_table = 1;
		if ((i = dup(fileno(stdout))) == SYSERROR)
		{
			fprintf(stderr, "%s: Cannot dup stdout.\n", myname);
			return 1;
		}
		if (freopen("/dev/null", "w", stdout) == NULL)
		{
			fprintf(stderr, "%s: Cannot open %s.\n", myname, "/dev/null");
			return 1;
		}
		if ((dump_f = fdopen(i, "w")) == NULL)
		{
			fprintf(stderr, "%s: Cannot reopen stdout.\n", myname);
			return 1;
		}
		if ((type_f = fopen("type_file", "a")) == NULL)
		{
			fprintf(stderr, "%s: Cannot open %s.\n", myname, "type_file");
			return 1;
		}
	}

	begin_source();

	while ((t = get_token()) != sy_eof)
	{
		switch (s)
		{
		case in_source:
			switch (t)
			{
			case sy_set:
				s = got_set;
				continue;

			case sy_typedef:
				s = got_typedef;
				continue;

			case sy_enum:
				s = got_enum;
				continue;

			case sy_on_squares:
				s = got_on_squares;
				continue;
			}

		case got_set:
			switch (t)
			{
			case sy_char:
				i = 127;
				s = want_setid;
				continue;

			case sy_id:
				if ((enum_id = find(yytext))->flags & ENUM_TYPE)
				{
					i = enum_id->range;
					s = want_setid;
					continue;
				}
				fprintf(stderr, "%s: line %d, %s is not an enum\n", myname, yylineno, enum_id->name);
				s = in_source;
				begin_source();
				continue;

			case sy_enum:
/*				not_yet();*/
				s = in_source;
				begin_source();
				continue;

			default:
				fprintf(stderr, "%s: line %d, bad set declaration\n", myname, yylineno);
				continue;
			}

		case want_setid:
			if (t == sy_id)
				printf("char\t%s[%d]", yytext, (i / 8) + 1);
			s = in_source;
			begin_source();
			continue;

		case got_typedef:
			if (t == sy_set)
			{
				s = got_set;
				begin_setdefn();
			}
			else if (t == sy_enum)
				s = got_tenum;
			else
			{
				s = in_source;
				begin_source();
			}
			continue;

		case got_tenum:
			if (t == sy_id)
			{
				enum_id = find(yytext);
				enum_id->flags |= ENUM_ID;
				s = got_etypedef;
				continue;
			}
			if (t == sy_on_curly && (i = gather_enum()) != 0)
			{
				enum_id = NULL;
				s = got_etypedef;
				continue;
			}
			s = in_source;
			begin_source();
			continue;

		case got_etypedef:
			if (t == sy_id)
			{
				if (enum_id != NULL)
					enum_id->range = i;
				enum_id = find(yytext);
				enum_id->flags |= ENUM_TYPE;
				enum_id->range = i;
				if (want_table)
				{
					fprintf
					(
						type_f,
						"%s%c%c%c",
						yytext,
						'\0',
						(i >> 8) & 0xFF,
						i & 0xFF
					);
				}
			}
			s = in_source;
			begin_source();
			continue;

		case got_enum:
			if (t != sy_id)
			{
				s = in_source;
				begin_source();
				continue;
			}
			enum_id = find(yytext);
			s = got_eid;
			continue;

		case got_eid:
			if (t == sy_on_curly && (i = gather_enum()) != 0)
			{
				enum_id->range = i;
				enum_id->flags |= ENUM_ID;
			}
			s = in_source;
			begin_source();
			continue;

		case got_on_squares:
			if ((i = gather_set()) != 0)
			{
				register int	j;

				i = (i + 7) / 8;

				printf("{");
				for (j = 0; j < i; j++)
					printf("0x%X, ", 0xFF & (int)iset[j]);
				printf("}");
			}
			s = in_source;
			begin_source();
			continue;

		default:
			fprintf(stderr, "%s: unrecognised state %d\n", myname, s);
			exit(1);
		}
	}
}

char	*
print_char()
{
	register int	c;
	static char	buff[3];

	c = *yytext & 0x7F;

	if (c < ' ' || c == 0x3F)
	{
		buff[0] = '^';
		buff[1] = c ^ 0x40;
		buff[2] = '\0';
	}
	else
	{
		buff[0] = c;
		buff[1] = '\0';
	}
	return buff;
}
