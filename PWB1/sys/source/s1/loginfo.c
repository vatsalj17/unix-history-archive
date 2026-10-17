main(argc, argv) char **argv; {
	if(equal(*argv, "logname")) printf("%s\n", logname());
	if(equal(*argv, "logdir")) printf("%s\n", logdir());
	if(equal(*argv, "logtty")) printf("tty%s\n", logtty());
}

equal(as1, as2)
char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while(*s1++ == *s2)
		if(*s2++ == '\0')
			return(1);
	return(0);
}
