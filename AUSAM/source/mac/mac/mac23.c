#include	"mac.h"
#include	"mac.x"


/*
 *   Pseudo opcode line listing.
 */
plist(lc)
register int lc;
{
	register struct st *q;
	register int i;
	register int l;
	register int ps;

	if (!OPTION('l'))
		/* no listing required */
		return;

	ps = intercode.i_op;

	/*
	 *   Call header() if at top of page, and instruction
	 *   to be listed is not 'title' or 'eject'.
	 */
	newpage();

	switch (ps)  {

		case 0:		/* dc's */
		case 1:
		case 2:
		case 3:
			l = head.h_bu_len;
			if (OPTION('h'))
				l =/ 4;
			else
				l =/ 3;

			ps = head.dctype[ps].f_len;
			printl(8, lc);
			printf("   ");

			for (i=0; i<ps; i++)
				printl(l, *cp++);

			/* padding */
			i = 20 - (ps * l);
			while (i--)
				putchar(' ');

			break;

		case 13:	/* dc special */
			l = head.h_bu_len;
			if (OPTION('h'))
				l =/ 4;
			else
				l =/ 3;

			printl(8, lc);
			printf("   ");
			for (i=0; i<length; i++)
				printl(l, *cp++);

			i = 20 - (length * l);
			while (i-- > 0)
				putchar(' ');

			break;

		case 6:		/* align */
		case 7:		/* org */
		case 9:		/* ds */
		case 10:	/* end */
			printl(8, lc);
			for (i=0; i<23; i++)
				putchar(' ');
			break;

		case 4:		/* equ */
		case 14:	/* struc */
		case 15:	/* ends  */
			i = intercode.i_label;
			if (i != ERR)  {
				q = &symtab[i];
				printl(8, q->s_value);
				}
			else
				printf("        ");
			for (i=0; i<23; i++)
				putchar(' ');
			break;

		case 5:		/* global */
			for (i=0; i<31; i++)
				putchar(' ');
			break;

		case 8:		/* seg */
			printf("  %3d", lcntr);
			for (i=0; i<26; i++)
				putchar(' ');
			break;

		case 11:	/* title */
		case 12:	/* eject */
			getlin();
			header();
			return;

		}

	source();			/* print line of source */

	return;
}

/*
 *   Dud line listing - just print source.
 */
tlist(lc)
{
	register int i;
	register int l;

	if (!OPTION('l'))
		/* listing not required */
		return;

	newpage();

	if ((l = intercode.i_label) != ERR)
		printl(8, lc);
	else
		printf("        ");

	for (i=0; i<23; i++)
		putchar(' ');

	source();

	return;
}

/*
 *   Opcode listing.
 */
olist(lc)
{
	register int i;
	register int l;
	register int len;


	if (!OPTION('l'))
		/* listing not required */
		return;

	newpage();

	/*  location counter  */
	printl(8, lc);
	printf("   ");

	/*  instruction  */
	len = head.h_bu_len;
	if (OPTION('h'))
		len =/ 4;
	else
		len =/ 3;

	for (i=0; i<length; i++)
		printl(len, *cp++);

	/*  padding  */
	l = 20 - (length * len);
	for (i=0; i<l; i++)
		putchar(' ');

	/* remainder of line */
	source();

	return;
}
