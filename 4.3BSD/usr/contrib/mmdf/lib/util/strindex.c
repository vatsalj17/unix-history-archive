strindex (str, target)           /* return column str starts in target */
register char   *str,
		*target;
{
    char *otarget;
    register short slen;

    for (otarget = target, slen = strlen (str); ; target++)
    {
	if (*target == '\0')
	    return (-1);

	if (equal (str, target, slen))
	    return (target - otarget);
    }
}
