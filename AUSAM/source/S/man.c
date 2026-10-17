#
/*
 *		copyright 1975 ian inc.
 */

char stra[800];
char **args[100] { "nroff", "-manual", };
char dirnm[] "/srce/usr/docmention/man/man1";
#define zz (sizeof dirnm - 2)

main(argc,argv)
int argc;
char **argv;
{
	register int i = 1;
	register int o = 2;
	char register *s = stra;

	if( argc < 2 )
		return 1;

	if( (*argv[i]>='0') && (*argv[i]<='8') && (argv[i][1]==0) )
		dirnm[zz] = argv[i++][0];
	if( chdir(dirnm) )
	{
		perror(dirnm);
		return 1;
	}
	while( argc > i )
	{
		args[o++] = s;
		while(*s++ = *argv[i]++);
		i++;
		s[-1] = '.';
		*s++ = dirnm[zz];
		*s++ = 0;
	}
	args[o] = 0;
	execv("/bin/nroff",args);
	perror("nroff");
}
