/* quickly coded bmove for any machine */

/* @(#)bmove.SUN.c	8.1	(INGRES) 12/8/85 */

bmove(s, d, l)
char	*s, *d;
int	l;
{
	bcopy(s, d, l);
}
