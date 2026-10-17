static char Sccsid[] "@(#)strend	1.1";
strend(p)
register char *p;
{
	while (*p++)
		;
	return(--p);
}
