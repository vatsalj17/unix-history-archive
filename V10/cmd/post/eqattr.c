eqattr(a1, a2)
register char	*a1, *a2;
{
	while (*a1 && *a1 == *a2)
		a1++, a2++;
	return (!*a1 && *a2 == '=') || (!*a1 && !*a2)
	    || (!*a2 && *a1 == '=');
}
