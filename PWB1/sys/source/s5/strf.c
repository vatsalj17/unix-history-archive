/* stringf lives! (drat it) */
char xstring[128];

stringf(args)
{
	sprintf(xstring, "%r", &args);
	return &xstring;
}
