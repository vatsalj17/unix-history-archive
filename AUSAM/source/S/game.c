	/*
	   a program to invoke games,
	   SO as to allow access to games,
	   BUT not to anything else,
	   ASSUMES process limit for games account
	   of 2.
	*/
main()
{
	char buf[100];
	register n;

	nice(1);
	goto prompt;

	while( (n=read(0,buf,sizeof buf)) > 1 )
	{
		buf[n-1] = 0;	/* null terminate input name */
		for( n=0; buf[n] ; n++ )
			if( buf[n] =='/' ) exit();
		switch( fork() )
		{
		case -1:	/* fork fail */
			exit(1);
		case 0:		/* child */
			execl(buf,buf,0);
			exit(1);
		default:	/* parent() */
			while( waitx(buf) != -1 );
			break;
		}
	 prompt:
		prints(2,"which game do you wish to play ? ");
	}
}
