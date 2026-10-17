#include	<sys/types.h>
#include	<sys/stat.h>
#include	<errno.h>
#include	<signal.h>
#include	<stdio.h>

/*
 *	Compiler front end program.
 *
 *	Bruce Ellis	- September 1984.
 */

/*
 *	Argument list structures.
 */
typedef struct arg	arg;

struct arg
{
	char	*a_str;
	arg	*a_next;
};

typedef struct
{
	arg	*a_head;
	arg	**a_tail;
}
	args;

/*
 *	Option structures.
 */
typedef enum
{
	c_bad,
	c_flag,
	c_function,
	c_str_func,
	c_string,
}
	opcode;

typedef struct
{
	opcode	o_code;
	args	**o_args;
	int	*o_flag;
	int	(*o_func)();
}
	option;

/*
 *	Exec argument structures.
 */
typedef enum
{
	e_args,
	e_input,
	e_literal,
	e_output,
	e_splice,
	e_string,
}
	eatype;

typedef struct earg	earg;

struct earg
{
	eatype	e_code;
	char	*e_str;
	char	**e_strp;
	args	**e_args;
	int	e_count;
	earg	*e_next;
};

typedef struct
{
	char	*e_path;
	earg	*e_list;
}
	eargs;

args		*files;
char		**tempv;
char		*input;
char		*my_name;
char		*output;
char		*object;
int		ntemps;
int		verbose;

extern int	errno;
extern char	*strcpy();
extern char	*strrchr();
extern int	strlen();

/*
 *	Cyntax.
 */
args		*comp_args;
args		*cpp_args;
args		*load_args;
char		*load_out;
char		*src_name;
int		cflag;
int		no_op;
int		hflag;
int		ken;
int		jerq;
int		gnot;
int		nolib;
int		multi_cfile;

/*
 *	Path names.
 */
#define	CPP_PATH	"/lib/cpp"
#define	CPP_NAME	"cpp"
#define	COMP_PATH	"/usr/lib/cyntax/ccom"
#define	COMP_NAME	"ccom"
#define	LOAD_PATH	"/usr/lib/cyntax/cem"
#define	LOAD_NAME	"cem"

#define	DEF_OUT		"a.out"
#define	DEF_LIB		"-lc"
#define	KEN_INCLUDE	"-I/usr/ninclude"
#define	KEN_LIB		"-lk"
#define	JERQ_INCLUDE	"-I/usr/jerq/include"
#define	JERQ_LIB	"-lj"
#define	GNOT_INCLUDE	"-I/usr/2include"
#define	GNOT_LIB	"-lg"
#define	DEF_MUX		"-DMUX"
#define	TMP_DIR		"/tmp"

extern int	dflag();
extern int	kflag();
extern int	jflag();
extern int	ugflag();
extern int	mflag();
extern int	wflag();
extern int	ostring();

/*
 *	Lower case option table.
 */
option	lcase[26]	=
{
/* a */	{c_bad,		NULL,		NULL,		NULL},	
/* b */	{c_bad,		NULL,		NULL,		NULL},	
/* c */	{c_flag,	NULL,		&cflag,		NULL},	
/* d */	{c_function,	NULL,		NULL,		dflag},	
/* e */	{c_bad,		NULL,		NULL,		NULL},	
/* f */	{c_bad,		NULL,		NULL,		NULL},	
/* g */	{c_flag,	NULL,		&no_op,		NULL},	
/* h */	{c_flag,	NULL,		&hflag,		NULL},	
/* i */	{c_bad,		NULL,		NULL,		NULL},	
/* j */	{c_function,	NULL,		NULL,		jflag},	
/* k */	{c_function,	NULL,		NULL,		kflag},	
/* l */	{c_string,	&files,		NULL,		NULL},	
/* m */	{c_function,	NULL,		NULL,		mflag},	
/* n */	{c_flag,	NULL,		&nolib,		NULL},	
/* o */	{c_str_func,	NULL,		NULL,		ostring},	
/* p */	{c_flag,	NULL,		&no_op,		NULL},	
/* q */	{c_bad,		NULL,		NULL,		NULL},	
/* r */	{c_bad,		NULL,		NULL,		NULL},	
/* s */	{c_flag,	NULL,		&no_op,		NULL},	
/* t */	{c_bad,		NULL,		NULL,		NULL},	
/* u */	{c_bad,		NULL,		NULL,		NULL},	
/* v */	{c_flag,	NULL,		&verbose,	NULL},	
/* w */	{c_function,	NULL,		NULL,		wflag},	
/* x */	{c_bad,		NULL,		NULL,		NULL},	
/* y */	{c_bad,		NULL,		NULL,		NULL},	
/* z */	{c_bad,		NULL,		NULL,		NULL},	
};

/*
 *	Upper case option table.
 */
option	ucase[26]	=
{
/* A */	{c_bad,		NULL,		NULL,		NULL},	
/* B */	{c_bad,		NULL,		NULL,		NULL},	
/* C */	{c_bad,		NULL,		NULL,		NULL},	
/* D */	{c_string,	&cpp_args,	NULL,		NULL},	
/* E */	{c_bad,		NULL,		NULL,		NULL},	
/* F */	{c_bad,		NULL,		NULL,		NULL},	
/* G */	{c_function,	NULL,		NULL,		ugflag},	
/* H */	{c_bad,		NULL,		NULL,		NULL},	
/* I */	{c_string,	&cpp_args,	NULL,		NULL},	
/* J */	{c_bad,		NULL,		NULL,		NULL},	
/* K */	{c_bad,		NULL,		NULL,		NULL},	
/* L */	{c_bad,		NULL,		NULL,		NULL},	
/* M */	{c_bad,		NULL,		NULL,		NULL},	
/* N */	{c_bad,		NULL,		NULL,		NULL},	
/* O */	{c_flag,	NULL,		&no_op,		NULL},	
/* P */	{c_flag,	NULL,		&no_op,		NULL},	
/* Q */	{c_bad,		NULL,		NULL,		NULL},	
/* R */	{c_bad,		NULL,		NULL,		NULL},	
/* S */	{c_bad,		NULL,		NULL,		NULL},	
/* T */	{c_bad,		NULL,		NULL,		NULL},	
/* U */	{c_string,	&cpp_args,	NULL,		NULL},	
/* V */	{c_string,	&comp_args,	NULL,		NULL},	
/* W */	{c_bad,		NULL,		NULL,		NULL},	
/* X */	{c_bad,		NULL,		NULL,		NULL},	
/* Y */	{c_bad,		NULL,		NULL,		NULL},	
/* Z */	{c_bad,		NULL,		NULL,		NULL},	
};

/*
 *	Programs to be called.
 */
earg	cpp_list[]	=
{
	{e_literal,	CPP_NAME, NULL, NULL, 0,	&cpp_list[1]},
	{e_literal,	"-M", NULL, NULL, 0,		&cpp_list[2]},
	{e_literal,	"-Dlint", NULL, NULL, 0,	&cpp_list[3]},
	{e_args,	NULL, NULL, &cpp_args, 0,	&cpp_list[4]},
	{e_input,	NULL, NULL, NULL, 0,		&cpp_list[5]},
	{e_output,	NULL, NULL, NULL, 0,		NULL},
};

earg	comp_list[]	=
{
	{e_literal,	COMP_NAME, NULL, NULL, 0,	&comp_list[1]},
	{e_literal,	"-O", NULL, NULL, 0,		&comp_list[2]},
	{e_splice,	NULL, NULL, NULL, 2,		&comp_list[3]},
	{e_literal,	"-f", NULL, NULL, 0,		&comp_list[4]},
	{e_string,	NULL, &src_name, NULL, 0,	&comp_list[5]},
	{e_args,	NULL, NULL, &comp_args, 0,	&comp_list[6]},
	{e_input,	NULL, NULL, NULL, 0,		&comp_list[7]},
	{e_output,	NULL, NULL, NULL, 0,		NULL},
};

earg	load_list[]	=
{
	{e_literal,	LOAD_NAME, NULL, NULL, 0,	&load_list[1]},
	{e_args,	NULL, NULL, &load_args, 0,	&load_list[2]},
	{e_args,	NULL, NULL, &files, 0,		&load_list[3]},
	{e_literal,	"-o", NULL, NULL, 0,		&load_list[4]},
	{e_output,	NULL, NULL, NULL, 0,		NULL},
};

eargs	cpp_eargs	=	{CPP_PATH, cpp_list};
eargs	comp_eargs	=	{COMP_PATH, comp_list};
eargs	load_eargs	=	{LOAD_PATH, load_list};

/*
 *	General ruotines.
 */

#define	talloc(t)	(t *)salloc(sizeof (t))

/*
 *	Remove temporaries and exit.
 */
void
quit(s)
int	s;
{
	register int	i;

	for (i = 0; i < ntemps; i++)
	{
		if (tempv[i] != NULL)
			(void)unlink(tempv[i]);
	}

	if (object != NULL)
		(void)unlink(object);

	exit(s);
}

/*
 *	Internal error.
 */
void
internal(s)
char	*s;
{
	fprintf(stderr, "%s: internal error - %s\n", my_name, s);
	quit(1);
}

/*
 *	Interrupt routine.
 */
int
rubbed()
{
	quit(1);
}

/*
 *	System error.
 */
perr(s)
char	*s;
{
	fprintf(stderr, "%s: ", my_name);
	perror(s);
	quit(1);
}

/*
 *	Memory allocation.
 */
char	*
salloc(n)
int	n;
{
	register char	*p;
	extern char	*malloc();

	if ((p = malloc((unsigned int)n)) == NULL)
	{
		fprintf(stderr, "%s: ran out of memory\n", my_name);
		quit(1);
	}

	return p;
}

char	*
srealloc(p, n)
register char	*p;
int		n;
{
	extern char	*realloc();

	if (p == NULL)
		return salloc(n);
	else if ((p = realloc(p, (unsigned int)n)) == NULL)
	{
		fprintf(stderr, "%s: ran out of memory\n", my_name);
		quit(1);
	}

	return p;
}

/*
 *	Copy a string.
 */
char	*
copy(s)
char	*s;
{
	return strcpy(salloc(strlen(s) + 1), s);
}

/*
 *	File suffix.
 */
int
suffix(s)
register char	*s;
{
	register int	i;

	if ((i = strlen(s)) < 2 || s[i - 2] != '.')
		return '\0';
	else
		return s[i - 1];
}

/*
 *	Replace file suffix.
 */
void
replace_suffix(s, c)
char	*s;
int	c;
{
	s[strlen(s) - 1] = c;
}

/*
 *	Construct a temporary file name.
 */
char	*
make_temp(sp, i)
char	**sp;
int	i;
{
	char		letter;
	char		buff[64];
	struct stat	statb;
	static int	pid;

	if (*sp != NULL)
		return *sp;

	if (pid == 0)
		pid = getpid();

	letter = 'a';

	for (;;)
	{
		sprintf(buff, "%s/ctm%05d_%02d%c", TMP_DIR, pid, i, letter);

		if (stat(buff, &statb) == -1)
		{
			if (errno == ENOENT)
			{
				*sp = copy(buff);
				return *sp;
			}

			perr(buff);
			quit(1);
		}

		letter++;
	}
}

/*
 *	Add an argument to a list.
 */
void
add_arg(s, p)
char		*s;
register args	*p;
{
	register arg	*a;

	a = talloc(arg);
	a->a_str = s;
	a->a_next = NULL;
	*p->a_tail = a;
	p->a_tail = &a->a_next;
}

/*
 *	Construct a new argument list.
 */
args	*
new_args()
{
	register args	*p;

	p = talloc(args);
	p->a_head = NULL;
	p->a_tail = &p->a_head;
	return p;
}

/*
 *	Free an argument list.
 */
void
free_args(a)
args	*a;
{
	register arg	*p;
	register arg	*q;

	for (p = a->a_head; p != NULL; p = q)
	{
		q = p->a_next;
		free((char *)p);
	}

	free((char *)a);
}

/*
 *	Make a string out of the catenation of an argument list.
 */
char	*
cat_args(a)
args	*a;
{
	register arg	*p;
	register int	i;
	register char	*s;
	register char	*t;

	for (i = 1, p = a->a_head; p != NULL; p = p->a_next)
		i += strlen(p->a_str);

	s = salloc(i);
	t = s;

	for (p = a->a_head; p != NULL; p = p->a_next)
	{
		(void)strcpy(s, p->a_str);
		s += strlen(p->a_str);
	}

	return t;
}

/*
 *	Copy an argument list.
 */
void
cp_args(from, to)
args		*from;
register args	*to;
{
	register arg	*p;

	for (p = from->a_head; p != NULL; p = p->a_next)
		add_arg(p->a_str, to);
}

/*
 *	Add an exec argument and return a pointer to the next item.
 */
earg	*
add_exec_arg(e, a)
register earg	*e;
register args	*a;
{
	register int	i;
	register char	*s;
	register char	*t;
	args		*temp;

	switch (e->e_code)
	{
	case e_args:
		cp_args(*e->e_args, a);
		break;

	case e_input:
		add_arg(input, a);
		break;

	case e_literal:
		add_arg(e->e_str, a);
		break;

	case e_output:
		add_arg(output, a);
		break;

	case e_splice:
		s = copy("");
		i = e->e_count;
		e = e->e_next;

		while (--i >= 0)
		{
			if (e == NULL)
				internal("bad splice");

			temp = new_args();
			add_arg(s, temp);
			e = add_exec_arg(e, temp);
			t = cat_args(temp);
			free(s);
			free_args(temp);
			s = t;
		}

		add_arg(s, a);
		return e;

	case e_string:
		add_arg(*e->e_strp, a);
		break;

	default:
		internal("bad ecode switch");
	}

	return e->e_next;
}

/*
 *	Argument processing.
 */
int
options(n, v)
register int	n;
register char	**v;
{
	register char	*p;
	register int	c;
	register option	*o;

	while (--n >= 0)
	{
		if (**v == '-')
		{
			p = *v++;

			while ((c = *++p) != '\0')
			{
				if (c >= 'a' && c <= 'z')
					o = &lcase[c - 'a'];
				else if (c >= 'A' && c <= 'Z')
					o = &ucase[c - 'A'];
				else
				{
					fprintf(stderr, "%s: unknown option '%c'\n", my_name, c);
					return 1;
				}

				switch (o->o_code)
				{
				case c_bad:
					/*
					 *	Unknown flag.
					 */
					fprintf(stderr, "%s: unknown option '%c'\n", my_name, c);
					return 1;

				case c_flag:
					/*
					 *	Set a flag.
					 */
					*o->o_flag = 1;
					continue;

				case c_function:
					/*
					 *	Call a function.
					 */
					if ((*o->o_func)())
						return 1;

					continue;

				case c_str_func:
					/*
					 *	Pick up a string and call a function.
					 */
					if (*++p == '\0')
					{
						if (--n < 0 || *(p = *v++) == '-')
						{
							fprintf(stderr, "%s: argument expected for '-%c' option\n", my_name, c);
							return 1;
						}
					}

					if ((*o->o_func)(p))
						return 1;

					break;

				case c_string:
					/*
					 *	Pick up a complete argument.
					 */
					if (p != &v[-1][1])
					{
						fprintf(stderr, "%s: bad '%c' option\n", my_name, c);
						return 1;
					}

					if (p[1] == '\0')
					{
						fprintf(stderr, "%s: no string for '%c' option\n", my_name, c);
						return 1;
					}

					add_arg(v[-1], *o->o_args);
					break;

				default:
					internal("bad option switch");
					return 1;
				}

				break;
			}
		}
		else
			add_arg(*v++, files);
	}

	return 0;
}

/*
 *	Catch a signal if it isn't being ignored.
 */
void
set_signal(i)
int	i;
{
	if (signal(i, SIG_IGN) != SIG_IGN)
		(void)signal(i, rubbed);
}

/*
 *	Initialise.
 */
void
init(s)
char	*s;
{
	if ((my_name = strrchr(s, '/')) == NULL || *++my_name == '\0')
		my_name = s;

	files = new_args();

	set_signal(SIGINT);
	set_signal(SIGTERM);
}

/*
 *	Run s on argv.
 */
int
run(s, argv)
char	*s;
char	**argv;
{
	register int	pid;
	register int	ret;
	register int	(*isig)();
	register int	(*qsig)();
	int		status;

	switch (pid = fork())
	{
	case 0:
		execv(s, argv);
		perr(s);

	case -1:
		perr("fork");

	default:
		isig = signal(SIGINT, SIG_IGN);
		qsig = signal(SIGQUIT, SIG_IGN);

		while ((ret = wait(&status)) != pid)
		{
			if (ret == -1)
				perr("wait");
		}

		if (isig != SIG_IGN)
			signal(SIGINT, isig);

		if (qsig != SIG_IGN)
			signal(SIGQUIT, qsig);
	}

	if ((status & 0xFF) != 0)
	{
		if ((status & 0x80) != 0)
			fprintf(stderr, "%s: fatal error in %s (core dumped)\n", my_name, s);

		quit(1);
	}

	return (status >> 8) & 0xFF;
}

/*
 *	Execute an exec arglist.
 */
int
execute(e)
eargs	*e;
{
	register int 	i;
	register earg 	*p;
	register arg 	*q;
	args		*a;
	static char	**argv;
	static int	argc;

	a = new_args();

	for (p = e->e_list; p != NULL; p = add_exec_arg(p, a))
		;

	for (i = 1, q = a->a_head; q != NULL; q = q->a_next)
		i++;

	if (i > argc)
		argv = (char **)srealloc((char *)argv, i * sizeof (char *));

	for (i = 0, q = a->a_head; q != NULL; q = q->a_next)
		argv[i++] = q->a_str;

	if (verbose)
	{
		printf("%s:", e->e_path);

		for (q = a->a_head; q != NULL; q = q->a_next)
			printf(" %s", q->a_str);

		printf("\n");
		fflush(stdout);
	}

	free_args(a);
	argv[i] = NULL;

	return run(e->e_path, argv);
}

/*
 *	Cyntax.
 */

/*
 *	User initialisation.
 */
void
uinit()
{
	cpp_args = new_args();
	comp_args = new_args();
	load_args = new_args();
}

/*
 *	Handle '-d' flag.
 */
int
dflag()
{
	add_arg("-d", load_args);
	return 0;
}

/*
 *	Handle '-G' flag.
 */
int
ugflag()
{
	gnot = 1;
	add_arg(GNOT_INCLUDE, cpp_args);
	return 0;
}

/*
 *	Handle '-k' flag.
 */
int
kflag()
{
	ken = 1;
	add_arg(KEN_INCLUDE, cpp_args);
	return 0;
}

/*
 *	Handle '-j' flag.
 */
int
jflag()
{
	jerq = 1;
	add_arg(JERQ_INCLUDE, cpp_args);
	return 0;
}

/*
 *	Handle '-m' flag.
 */
int
mflag()
{
	add_arg(DEF_MUX, cpp_args);
	return 0;
}

/*
 *	Handle '-w' flag.
 */
int
wflag()
{
	add_arg("-w", comp_args);
	return 0;
}

/*
 *	Handle '-o' string.
 */
int
ostring(s)
char	*s;
{
	if (load_out != NULL)
	{
		fprintf(stderr, "%s: repeat '-o' option\n", my_name);
		return 1;
	}

	load_out = s;
	return 0;
}

/*
 *	Compile the '.c's.
 */
int
compile()
{
	register int	i;
	register int	ret;
	register arg	*p;
	register char	*s;
	struct stat	statb;

	ret = 0;

	ntemps = 1;

	tempv = (char **)salloc(ntemps * sizeof (char *));

	for (i = 0; i < ntemps; i++)
		tempv[i] = NULL;

	for (p = files->a_head; p != NULL; p = p->a_next)
	{
		switch (suffix(p->a_str))
		{
		case 'c':
			if (src_name != NULL)
				free(src_name);

			if (stat(p->a_str, &statb) == -1)
				src_name = copy(p->a_str);
			else
			{
				src_name = salloc(strlen(p->a_str) + 1 + 12 + 1);
				sprintf(src_name, "%s@%ld", p->a_str, statb.st_mtime);
			}

			if (multi_cfile)
			{
				printf("%s:\n", p->a_str);
				fflush(stdout);
			}

			i = 0;
			input = p->a_str;
			output = make_temp(&tempv[i], i);

			if (execute(&cpp_eargs))
			{
				ret = 1;
				break;
			}

			if (hflag && (s = strrchr(p->a_str, '/')) != NULL)
				p->a_str = s + 1;

			replace_suffix(p->a_str, 'O');
			input = output;
			output = p->a_str;
			object = p->a_str;

			if (execute(&comp_eargs))
			{
				(void)unlink(object);
				ret = 1;
				break;
			}
			else
				object = NULL;

			break;

		case 'A':
		case 'O':
		case '\0':
			break;

		default:
			fprintf(stderr, "%s: unknown file type '%s'\n", my_name, p->a_str);
			ret = 1;
		}
	}

	return ret;
}

/*
 *	Load the resultant objects and the rest.
 */
int
load()
{
	if (cflag)
		return 0;

	if (load_out != NULL)
		output = load_out;
	else
		output = DEF_OUT;

	if (ken)
		add_arg(KEN_LIB, files);
	else if (jerq)
		add_arg(JERQ_LIB, files);
	else if (gnot)
		add_arg(GNOT_LIB, files);
	else if (!nolib)
		add_arg(DEF_LIB, files);

	return execute(&load_eargs);
}

/*
 *	Find out if there are more than one '.c' and at least one file.
 */
int
scan_files()
{
	register arg	*p;
	register int	i;

	p = files->a_head;

	if (p == NULL)
	{
		fprintf(stderr, "%s: no files specified\n", my_name);
		return 1;
	}

	i = 0;

	do
	{
		if (suffix(p->a_str) == 'c')
		{
			if (i)
			{
				multi_cfile = 1;
				break;
			}

			i = 1;
		}
	}
	while ((p = p->a_next) != NULL);

	return 0;
}

/*
 *	User action routine.
 */
int
do_it()
{
	return scan_files() || compile() || load();
}

/*
 *	Compiler front end.
 */
int
main(argc, argv)
int	argc;
char	*argv[];
{
	init(argv[0]);
	uinit();
	quit(options(argc - 1, &argv[1]) || do_it());
}
