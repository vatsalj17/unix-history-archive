static char Sccsid[] "@(#)cat	1.1";
/*
	Concatenate strings.
 
	cat(destination,source1,source2,...,sourcen,0);
 
	returns destination.
*/

cat(dest,source)
char *dest, *source;
{
	register char *d, *s, **sp;

	d = dest;
	for (sp = &source; s = *sp; sp++) {
		while (*d++ = *s++) ;
		d--;
	}
	return(dest);
}
