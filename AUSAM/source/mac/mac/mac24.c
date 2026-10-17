#include	"mac.h"
#include	"mac.x"


/*
 *   A.out structure generation.
 */

struct	out	{
	int	ou_nseg;			/* segment counter */
	int	ou_start;			/* start addr */
	int	ou_length;			/* seg len */
	}	out;

struct	aout	{
	int	ao_mword;			/* header - magic word */
	int	ao_bu_len;			/* byte width */
	}	aout;

#ifdef		PDP11

#define	AOUT	4
#define	OUT	6

#endif


#ifdef		INTERDATA

#define	AOUT	(sizeof (struct aout))
#define	OUT	(sizeof (struct out))

#endif



/*
 *   Symbol table dump.
 */
sdump()
{
	register struct st *q;
	register int n;
	register int obj;			/* object dump flag */
	register int dump;			/* code dump */

	dump = FALSE;
	obj = FALSE;
	if (OPTION('s'))
		dump = TRUE;			/* dump */
	if (OPTION('a'))  {
		obj = TRUE;
		out.ou_nseg = -1;
		out.ou_start = 0;
		out.ou_length = 0;
		write(afd, &out, OUT);
		}

	/*
	 *   Calculate column width for the printing
	 *   of the label's value.
	 */
	n = head.h_bu_len;
	if (OPTION('h'))
		n =/ 4;
	if (OPTION('o'))
		n =/ 3;
	n =* head.h_w_len;

	q = symtab;
	if (q >= coreptr)
		return;

	if (dump)  {
		header();
		printf("NAME          VALUE\n\n");
		}

	while (q < coreptr)  {

		if (dump)  {
			printf("%-8s      ", q->s_name);
			printl(n, q->s_value);
			printf("  \t");

			if (!(q->s_mode & REFR))
				printf("NREF\t");
			if (q->s_mode & GLOB)
				printf("GLOB\t");
			putchar('\n');
			newpage();
			}

		if (obj &&  q->s_mode & GLOB)
			/* write symbol table entry */
			write(afd, q, 8+2*INT);

		q++;
		}

	return;
}


/*
 *   Code dump to std. output
 *   and a.out generation.
 */
cdump()
{
	register int *len;
	register int *r;
	register int i;
	register int j;
	register int wdump;			/* page width */
	register int width;


	/*
	 *   Decide upon dump format.
	 *   hexadecimal defaults.
	 */
	width = head.h_bu_len / 4;

	if (OPTION('o'))
		width = head.h_bu_len / 3;
	if (OPTION('b'))
		width = head.h_bu_len;

	wdump = 64 / (width + 1);


	if (OPTION('d'))
		header();

	if (OPTION('a'))  {
		/*
		 *   Write a.out header
		 */
		aout.ao_mword = MWORD;
		aout.ao_bu_len = head.h_bu_len;
		write(afd, &aout, AOUT);
		}


	/*
	 *   Search each location counter in turn
	 *   for code of any sort to generate.
	 */
	for (i=0; i<LCOUNT; i++)  {

		if (!locn[i].l_limit)
			/* no code */
			continue;

		/*
		 *   Segment to dump - calculate start in core,
		 *   (r) and length of seg. (j).
		 */
		r = locn[i].l_rel_f;
		j = locn[i].l_limit - locn[i].l_start;

		if (OPTION('d'))  {
			len = r + j;			/* end of seg. pointer */
			printf("\n\nSEGMENT %2d  ", i);
			printf("LENGTH %5d  ", j);
			printf("START   0x%x\n\n", locn[i].l_start);

			j = 0;
			while (r < len)  {
				if (j > wdump)  {
					putchar('\n');
					newpage();
					j = 0;
					}

				printl(width, *r);
				putchar(' ');		/* spacer */
				r++;
				j++;
				}

			putchar('\n');
			newpage();
			}

		/*
		 *   Check for a.out
		 */
		if (OPTION('a'))  {
			out.ou_nseg = i;
			out.ou_start = locn[i].l_start;
			out.ou_length = locn[i].l_limit - out.ou_start;
			write(afd, &out, OUT);
			write(afd, locn[i].l_rel_f, out.ou_length*INT);
			if (OPTION('r'))  {
				/* rel info generation */
				out.ou_start = 0;
				write(afd, &out, OUT);
				write(afd, locn[i].l_reloc, out.ou_length*INT);
				}
			}

		}

	return;
}


/*
 *   Generate header lines for listing.
 */
header()
{
	register char *r;
	register int i;

	if (lpage)
		if (OPTION('f'))
			putchar('\n');
		else
			putchar('\f');

	putchar('\n');
	printf("\t\t\t\t\t\t\t\t\t\t\t\tPage %d\n", ++lpage);
	printf("MAC CROSS-ASSEMBLER   (%s)\n\n", head.h_mac);

	printf("\t\t\t%s\n\n\n", ctitle);

	lline = 7;

	return;
}


/*
 *   Check to see whether a new page is needed or not.
 */
newpage()
{
	register int title;
	register int i;

	if (intercode.i_flags & PS)  {
		i = intercode.i_op;
		if (i == 11 || i == 12)
			title = TRUE;
		}
	else
		title = FALSE;


	if (!lline && !title)
		header();

	lline = (lline + 1) % head.h_page;

	return;
}


/*
 *   Print source line.
 */
source()
{
	register int i;

	/* line number */
	printf("  %3d\t", nline);

	i = getlin();
	buf[i] = '\0';
	printf("%s", buf);

	return;
}
