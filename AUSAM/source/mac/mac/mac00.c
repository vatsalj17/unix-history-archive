#include	"mac.h"
#include	"mac.x"

char	tmp[16]		"/tmp/mac";		/* Space for tmp file name */
char	*arg[8];				/* Exec arg list */

main(argc, argv)
char	*argv[ ];
{
	register int nfile;
	register int pid;
	register int fd;
	register int i;
	register char *r;

	nfile = 0;					/* No args seen */
	OPTION('h') = TRUE;				/* default */
	for (i=1; i<argc; i++)  {
		p = argv[i];
		if (*p == '-')  {
			/* Option flag */
			p++;
			if (*p == 'h' || *p == 'o' || *p == 'b')  {
				OPTION('h') = FALSE;
				OPTION('o') = FALSE;
				OPTION('b') = FALSE;
				}

			OPTION(*p) = TRUE;
			}
		else
			/* File-name */
			arg[nfile++] = p;
		}

	if (nfile > 2)  {
		/* object file needed */
		OPTION('a') = TRUE;
		OPTION('d') = FALSE;
		OPTION('l') = FALSE;
		}

	if (OPTION('l'))
		/* listing - no code dump */
		OPTION('d') = FALSE;
	else
		if (OPTION('h') || OPTION('o') || OPTION('b'))
			OPTION('d') = TRUE;

	if (OPTION('n'))  {
		OPTION('d') = FALSE;
		OPTION('l') = FALSE;
		}


	if (nfile < 1)  {
		printf("Usage: %s opcode-file [source] [object]\n", argv[0]);
		exit(1);
		}

	/*
	 *   Try to open r-file.
	 *   try 'file' first, and if that fails,
	 *   try '/usr/lib/mac/file'.
	 */
	for (i=0; i<13; i++)
		buf[i] = "/usr/lib/mac/"[i];
	r = arg[0];
	for (i=13; *r != '\0'; i++)
		buf[i] = *r++;
	fd = open(&buf[13], 0);
	if (fd < 0)
		/* try lib */
		fd = open(&buf[0], 0);

	if (fd < 0)  {
		synerr("Can't open r-file");
		exit(1);
		}

	if (nfile > 1 && fopen(arg[1], &ibuf) < 0)  {
		printf("Can't find source file\n");
		exit(1);
		}


	/*
	 *   Make temp file.
	 */
	pid = getpid();
	num(pid, tmp+8);
	unit = creat(tmp, 0600);
	if (unit < 0)  {
		printf("Can't create temp\n");
		exit(1);
		}

	close(unit);
	unit = open(tmp, 2);

	if (!OPTION('u'))
		/* unlink temp file if OPTION('u') is not on */
		unlink(tmp);

	tbl(fd);		/* initialise tables */
	pass1();		/* parse source, build symtab, temp code */

	close(ibuf.b_fdesc);
	if (errcount)  {
		synerr("Errors in pass 1.");
		exit(1);
		}

	seek(unit, 0, 0);

	if (OPTION('l'))
		if (nfile < 2)  {
			warning("Listing from std. input impossible!!");
			OPTION('l') = FALSE;
			}
		else	{
			i = fopen(arg[1], &ibuf);
			if (i < 0)  {
				synerr("Can't re-open source");
				exit(1);
				}
			}

	if (nfile > 2 && (afd = creat(arg[2], 0666)) < 0)  {
		synerr("Can't create object file");
		OPTION('a') = FALSE;
		}

	if (nfile < 3  &&  OPTION('a'))
		if ((afd = creat("m.out", 0666)) < 0)  {
			synerr("Can't create m.out");
			OPTION('a') = FALSE;
			}


	adjust();		/* adjust location counters for assembly */
	pass2();		/* initiate pass2 */
	cdump();		/* code dump to std output */
	sdump();		/* symbol table dump to std output */

	return;
}

tbl(fd)
register int fd;
{
	register struct st *s;
	register int i;


	/*
	 *	read header record:
	 *
	 *	contains number of format descriptors,
	 *	and number of opcode descriptors,
	 *	pre-defined labels, literals,
	 *	and the size of the parser table.
	 *	It also contains interesting information
	 *	about the target machine.
	 *
	 *	the file contains:-
	 *		literals;
	 *		format descriptors;
	 *		opcode table;
	 *		parser table;
	 *		pre-defined labels;
	 *
	 *	calculate core requirements and sbrk() 
	 *	for the core.
	 */

	read(fd, &head, HT);
	i = head.h_literals;
	if (i)  {
		i =* 8;			/* sizeof literal */
		literals = sbrk(i);
		read(fd, literals, i);
		}
	i = FD * head.h_formats;
	memory = sbrk(i);
	read(fd, &memory[0], i);
	i = head.h_o_len * head.h_ops;
	opcode = sbrk(i);
	read(fd, &opcode[0], i);
	i = head.h_p_len * TBL;
	parse = sbrk(i);
	read(fd, &parse[0], i);
	symtab = coreptr = cpget();

	i = head.h_labels;
	while (i-- > 0)  {
		read(fd, buf, ST);
		/* enter label in symbol table */
		s = buf;
		lscan(s->s_name, DEF, any(buf[0],alptab), s->s_value, s->s_mode);
		}

	return;
}

adjust()
{
	register struct	lt *q;
	register int rs;
	register int i;
	register int nwrd;

	/*
	 *   adjust all locn. counters and
	 *   set up code pointers. brk core.
	 *
	 *	l_rel_f	:	pointer to start of core segment
	 *	l_start :	start addr of code seg. (logical)
	 */
	rs = 0;
	q = &locn[0];
	for (i=0; i<LCOUNT; i++)  {
		if (q->l_value > q->l_limit)		/* backwards org ??? */
			q->l_limit = q->l_value;
		rs =+ (q->l_limit - q->l_start);
		q->l_value = 0;
		q++;
		}

	/*
	 *   rs:  Number of basic units in all segments.
	 *	  Allow 1 word / byte.
	 *	  If r option is on - allow space for reloc info.
	 */
	if (OPTION('r'))
		rs =* 2;

	code = sbrk(rs * INT);
	if (code < 0)  {
		synerr("no core for assembly");
		exit(1);
		}
	endcore = sbrk(0);

	/*
	 *   Fix pointers to core for pass 2 assembly
	 */
	rs = 0;
	q = &locn[0];
	for (i=0; i<LCOUNT; i++)  {
		q->l_rel_f = q->l_next = code + rs;
		nwrd = q->l_limit - q->l_start;
		if (OPTION('r'))  {
			q->l_rel_n = q->l_reloc = code + rs + nwrd;
			rs =+ (nwrd * 2);
			}
		else
			rs =+  nwrd;

		q++;
		}

	nline = lcntr = 0;		/* Reset line and segment counters */
	return;
}
