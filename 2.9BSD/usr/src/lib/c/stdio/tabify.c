/*	@(#)tabify.c	2.1	SCCS id keyword	*/
# include	<stdio.h>

# define	reg	register

tabify(cp)
reg char	*cp; {

	reg int		dcol, ocol;

	ocol = 0;
	dcol = 0;
	for (;;) {
		switch (*cp) {
		  case ' ':
			dcol++;
			break;
		  case '\t':
			dcol =+ 8;
			dcol =& ~07;
			break;
		  default:
			while (((ocol + 8) &~ 07) <= dcol) {
				if (ocol + 1 == dcol)
					break;
				putchar('\t');
				ocol =+ 8;
				ocol =& ~07;
			}
			while (ocol < dcol) {
				putchar(' ');
				ocol++;
			}
			putchar(*cp);
			ocol++;
			dcol++;
			break;
		  case '\0':
			putchar('\n');
			return;
		}
		cp++;
	}
}
