/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"cnodes.h"
#include	"types.h"
#include	"tokens.h"
#include	"io.h"
#include	"stdobj.h"

/*
 *	Buffering.
 *
 *	Input
 *	-----
 *	The source is read into &get_buff[PUSHBACKZ], the next char is
 *	pointed to by get_ptr, the end by get_end.  get() returns the
 *	next char (perhaps calling get_fill() for the next buffer).
 *	unget() allows for guaranteed PUSHBACKZ chars of character
 *	push back.
 */

char	get_buff[BUFFZ + PUSHBACKZ];
char	*get_end		= get_buff + PUSHBACKZ;
char	*get_ptr		= get_buff + PUSHBACKZ;


formarg	Ea[8];
int	in_fid		= STDIN;
int	out_fid		= NO_FID;
int	err_fid		= STDOUT;

long	type_index	= 1;
long	var_index	= 1;

struct
{
	char	hd_cmd[24];
	long	hd_str_off;
	long	hd_str_size;
	long	hd_type_size;
	long	hd_var_size;
}
obj_header = 
{
	"#!/bin/echo 3r\n"
};

int	errored;

#define	err(c)	Fputc(1, (c))

void exit();

get_fill()
{
    register int	i;
    register char	*p;
    static int		eof_seen	= 0;

    get_ptr = get_buff + PUSHBACKZ;

    if (eof_seen)
    {
	get_buff[PUSHBACKZ] = EOF_CHAR;
	get_end = get_ptr + 1;
	return;
    }

    switch (i = read(in_fid, get_buff + PUSHBACKZ, BUFFZ))
    {
    case SYSERROR:
	fatal("read error");

    case 0:
	eof_seen = 1;
	get_buff[PUSHBACKZ] = EOF_CHAR;
	get_end = get_ptr + 1;
	break;

    default:
	get_end = get_ptr + i;

	p = get_buff + PUSHBACKZ;

	while (--i >= 0)
	{
	    if ((*p &= 0x7F) == EOF_CHAR)
		*p = '\177';

	    p++;
	}
    }
}

void
put_varargs(p)
char	*p;
{
	register char	*q;
	register ident	*id;
	extern int	atoi();

	for (q = p; *q != '\0'; q++)
	{
		if (*q == ':')
		{
			*q++ = '\0';
			break;
		}
	}

	if ((id = refind_ident(find_str(p), ID_FACTOR, (ident *)NULL)) == NULL)
		return;

	if (id->id_type == NULL)
		warnf("varargs function '%' undefined", p);
	else if (id->id_type->x_what != xt_ftnreturning)
	{
		Ea[0].m = p;
		Ea[1].x = id->id_type;
		warn("varargs function '%' is #");
	}
	else if ((id->id_flags & IDIS_INIT) != 0)
	{
		put(obj_code(i_var, v_varargs));
		putnum( id->id_index );
		putnum( *q ? atoi(q) : 0 );
	}
}

void
put_src(lib, src)
char	*lib;
char	*src;
{

	global_filename = find_str(src);

	if (lib == NULL)
	{
		put(obj_code(i_src, 0));
		putnum( global_filename->st_index );
	}
	else
	{
		put(obj_code(i_lib, 0));
		putnum( find_str(lib)->st_index );
	}
}



void
init_obj()
{
    Fwrite(out_fid, (char*)&obj_header, (long)sizeof(obj_header) );
}

void
end_obj()
{
    extern long	dump_strings();
    extern long	lseek();

    if (out_fid == NO_FID)
	return;

    obj_header.hd_str_off = Fseek(out_fid, 0L, 2);
    obj_header.hd_str_size = dump_strings();
    obj_header.hd_type_size = type_index;
    obj_header.hd_var_size = var_index;

    if (Fseek(out_fid, 0L, 0) == SYSERROR)
	fatal("lseek error");

    if (Fwrite(out_fid, (char *)&obj_header, (long)sizeof (obj_header)) == SYSERROR)
	fatal("write error");
    Fflush(out_fid);
}


err_file(p, f1)
register char	**p;
{
    register char *s, *t, c;

    if(p==0 || *p == 0) s = "(null)";
    else s = *p;
    for(t= s; c = *t; t++ )
	if(c == '@') break;
    strconv(s,f1,t-s);
    return sizeof(s);
}

#define err_str(ss) Fprint(1, "%s", (ss))


/*
 *	Possible values of 'severity'.
 */
#define	WARNING		0
#define	ERROR		1
#define	FATAL		2

/*
 *	fmt contains the following escapes:
 *
 *	? means next char verbatim
 *	% means string in Ea[].m
 *	# means type in Ea[].x (plural if followed by s)
 *	! means type in Ea[].x with array dimensions (plural if followed by s)
 *	$ means type in Ea[].x->x_type (plural if followed by s) and
 *		if x->x_what == xt_name
 *			name in x->x_value.d->id_name
 *	@ means (possibly NULL) token
 *	= means integer in Ea[].i
 *	/ means where_name in Ea[].i
 */
void
message(tok, severity, fmt)
token		*tok;
int		severity;
register char	*fmt;
{
    register long	lineno;
    register char	*filename;
    register formarg	*e;
    register token *t;
    extern char		*myname;
    extern char		*where_name[];

    e = Ea;

    if (tok == NULL)
    {
	lineno = global_lineno;
	filename = global_filename->st_name;
    }
    else
    {
	lineno = tok->t_lineno;
	filename = tok->t_filename->st_name;
    }
    if (lineno == 0)
	Fprint(1, "%s: ", myname);
    else if(lineno>0)
	Fprint(1, "%@: %d: ", filename, lineno);
    else
	Fprint(1, "%@: ", filename);

    if (severity == WARNING)
	err_str("warning: ");

    while (*fmt != '\0')
    {
	switch (*fmt)
	{
	case '%':
	    err_str(e->m?e->m:"");
	    e++;
	    break;

	case '/':
	    err_str(where_name[e->i]);
	    e++;
	    break;

	case '$':
	    if (fmt[1] == 's')
	    {
		fmt++;
		err_type(e->x->x_type, 1, 0);
	    }
	    else
		err_type(e->x->x_type, 0, 0);

	    if (e->x->x_what == xt_name)
		Fprint(1, " (%s)", e->x->x_value.d->id_name);

	    e++;
	    break;

	case '!':
	    if (fmt[1] == 's')
	    {
		fmt++;
		err_type(e->x, 1, 1);
	    }
	    else
		err_type(e->x, 0, 1);

	    e++;
	    break;

	case '#':
	    if (fmt[1] == 's')
	    {
		fmt++;
		err_type(e->x, 1, 0);
	    }
	    else
		err_type(e->x, 0, 0);

	    e++;
	    break;

	case '@':
	    if ((t=e->t) != NULL)
	    {
		if (t->t_filename->st_name != filename)
		{
		    Fprint(1, " (%@: %d)", t->t_filename->st_name, t->t_lineno);
		}
		else
		    Fprint(1, " (line %d)", t->t_lineno);
	    }

	    e++;
	    break;

	case '=':
	    Fprint(1, "%d", e->i);
	    e++;
	    break;

	case '?':
	    fmt++;
	    
	default:
	    err(*fmt);
	}

	fmt++;
    }

    err('\n');
}

/*
 *	f is a format, args in Ea
 */
error(f)
char	*f;
{
    message((token *)NULL, ERROR, f);
    errored++;
}

/*
 *	as error but with token
 */
errort(t, f)
token	*t;
char	*f;
{
    message(t, ERROR, f);
    errored++;
}

/*
 *	short hand, one string
 */
warnf(f, s)
char	*f;
char	*s;
{
    Ea[0].m = s;
    message((token *)NULL, WARNING, f);
}

/*
 *	short hand, one string
 */
errorf(f, s)
char	*f;
char	*s;
{
    Ea[0].m = s;
    message((token *)NULL, ERROR, f);
    errored++;
}

/*
 *	same with token
 */
errortf(t, f, s)
token	*t;
char	*f;
char	*s;
{
    Ea[0].m = s;
    message(t, ERROR, f);
    errored++;
}

warn(m)
char	*m;
{
    message((token *)NULL, WARNING, m);
}

warnt(t, m)
token	*t;
char	*m;
{
    message(t, WARNING, m);
}

fatal(m)
char	*m;
{
    message((token *)NULL, FATAL, m);
    exit(1);
}

internal(r, d)
char	*r;
char	*d;
{
    Ea[0].m = r;
    Ea[1].m = d;
    message((token *)NULL, FATAL, "internal error in routine % - %");
    exit(1);
}

err_type(x, plural, dims)
register xnode	*x;
register int	plural;
int		dims;
{
    register int	spoken;
    static char		*type_names[] =
    {
	"unsigned",
	"long",
	"char",
	"short",
	"int",
	"float",
	"void",
	"<bad type 7>",
	"<bad type 8>",
	"<bad type 9>",
	"<bad type 10>",
	"<bad type 11>",
	"<bad type 12>",
	"<bad type 13>",
	"<bad type 14>",
	"<bad type 15>",
	"<bad type 16>",
	"<bad type 17>",
	"<bad type 18>",
	"<bad type 19>",
    };

    spoken = 0;

    while (x != NULL)
    {
	if (spoken)
	    err(' ');

	switch (x->x_what)
	{
	case xt_arrayof:
	    if (dims && x->x_value.i >= 0)
		Fprint(1, "array[%d]%s of", x->x_value.i, plural?"s":"");
	    else
		Fprint(1, "array%s of", plural?"s":"");

	    plural = 1;
	    break;

	case xt_ftnreturning:
	    Fprint(1, "function%s returning", plural?"s":"");
	    break;

	case xt_ptrto:
	    Fprint(1, "pointer%s to", plural?"s":"");
	    break;

	case xt_enum:
	case xt_structof:
	case xt_unionof:
	    if (x->x_value.d->id_flags & IDIS_FAKE)
	    {
		if (x->x_value.d==0 || *x->x_value.d->id_name == '\0')
		{
		    err_str("nameless ");

		    switch (x->x_what)
		    {
		    case xt_enum:
			err_str("enum");
			break;

		    case xt_structof:
			err_str("struct");
			break;

		    case xt_unionof:
			err_str("union");
		    }
		}
		else
		{
		    Fprint(1, "type %s", x->x_value.d->id_name);
		}
	    }
	    else
	    {
		switch (x->x_what)
		{
		case xt_enum:
		    err_str("enum");
		    break;

		case xt_structof:
		    err_str("struct");
		    break;

		case xt_unionof:
		    err_str("union");
		}

		Fprint(1, " %s", x->x_value.d->id_name);
	    }
	    break;

	case xt_basetype:
	{
	    register int	bit;
	    register int	my_type;

	    my_type = x->x_value.i;

	    if ((my_type & (LONG | FLOAT)) == (LONG | FLOAT))
	    {
		err_str("double");
		my_type &= ~(LONG | FLOAT);
	    }
	    else if ((my_type & (LONG | SHORT)) != 0)
		my_type &= ~INT;

	    spoken = 0;
	
	    for (bit = 0; my_type != 0; bit++, my_type >>= 1)
	    {
		if ((my_type & 1) != 0)
		{
		    if (spoken)
		    {
			err(' ');
		    }

		    spoken++;
		    err_str(type_names[bit]);
		}
	    }

	    if (plural)
		err('s');

	    return;
	}

	case xt_bitfield:
	    Fprint(1, "bitfield%s of", plural?"s":"");
	    break;

	default:
	    Fprint(1, "unknown%s", plural?"s":"");
	}

	x = x->x_subtype;
	spoken = 1;
    }
}

