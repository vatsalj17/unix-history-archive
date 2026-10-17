# include	<ingres.h>
# include	<aux.h>
# include	<tree.h>
# include	<symbol.h>
# include	"globs.h"
# include	<sccs.h>

SCCSID(@(#)prlinks.c	8.1	12/31/84)


prlinks(label,linkmap)
char	*label;
char	linkmap[];
{

	register char	*lm;
	register int	i;

	printf("\n%s: ", label);
	lm = linkmap;
	for (i = 0; i < MAXDOM; i++)
		if (*lm++)
			printf("dom %d,", i);
	putchar('\n');
}
