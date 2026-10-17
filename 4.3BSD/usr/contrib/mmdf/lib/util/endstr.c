/*    check for EXACT match of suffix string at end of thestring          */

endstr (suffix, thestring, suflen)
    register char   *suffix,
		    *thestring;
    register short    suflen;
{                                 /* does str end with suffix string?     */
    short thelength;

    if (suflen < 0)
	suflen = strlen (suffix);
    thelength = strlen (thestring);

    if (suflen > thelength)
	return (0);

    return (initstr (suffix, &thestring[thelength - suflen], suflen));
}
