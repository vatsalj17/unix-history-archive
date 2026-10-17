int argi;
int ldivr;
main(argc, argv)
int argc;
char *argv[];
{
	register char *cp;
	register wd;

	argc--;
	for(argi=1; argi<=argc; argi++) {
		for(cp=argv[argi]; *cp ; ++cp) {
			if(*cp=='\\') {
				if(*++cp=='n') {
					putchar('\n');
					continue;
				} else if(*cp=='c')
					exit(0);
				else if(*cp=='\\'){
					putchar('\\');
					continue;
				}else if(*cp=='0') {
					wd = 0;
					while(*++cp>='0' && *cp<='7') {
						wd =<< 3;
						wd =| (*cp-'0');
					}
					putchar(wd);
					--cp;
					continue;
				} else --cp;
			}
			putchar(*cp);
		}
		putchar(argi==argc? '\n': ' ');
	}
	exit(0);
}
