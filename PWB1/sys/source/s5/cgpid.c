 /* C Library char string getpid function
    Appends zero filled 5 digit process ID to user supplied string arg.
 */
cgetpid(s) char *s; {

register char *s1, *i;

extern int ldivr;

	s1 = s;
	while(*s1) s1++;
	for(i = 1; i <= 5; ++i) *s1++ = '0';
	*s1 = '\0';
	i = getpid();
	while(i) {
		i = ldiv(0,i,10);
		*--s1 = ldivr + '0';
	}
	return(s);
}
