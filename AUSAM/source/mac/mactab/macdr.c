#include	"../mac/mac.h"


/*
 *   MACDR:
 *
 *   Decode a pre-formatted MAC r-file, and symbolically
 *   print it.  This program is useful for debugging the
 *   MACTAB table formatter, and for inspecting a forgotten
 *   r-file.
 *
 */



char	fmtdesc[ ]	=	"\tdesc %8x  value %8x  width %d\n";
char	buf[256];
int	fd;
struct	ht	h;

char	*symt[ ]	{
	"NUL",
	"LBL",
	"CON",
	"SPA",
	"DEL",
	"EOL",
	"EOF",
	"COM",
	"OPR",
	"STR",
	"MCH",
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	"EXP",
	"SKP",
	"LIT",
	"CHR"
	};

char	*actt[ ]	{
	"NOOP",
	"ALBL",
	"DOPV",
	"OLBL",
	"OOPR",
	"ODEL",
	"OCON",
	"OCHR",
	"OSTR",
	"ENDP",
	"SELC",
	"EXPR",
	"RETN",
	"GOTO",
	"OERR",
	"OREC"
	};


main(argc, argv)
char *argv[ ];
{
	register struct st *s;
	register char *p;
	register int len;
	register int i;
	register int j;

	if (argc != 2)  {
		printf("Usage: %s r-file-name\n", argv[0]);
		exit(1);
		}

	if ((fd = open(argv[1], 0)) < 0)  {
		printf("Can't open %s\n", argv[1]);
		exit(1);
		}

	p = buf;
	read(fd, &h, HT);
	printf("byte width is %d bits\n", h.h_bu_len);
	printf("word width is %d bytes\n", h.h_w_len);
	printf("illegal instruction opcode is %x\n", h.h_ii);
	printf("machine %s\n", h.h_mac);
	printf("parser starts at %d\n", h.h_p_start);
	printf("page length %d lines\n", h.h_page);
	printf("pc is ");
	(h.h_pc_post ? printf("post") : printf("pre"));
	printf("-incremented\n\n");
	for (i=0; i<5; i++)
		if (h.dctype[i].f_len)  {
			printf("dc%c  ", h.dctype[i].f_class);
			printf("%2d bytes\n", h.dctype[i].f_len);
			for (j=0; h.dctype[i].f_desc[j]; j++)
				printf(fmtdesc, h.dctype[i].f_desc[j],
						h.dctype[i].f_value[j],
						h.dctype[i].f_width[j]);
			}
	putchar('\n');

	if (h.h_literals)
		printf("%d literals\n", h.h_literals);
	else
		printf("no literals\n");

	while (h.h_literals--)  {
		read(fd, buf, 8);
		printf("%8s  ", buf);
		}
	printf("\n\n");

	printf("format descriptors\n");
	for (i=0; h.h_formats--; i++)  {
		read (fd, buf, FD);
		printf("%3d: ", i);
		printf("class %2d  ", p->f_class);
		printf("length %2d\n", p->f_len);
		for (j=0; p->f_desc[j]; j++)
			printf(fmtdesc, p->f_desc[j],
					p->f_value[j], p->f_width[j]);
		}
	printf("\n\n");

	printf("opcode table\n");
	len = (h.h_o_len - 8 - INT) / (2 * INT);
	while (h.h_ops--)  {
		read(fd, buf, h.h_o_len);
		printf("%-8s  ", p->o_name);
		for (i=0; i<len; i++)  {
			printf("(%2d ", p->o_code[i].o_format);
			printf("%4x),", p->o_code[i].o_value);
			}

		putchar('\n');
		}
	printf("\n\n");

	printf("parser table\n\n");
	for (i=0; h.h_p_len--; i++)  {
		read(fd, buf, TBL);
		printf("%3d: ", i);
		printf("%3s  ", symt[abs(p->tb_sym)]);
		printf("%8x  ", p->tb_mem);
		printf("(%4d)  ", p->tb_next);
		printf("%4s  ", actt[p->tb_act]);
		for (j=0; j<4; j++)
			printf("%2x ", p->tb_arg[j]);
		putchar('\n');
		}
	printf("\n\n");

	if (h.h_labels)
		printf("%d labels\n", h.h_labels);
	else
		printf("no pre-defined labels\n");

	while (h.h_labels--)  {
		read(fd, buf, ST);
		printf("%-8s ", p->s_name);
		printf("%11d  %8x\n", p->s_value, p->s_value);
		}

	return;
}


abs(k)
register int k;
{
	if (k < 0)
		return(-k);
	return(k);
}
