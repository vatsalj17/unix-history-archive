/*  Check for EXACT match prefix string at beginning of thestring       */

initstr (prefix, thestring, preflen)
    register char   *prefix,
		    *thestring;
    register short     preflen;
{

    if (preflen < 0)
	preflen = strlen (prefix);

    while (*prefix++ == *thestring++)
	if (--preflen == 0)
	    return (1);
    return (0);
}
