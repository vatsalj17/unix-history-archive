/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"tokens.h"
#include	"macdep.h"
#include	"io.h"

st_node		*global_filename;
int		global_lineno;

/*
 *	Mapping from character to lexical class.
 */
#define	NL		0
#define	SPACE		1
#define	ALPHA		2
#define	DOT		3
#define	DIGIT		4
#define	SINGLE		5
#define	DOUBLE		6
#define	CNTRL		7
#define	BAD		8

static char	char_tab[128]	=
{
	SINGLE,	CNTRL,	CNTRL,	CNTRL,	CNTRL,	CNTRL,	CNTRL,	CNTRL,
	CNTRL,	SPACE,	NL,	SPACE,	SPACE,	SPACE,	CNTRL,	CNTRL,
	CNTRL,	CNTRL,	CNTRL,	CNTRL,	CNTRL,	CNTRL,	CNTRL,	CNTRL,
	CNTRL,	CNTRL,	CNTRL,	CNTRL,	CNTRL,	CNTRL,	CNTRL,	CNTRL,

	SPACE,	DOUBLE,	SINGLE,	BAD,	BAD,	SINGLE,	DOUBLE,	SINGLE,
	SINGLE,	SINGLE,	SINGLE,	DOUBLE,	SINGLE,	DOUBLE,	DOT,	DOUBLE,
	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,
	DIGIT,	DIGIT,	SINGLE,	SINGLE,	DOUBLE,	DOUBLE,	DOUBLE,	SINGLE,

	BAD,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,
	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,
	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,
	ALPHA,	ALPHA,	ALPHA,	SINGLE,	BAD,	SINGLE,	SINGLE,	ALPHA,

	BAD,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,
	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,
	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,	ALPHA,
	ALPHA,	ALPHA,	ALPHA,	SINGLE,	DOUBLE,	SINGLE,	SINGLE,	CNTRL,
};

/*
 *	The get() and unget() macros now use local variables for the pointers.
 */
#define	get()		(\
			    lget_ptr == lget_end\
			    ?\
			    (\
				get_fill(),\
				(lget_ptr = get_ptr),\
				(lget_end = get_end),\
				*lget_ptr++\
			    )\
			    :\
			    *lget_ptr++\
			)
#define	unget(c)	(*--lget_ptr = (c))

/*
 *	'buff' points to a buffer whose upper bound is 'buffend'.
 *	If this runs out (big identifiers or strings) then we realloc
 *	some more and update 'buffend' and 'buffp'.
 *
 *	EX_SZ is the size increment.
 */
#define	buffstash(c)	(*(buffp != buffend ? buffp++ : extend_buff()) = (c))

#define	EX_SZ	128

static char	*buff;
static char	*buffend;
static char	*buffp;

/*
 *	Token buffering data.
 */
token		*tok_list;

/*
 *	Lexical character sets.
 */
extern chset	alpha_nums;
extern chset	uppers;
extern chset	octals;
extern chset	decimals;
extern chset	hexadecimals;
extern chset	float_cont;
extern chset	digits_and_signs;

/*
 *	Keyword structure.
 */
typedef	struct
{
    char	*keyw_name;
    tokens	keyw_token;
}
    init_keywd;

/*
 *	Table of keywords and the token to which they belong.
 */
readonly static	init_keywd	keywords[] =
{
    {"auto",		tk_auto},
    {"break",		tk_break},
    {"case",		tk_case},
    {"char",		tk_char},
    {"continue",	tk_continue},
    {"default",		tk_default},
    {"do",		tk_do},
    {"double",		tk_double},
    {"else",		tk_else},
    {"enum",		tk_enum},
    {"extern",		tk_extern},
    {"float",		tk_float},
    {"for",		tk_for},
    {"goto",		tk_goto},
    {"if",		tk_if},
    {"int",		tk_int},
    {"long",		tk_long},
    {"register",	tk_register},
    {"return",		tk_return},
    {"short",		tk_short},
    {"sizeof",		tk_sizeof},
    {"static",		tk_static},
    {"struct",		tk_struct},
    {"switch",		tk_switch},
    {"typedef",		tk_typedef},
    {"union",		tk_union},
    {"unsigned",	tk_unsigned},
    {"void",		tk_void},
    {"while",		tk_while}
};

/*
 *    The static array which contains the keyword identifier structs.
 */
static ident    keyw_ids[nels(keywords)];

/*
 *	Lexical analysis initialisation.  Install the keywords
 *	in the symbol table, allocate buffer and pushback newline.
 */
void
init_lex()
{
    register init_keywd	*p;
    register ident	*keyw_idptr;

    for
    (
	keyw_idptr = keyw_ids, p = keywords;
	p < &keywords[nels(keywords)];
	keyw_idptr++, p++
    )
    {
	find_str(p->keyw_name)->st_idlist = keyw_idptr;
	keyw_idptr->id_value.i = (int)p->keyw_token;
	keyw_idptr->id_name = p->keyw_name;
    }

    buff = chunk(EX_SZ);
    buffend = buff + EX_SZ;

    /*
     *	We look for "\n#" exactly for preprocessor directives.
     */
    *--get_ptr = '\n';
}

/*
 *	Extend the 'buff_stash' buffer.
 */
char	*
extend_buff()
{
    register int	i;

    i = buffend - buff;
    buff = vector(buff, i + EX_SZ, char);
    buffp = buff + i;
    buffend = buffp + EX_SZ;

    return buffp++;
}

/*
 *	Fill the token buffer
 */
token	*
tok_fill()
{
    register int	c;
    register token	*t;
    register char	*lget_ptr;
    register char	*lget_end;
    int			stash;
    token		**tok_end;
    static int		lineno;
    static st_node	*filename;

    if (filename == NULL)
	filename = find_str("<stdin>");

    lget_ptr = get_ptr;
    lget_end = get_end;
    tok_end = &tok_list;
    stash = TOK_STASH;

    c = get();
    t = new_token();

    loop
    {
	switch (char_tab[c])
	{
	case SPACE:
	    c = get();
	    continue;

	case NL:
	    if ((c = get()) == '#')
	    {
		register int	i;

		while ((c = get()) == ' ')
		    ;

		for (i = 0; in(decimals, c); c = get())
		    i = i * 10 + c - '0';

		lineno = i - 1;

		while (c == ' ')
		    c = get();

		for (buffp = buff; c != '\n' && c != EOF_CHAR; c = get())
		{
		    if (c != '"')
			buffstash(c);
		}

		buffstash('\0');

		if (buffp != buff)
		    filename = find_str(buff);
	    }
	    else
		lineno++;

	    continue;

	case ALPHA:
	    {
		register int	i;
		register char	*p;

		i = buffend - buff;
		p = buff;

		loop
		{
		    *p++ = c;
		    c = get();

		    if (!in(alpha_nums, c))
		    {
			*p = '\0';
			break;
		    }

		    if (--i == 0)
		    {
			buffp = p;

			do
			{
			    buffstash(c);
			    c = get();
			}
			while (in(alpha_nums, c));

			buffstash('\0');
			break;
		    }
		}
	    }

	    {
		register ident	*p;

		t->t_value.n = find_str(buff);
		p = t->t_value.n->st_idlist;

		if (p < &keyw_ids[nels(keyw_ids)] && p >= &keyw_ids[0])
		    t->t_what = (tokens)p->id_value.i;
		else
		    t->t_what = tk_name;
	    }

	    break;

	case DOT:
	    c = get();

	    if (!in(decimals, c))
	    {
		t->t_what = tk_dot;
		break;
	    }

	    buffp = buff;
	    goto got_dot;

	case DIGIT:
	    {
		int	allzeros;
		int	base;

		{
		    register setptr	valid_digits;

		    buffp = buff;
		    allzeros = 1;

		    if (c == '0')
		    {
			if ((c = get()) == 'x' || c == 'X')
			{
			    c = get();
			    base = 16;
			    valid_digits = hexadecimals;
			}
			else
			{
			    base = 8;
			    valid_digits = octals;
			}
		    }
		    else
		    {
			base = 10;
			valid_digits = decimals;
		    }

		    while (in(valid_digits, c))
		    {
			if (c != '0')
			    allzeros = 0;

			buffstash(c);
			c = get();
		    }
		}

		if (in(float_cont, c) && (base == 10 || (base == 8 && allzeros)))
		{
		    if (c == '.')
		    {
		got_dot:
			buffstash('.');

			while (c = get(), in(decimals, c))
			    buffstash(c);
		    }

		    if (c == 'e' || c == 'E')
		    {
			register char	d;

			d = get();

			if (!in(digits_and_signs, d))
			    unget(d);
			else
			{
			    buffstash(c);

			    if (d == '-' || d == '+')
			    {
				buffstash(d);
				c = get();
			    }
			    else
				c = d;

			    while (in(decimals, c))
			    {
				buffstash(c);
				c = get();
			    }
			}
		    }

		    buffstash('\0');

		    t->t_what = tk_cdouble;
		    t->t_value.f = atof(buff);
		}
		else
		{
		    register char	*p;
		    register int	i;

		    buffstash('\0');

		    i = 0;

		    if (base <= 10)
		    {
			for (p = buff; *p != '\0'; p++)
			    i = i * base - '0' + *p;
		    }
		    else
		    {

			for (p = buff; *p != '\0'; p++)
			{
			    if (in(decimals, *p))
				i = i * base + *p - '0';
			    else if (in(uppers, *p))
				i = i * base + *p - 'A' + 10;
			    else
				i = i * base + *p - 'a' + 10;
			}
		    }

		    t->t_value.i = i;

		    if (c == 'l' || c == 'L')
		    {
			t->t_what = tk_clong;
			c = get();
		    }
		    else
		    {
			if ((i & MAXINTBITS) != i)
			    t->t_what = tk_clong;
			else
			    t->t_what = tk_cint;
		    }
		}
	    }

	    break;

	case SINGLE:
	    switch (c)
	    {
	    case '%':
		t->t_what = tk_percent;
		break;

	    case '(':
		t->t_what = tk_onround;
		break;

	    case ')':
		t->t_what = tk_offround;
		break;

	    case '*':
		t->t_what = tk_times;
		break;

	    case ',':
		t->t_what = tk_comma;
		break;

	    case ':':
		t->t_what = tk_colon;
		break;

	    case ';':
		t->t_what = tk_semicolon;
		break;

	    case '?':
		t->t_what = tk_question;
		break;

	    case '[':
		t->t_what = tk_onsquare;
		break;

	    case ']':
		t->t_what = tk_offsquare;
		break;

	    case '^':
		t->t_what = tk_uparrow;
		break;

	    case '{':
		t->t_what = tk_oncurly;
		break;

	    case '}':
		t->t_what = tk_offcurly;
		break;

	    case '~':
		t->t_what = tk_tilda;
		break;

	    case EOF_CHAR:
		t->t_what = tk_eof;
		stash = 0;
		goto eof_found;

	    case '"':
	    case '\'':
		{
		    register int	lc;
		    token		start;

		    lc = c;
		    start.t_lineno = lineno;
		    start.t_filename = filename;

		    buffp = buff;
		    
		    while ((c = get()) != '"' || lc == '\'')
		    {
			if (c == '\\')
			{
			    switch (c = get())
			    {
			    case '\n':
				lineno++;
				continue;

			    case 'b':
				c = '\b';
				break;

			    case 'f':
				c = '\f';
				break;

			    case 'n':
				c = '\n';
				break;

			    case 'r':
				c = '\r';
				break;

			    case 't':
				c = '\t';
				break;

			    case 'v':
				c = '\v';
				break;

			    default:
				if (in(octals, c))
				{
				    register int	i;

				    i = c - '0';

				    if (c = get(), in(octals, c))
				    {
					i = i * 8 + c - '0';

					if (c = get(), in(octals, c))
					{
					    i = i * 8 + c - '0';
					    c = get();
					}
				    }

				    unget(c);
				    c = i;
				}
			    }
			}
			else if (c == EOF_CHAR)
			{
			    errort(&start, lc == '"' ? "unterminated string" : "unterminated character constant");
			    break;
			}
			else if (c == '\n')
			{
			    lineno++;
			    errortf(&start, "newline in %", lc == '"' ? "string" : "character constant");
			    break;
			}

			if (lc == '\'')
			{
			    if ((c & CHAR_SIGN) != 0)
				t->t_value.i = CHAR_SXT(c);
			    else
				t->t_value.i = c;

			    t->t_what = tk_cint;

			    if ((c = get()) != '\'')
			    {
				errort(&start, "no matching quote in character constant");

				while (c != '\'' && c != '\n' && c != EOF_CHAR)
				    c = get();

				if (c == '\n')
				    lineno++;
			    }

			    break;
			}

			buffstash(c);
		    }

		    if (lc == '"')
		    {
			buffstash('\0');
			lc = buffp - buff;
			t->t_value.s.str_value = cstr_alloc(buff, lc);
			t->t_value.s.str_size = lc;
			t->t_what = tk_string;
		    }
		}

		break;

	    default:
		internal("tok_fill", "unexpected SINGLE char");
	    }

	    c = get();

	eof_found:
	    break;

	case DOUBLE:
	    {
		register int	d;

		d = get();

		switch (c)
		{
		case '!':
		    if (d == '=')
		    {
			t->t_what = tk_noteq;
			c = get();
		    }
		    else
		    {
			t->t_what = tk_not;
			c = d;
		    }

		    break;

		case '&':
		    if (d == '&')
		    {
			t->t_what = tk_andand;
			c = get();
		    }
		    else
		    {
			t->t_what = tk_and;
			c = d;
		    }

		    break;

		case '+':
		    if (d == '+')
		    {
			t->t_what = tk_plusplus;
			c = get();
		    }
		    else
		    {
			t->t_what = tk_plus;
			c = d;
		    }

		    break;

		case '-':
		    switch (d)
		    {
		    case '-':
			t->t_what = tk_minusminus;
			c = get();
			break;

		    case '>':
			t->t_what = tk_minusgrt;
			c = get();
			break;

		    default:
			t->t_what = tk_minus;
			c = d;
		    }

		    break;

		case '/':
		    switch (d)
		    {
		    case '*':
			{
			    token	start;

			    start.t_lineno = lineno;
			    start.t_filename = filename;

			    c = get();

			    do
			    {
				if (c == EOF_CHAR)
				    goto eof_error;

				while (c != '*')
				{
				    if (c == '\n')
					lineno++;

				    c = get();

				    if (c == EOF_CHAR)
					goto eof_error;
				}

				c = get();
			    }
			    while (c != '/');

			    c = get();
			    continue;

			eof_error:
			    errort(&start, "unterminated comment");
			    stash = 0;
			    t->t_what = tk_eof;
			    break;
			}

		    default:
			t->t_what = tk_slash;
			c = d;
		    }

		    break;

		case '<':
		    switch (d)
		    {
		    case '<':
			t->t_what = tk_lessless;
			c = get();
			break;

		    case '=':
			t->t_what = tk_lesseq;
			c = get();
			break;

		    default:
			t->t_what = tk_less;
			c = d;
		    }

		    break;

		case '=':
		    if (d == '=')
		    {
			t->t_what = tk_eqeq;
			c = get();
		    }
		    else
		    {
			t->t_what = tk_eq;
			c = d;
		    }

		    break;

		case '>':
		    switch (d)
		    {
		    case '>':
			t->t_what = tk_grtgrt;
			c = get();
			break;

		    case '=':
			t->t_what = tk_grteq;
			c = get();
			break;

		    default:
			t->t_what = tk_grt;
			c = d;
		    }

		    break;

		case '|':
		    if (d == '|')
		    {
			t->t_what = tk_barbar;
			c = get();
		    }
		    else
		    {
			t->t_what = tk_bar;
			c = d;
		    }

		    break;

		default:
		    internal("tok_fill", "unexpected DOUBLE character");
		}
	    }

	    break;

	case BAD:
	    {
		char	buff[2];
		token	tok;

		tok.t_lineno = lineno;
		tok.t_filename = filename;
		buff[0] = c;
		buff[1] = '\0';
		errortf(&tok, "ignored unrecognised character '%'", buff);
		c = get();
		continue;
	    }

	case CNTRL:
	    {
		token	tok;

		tok.t_lineno = lineno;
		tok.t_filename = filename;
		errort(&tok, "ignored unrecognised control character");
		c = get();
		continue;
	    }

	default:
	    internal("tok_fill", "unexpected char_tab value");
	}

	t->t_lineno = lineno;
	t->t_filename = filename;
	*tok_end = t;
	tok_end = &t->t_next;

	if (--stash <= 0)
	{
	    unget(c);
	    break;
	}

	t = new_token();
    }

    get_ptr = lget_ptr;
    get_end = lget_end;
    *tok_end = NULL;
    return tok_list;
}
