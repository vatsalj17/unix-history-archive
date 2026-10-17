#include	<errno.h>
#include	"cem.h"
#include	"stdobj.h"

/*
 *  Ccom and cem must agree on .O format version.
 *
 * known versions to date:
 *  3		as Bruce Ellis left MH
 *  5		in sourceless binary from B.E. in Sydney, with bug fixes
 *		in ccom.  (Not known if .O formats actually differ.)
 *  3r		Reeds's revision of 3, with Ellis's faulty integer compression
 *		scheme abandoned. (Nov 88)
 */

static char 	version[24] = "#!/bin/echo 3r\n";

/*
 *	Read and load an object file.
 */
static void
read_obj(s, fd)
register char	*s;
register int	fd;
{
	register int	i;
	register long	sz;
	header		obj_header;
	extern void	enter_types();
	extern void	enter_vars();
	extern void	install_strings();

	switch (read(fd, (char *)&obj_header, sizeof (header)))
	{
	case SYSERROR:
		fprint(2, "%s: could not read ", my_name);
		perror(s);
		exit(1);

	case sizeof (header):
		break;

	default:
		fprint(2, "%s: %s is not an object module\n", my_name, s);
		exit(1);
	}

	if (strncmp(obj_header.hd_cmd, version, 12) != 0)
	{
		fprint(2, "%s: %s is not an object module\n", my_name, s);
		exit(1);
	}

	if (strncmp(obj_header.hd_cmd, version, sizeof(version)) != 0)
	{
		fprint(2, "%s: version mismatch for %s (recompile)\n", my_name, s);
		exit(1);
	}

	if (obj_header.hd_type_size == 0)
	{
		fprint(2, "%s: %s has compilation errors\n", my_name, s);
		exit(1);
	}

	sz = obj_header.hd_str_off + obj_header.hd_str_size - sizeof (header);
	data_base = salloc(sz);
	str_base = data_base + obj_header.hd_str_off - sizeof (header) - 1;

	if ((i = read(fd, data_base, (int)sz)) != sz)
	{
		if (i == SYSERROR)
		{
			fprint(2, "%s: could not read ", my_name);
			perror(s);
		}
		else
			fprint(2, "%s: %s is too small\n", my_name, s);

		exit(1);
	}

#if	DEBUG
	if (debug)
	{
		register char	*id;
		register long	l0;
		register long	l1;
		register long	l2;

		data_ptr = data_base;
		data_end = data_base + obj_header.hd_str_off - sizeof (header);
		type_index = 1;
		var_index = 1;
		str_num = 0;

		while (data_ptr < data_end)
		{
			i = getd();

			switch (obj_item(i))
			{
			case i_data:
				l0 = getnum();
				l1 = getnum();
				Fprint(1,"data: var %ld (%s:%ld)\n", l0, &str_base[l1], getnum());

				loop
				{
					i = getd();

					switch (obj_item(i))
					{
					case d_addr:
						Fprint(1,"\taddr var %ld for %ld\n", getnum(), obj_id(i));
						continue;

					case d_bytes:
						if (obj_id(i) == 0)
							l0 = getnum();
						else
							l0 = obj_id(i);

						Fprint(1,"\tbytes %ld\n", l0);
						data_ptr += l0;
						continue;

					case d_end:
						Fprint(1,"\tend\n");
						break;

					case d_istring:
						if (obj_id(i) == 0)
							l0 = getnum();
						else
							l0 = obj_id(i);

						data_ptr += l0;
						Fprint(1,"\tstring %ld, length %ld\n", str_num++, l0);
						continue;

					case d_irstring:
						if (obj_id(i) == 0)
							l0 = getnum();
						else
							l0 = obj_id(i);

						data_ptr += l0;
						Fprint(1,"\tstring %ld + %ld, length %ld\n", str_num++, getnum(), l0);
						continue;

					case d_space:
						Fprint(1,"\tspace %ld\n", obj_id(i) == 0 ? getnum() : obj_id(i));
						continue;

					case d_string:
						Fprint(1,"\tstring %ld\n", getnum());
						continue;

					case d_reloc:
						l0 = getnum();
						Fprint(1,"\treloc var %ld + %ld for %ld\n", l0, getnum(), obj_id(i));
						continue;

					case d_rstring:
						l0 = getnum();
						Fprint(1,"\tstring %ld + %ld\n", l0, getnum());
						continue;

					default:
						fprint(2, "%s: unknown data id %d\n", my_name, i);
						exit(1);
					}

					break;
				}

				break;

			case i_lib:
				Fprint(1,"lib: %s\n", &str_base[getnum()]);
				break;

			case i_src:
				Fprint(1,"src: %s\n", &str_base[getnum()]);
				break;

			case i_string:
				if (obj_id(i) == 0)
					l0 = getnum();
				else
					l0 = obj_id(i);

				Fprint(1,"string %ld, length %ld\n", str_num++, l0);
				data_ptr += l0;
				break;

			case i_type:
				Fprint(1,"type ");

				switch (obj_id(i))
				{
				case t_arrayof:
					l0 = getnum();
					l1 = getnum();
					Fprint(1,"%ld: array %ld of %ld\n", type_index++, l0, l1);
					break;

				case t_basetype:
					Fprint(1,"%ld: ", type_index++);
					print_basetype(getd() & 0xFF);
					Fprint(1,"\n");
					break;

				case t_bitfield:
					l0 = getnum();
					l1 = getnum();
					Fprint(1,"%ld: bitfield %ld of %ld\n", type_index++, l0, l1);
					break;

				case t_dimless:
					Fprint(1,"%ld: array [] of %ld\n", type_index++, getnum());
					break;

				case t_elaboration:
					l0 = getnum();
					l1 = getnum();
					Fprint(1,"elab %ld: (%s:%ld) ", l0, &str_base[l1], getnum());
					
					switch (i = obj_id(getd()))
					{
					case t_enum:
						Fprint(1,"enum\n");
						l0 = getnum();
						goto elab_enum;

					case t_structof:
						Fprint(1,"struct\n");
						l0 = getnum();

						do
						{
							l1 = getnum();
							Fprint(1,"\t%s = %ld @ %ld\n", &str_base[l0], l1, getnum());
							l0 = getnum();
						}
						while (l0 != 0);

						Fprint(1,"\t(%ld)\n", getnum());
						break;

					case t_unionof:
						Fprint(1,"union\n");
						l0 = getnum();

						do
						{
							l1 = getnum();
							Fprint(1,"\t%s = %ld\n", &str_base[l0], l1);
							l0 = getnum();
						}
						while (l0 != 0);

						Fprint(1,"\t(%ld)\n", getnum());
						break;

					default:
						fprint(2, "%s: unknown elaboration id %d\n", my_name, i);
						exit(1);
					}

					break;

				case t_enum:
					Fprint(1,"%ld: ", type_index++);

					if ((l0 = getnum()) == 0)
						id = "<anon>";
					else
						id = &str_base[l0];

					l0 = getnum();
					l1 = getnum();

					Fprint(1,"enum %s (%s:%ld)\n", id, &str_base[l0], l1);

					if ((l0 = getnum()) == 0)
					{
						Fprint(1,"\tforward\n");
						break;
					}

				elab_enum:
					do
					{
						l1 = getnum();
						Fprint(1,"\t%s = %ld\n", &str_base[l0], l1);
						l0 = getnum();
					}
					while (l0 != 0);

					l0 = getnum();
					l1 = getnum();

					Fprint(1,"\t(%ld, %ld)\n", l0, l1);
					break;

				case t_ftnreturning:
					Fprint(1,"%ld: function returning %ld\n", type_index++, getnum());
					break;

				case t_ptrto:
					Fprint(1,"%ld: pointer to %ld\n", type_index++, getnum());
					break;

				case t_structof:
					Fprint(1,"%ld: ", type_index++);

					if ((l0 = getnum()) == 0)
						id = "<anon>";
					else
						id = &str_base[l0];

					l0 = getnum();
					l1 = getnum();

					Fprint(1,"struct %s (%s:%ld)\n", id, &str_base[l0], l1);
					break;

				case t_unionof:
					Fprint(1,"%ld: ", type_index++);

					if ((l0 = getnum()) == 0)
						id = "<anon>";
					else
						id = &str_base[l0];

					l0 = getnum();
					l1 = getnum();

					Fprint(1,"union %s (%s:%ld)\n", id, &str_base[l0], l1);
					break;

				default:
					fprint(2, "%s: unknown type id %d\n", my_name, obj_id(i));
					exit(1);
				}

				break;

			case i_var:
				Fprint(1,"var ");

				switch (obj_id(i))
				{
				case v_arglist:
					l0 = getnum();
					l1 = getnum();
					Fprint(1,"arglist: %ld (%s:%ld)\n", l0, &str_base[l1], getnum());

					while ((l0 = getnum()) != 0)
					{
						l1 = getnum();
						l2 = getnum();
						Fprint(1,"\t%ld: %s type %ld (%s:%ld)\n", var_index++, &str_base[l0], l1, &str_base[l2], getnum());
					}

					break;

				case v_array_size:
					l0 = getnum();
					Fprint(1,"array %ld new type %ld\n", l0, getnum());
					break;

				case v_auto:
					id = "auto";
					goto get_var;

				case v_block_static:
					id = "block static";
					goto get_var;

				case v_call:
					l0 = getnum();
					l1 = getnum();
					Fprint(1,"call: var %ld (%s:%ld)\n", l0, &str_base[l1], getnum());

					while ((l0 = getnum()) != 0)
						Fprint(1,"\ttype %ld\n", l0);

					break;

				case v_global:
					id = "global";
					goto get_var;

				case v_implicit_function:
					id = "implicit()";
					goto get_var;

				case v_static:
					id = "static";
					goto get_var;

				get_var:
					l0 = getnum();
					l1 = getnum();
					l2 = getnum();
					Fprint(1,"%ld: %s %s type %ld (%s:%ld)\n", var_index++, id, &str_base[l0], l1, &str_base[l2], getnum());
					break;

				case v_varargs:
					l0 = getnum();
					Fprint(1,"%ld: varargs %ld\n", l0, getnum());
					break;

				default:
					fprint(2, "%s: unknown var id %d\n", my_name, obj_id(i));
					exit(1);
				}

				break;

			default:
				fprint(2, "%s: unknown obj_item %d\n", my_name, obj_item(i));
				exit(1);
			}
		}

		Fflush(1);
	}
#endif	DEBUG

	file_errors = 0;
	install_strings(str_base + 1, obj_header.hd_str_size);

	type_index = 1;
	data_ptr = data_base;
	data_end = data_base + obj_header.hd_str_off - sizeof (header);
	enter_types(obj_header.hd_type_size);

	str_num = 0;
	var_index = 1;
	data_ptr = data_base;
	data_end = data_base + obj_header.hd_str_off - sizeof (header);
	enter_vars(obj_header.hd_var_size);

	free((char *)str_trans);
	free((char *)type_trans);
	free((char *)var_trans);
	free(data_base);
	src_file = NULL;

	if (file_errors)
		Fflush(1);
}

/*
 *	Open a library and load it.
 */
void
load_lib(l)
char	*l;
{
	register int	fd;
	register char	*s;
	extern int	errno;
	extern char	*strcat();
	extern char	*strcpy();

	s = salloc(strlen(LIB_PATH) + strlen(l) + 1L);
	(void)strcat(strcpy(s, LIB_PATH), l);

	if ((fd = open(s, 0)) == SYSERROR)
	{
		if (errno == ENOENT)
			fprint(2, "%s: no library '-l%s'\n", my_name, l);
		else
		{
			fprint(2, "%s: could not open ", my_name);
			perror(s);
		}

		exit(1);
	}

	read_obj(s, fd);
	free(s);
	(void)close(fd);
}

/*
 *	Open an object file and load it.
 */
void
load_obj(s)
register char	*s;
{
	register int	fd;

	if ((fd = open(s, 0)) == SYSERROR)
	{
		fprint(2, "%s: could not open ", my_name);
		perror(s);
		exit(1);
	}

	read_obj(s, fd);
	(void)close(fd);
}
