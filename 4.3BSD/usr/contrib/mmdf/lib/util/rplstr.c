/*      replace an alloc'd string with a new one                */

rplstr (oldstr, newstr)
    char **oldstr,
	  *newstr;
{
    if (*oldstr != 0)
	free (*oldstr);

    *oldstr = newstr;
}

