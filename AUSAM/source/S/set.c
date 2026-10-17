#define	B1200	9
main(argc,argv)
char **argv;
int argc;
{
	struct
	{
		char ispeed,ospeed;
		char erase,kill;
		int mode;
	} arg;

	if( gtty( 1 , &arg ) == -1 )
	{
		perror("gtty");
		return 1;
	}
	if( arg.ispeed >= B1200 )
	{
		arg.erase = 010; /* backspace */
		if( stty( 1 , &arg ) == -1 )
		{
			perror("stty");
			return 1;
		}
	}
	if( argv[0][0] == '-')
	{
		execl( "/bin/sh" , "-" , 0 );
		return 1;
	}
	return 0;
}
