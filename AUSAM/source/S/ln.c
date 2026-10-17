#
#include	<errnos.h>
/*
 * ln target [ new name ]
 */

main(argc, argv)
char **argv;
{
	extern errno;
	register char *np;

	if (argc<2) {
		prints(2, "Usage: ln target [ newname ]\n");
		return 1;
	}
	if (argc==2) {
		np = argv[1];
		while(*np++);
		while (*--np!='/' && np>argv[1]);
		if( np == argv[1] )
		{
			prints(2, "Can't link to self\n");
			return 1;
		}
		np++;
		argv[2] = np;
	}
	if (link(argv[1], argv[2])<0) {
		if(errno == EEXIST)
			perror(argv[2]);
		  else
			perror(argv[1]);
		return 1;
	}
	return 0;
}
