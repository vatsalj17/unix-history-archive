/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"io.h"
#include	"options.h"

char	*myname;
char	*libname;
char	*srcname	= "<stdin>";

int	Options[(int)o_options];

static void
usage()
{
	fatal("[-f srcname[@modtime]] [-l libname] [-V func:n] [-Orw] [infile [outfile]]");
}

int
main(argc, argv)
int	argc;
char	*argv[];
{
	register int	i;
	register int	j;
	int		files;
	int		varargs;
	char		*ofile = 0;
	extern int	in_fid;
	extern int	out_fid;
	extern int	err_fid;
	extern void	init_alloc();
	extern void	init_lex();
	extern void	init_str();
	extern void	init_type();
	extern void	init_obj();
	extern void	end_obj();
	extern void	put_src();
	extern void	put_varargs();
	extern int	err_file();

	Finit(1, (char*)0);
	fmtinstall('@', err_file);
	myname = argv[0];
	files = 0;
	varargs = 0;

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (argv[i][1] == '\0')
			{
				files++;
				continue;
			}

			for (j = 1; argv[i][j] != '\0'; j++)
			{
				switch (argv[i][j])
				{
				case 'O':
					Options[(int)o_optimise] = 1;
					continue;

				case 'V':
					if (argv[i][++j] == '\0')
					{
						if (++i >= argc)
							usage();

						argv[varargs++] = argv[i];
					}
					else
						argv[varargs++] = &argv[i][j];

					break;

				case 'f':
					if (argv[i][++j] == '\0')
					{
						if (++i >= argc)
							usage();

						srcname = argv[i];
					}
					else
						srcname = &argv[i][j];

					break;

				case 'l':
					if (argv[i][++j] == '\0')
					{
						if (++i >= argc)
							usage();

						libname = argv[i];
					}
					else
						libname = &argv[i][j];

					break;

				case 'r':
					Options[(int)o_restricted] = 1;
					continue;

				case 'w':
					Options[(int)o_warnings] = 1;
					continue;

				default:
					usage();
				}

				break;
			}
		}
		else
		{
			switch (files)
			{
			case 0:
				if ((in_fid = open(argv[i], 0)) == SYSERROR)
				{
					Ea[0].m = argv[i];
					fatal("could not open \"%\"");
				}

				break;

			case 1:
				if ((out_fid = creat(ofile = argv[i], 0777)) == SYSERROR)
				{
					Ea[0].m = argv[i];
					fatal("could not create \"%\"");
				}
				Finit(out_fid, (char*)0);


				break;

			case 2:
				usage();
			}

			files++;
		}
	}

	init_alloc();
	init_lex();

	init_obj();

	init_str();

	put_src(libname, srcname);

	init_type();
	(void)declaration((ident *)NULL, at_outer);

	if (errored) {
		if(ofile)unlink(ofile);
		return 1;
	}

	global_lineno = -1;

	for (i = 0; i < varargs; i++)
		put_varargs(argv[i]);

	end_obj();

	return errored != 0;
}
