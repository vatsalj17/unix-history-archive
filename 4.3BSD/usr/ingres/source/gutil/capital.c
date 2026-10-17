# include	<sccs.h>

SCCSID(@(#)capital.c	8.1	12/31/84)

capital(lower, upper)
char *lower, *upper;
{
	register char	*l, *u;

	l = lower;
	u = upper;
	while (*l)
	{
		if (*l == ' ')
		{
			do
			{
				*u++ = *l++;
			} while (*l);
			break;
		}
		if (*l >= 'a' && *l <= 'z')
			*u++ = *l++ + 'A' - 'a';
		else
			*u++ = *l++;
	}
	*u = '\0';
}
