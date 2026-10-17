#include <cplot.h>
#include	"ask.c"
#define magicnumber 497
/*
 *
 *		Fancy pattern drawer
 *
 *			Adrian Freed
 *
 */

#define	POINTMAX	200




float	x[POINTMAX], y[POINTMAX], xt[POINTMAX], yt[POINTMAX], r;
int s;
int	numpoi;

main( argc, argv )
char **argv;
int argc;
{
	float   xsmall, ysmall, xlarge, ylarge;
	FILE *fd;

	int	j,	k,	t;
	char ans[30];
	char kc;


	fd = NULL;
	init( stdout );
	swindow( 0, 1023, 0,  780 );
	uwindow( -1.0, 1.0, -1.0, 1.0 );
	while( argc-- != 1 )
	{
		argv++;
		if( (fd = fopen( *argv, "r" )) == NULL )
		{
			perror( *argv );
			continue;
		}
		if( reload( fd ) )
		{
			home();
			alpha();
			fprintf( stderr, "%s: bad format\n", *argv);
			sleep(1);
		}
		else
		{
			bell();
			genpat();
			fclose(fd);
		}
	}

	if( fd != NULL )
	{
		finish();
		exit();
	}

	for( ;; )
	{
		erase();
		srect( 10, 10, 1020, 770 );
		/*	Get pattern co-ords.	*/
		kc = 0;
		ucursor( &x[0], &y[0]);
		upointa( x[0], y[0] );
		for( numpoi = 1; kc != 'P'; numpoi++ )
		{
			kc = ucursor( &x[numpoi], &y[numpoi]);
			udrawa( x[numpoi], y[numpoi] );
		}


		numpoi--;

		for( ;; )
		{
			ask( 20, 700,"how many iterations? " , ans );
			if( (s = atoi( ans )) == 0 ) continue;
			ask( 20, 680, "spin rate? ", ans );
			r = atof( ans );

			/*	copy co-ords into temporary array	*/
			for( j=0; j<=numpoi; j++ )
			{
				xt[j] = x[j];
				yt[j] = y[j];
			}

			/*	Find minimum and maximum x and y	*/
			xsmall = ysmall = 1.0;
			xlarge = ylarge = -1.0;
			for( k = 1; k < s;  k++ )
			{

					for( j = 1; j <= numpoi; j++ )
					{
						xt[j] = xt[j] + ( xt[j-1] - xt[j] ) * (r);
						yt[j] = yt[j] + ( yt[j-1] - yt[j] ) * (r);
						if( xt[j] > xlarge ) xlarge = xt[j];
						if( yt[j] > ylarge ) ylarge = yt[j];
						if( xt[j] < xsmall ) xsmall = xt[j];
						if( y[j] < ysmall ) ysmall = y[j];
					}

					xt[0] = xt[0] - ( xt[0] - xt[numpoi+1] )*(r);
					yt[0] - yt[0] + ( yt[0] - yt[numpoi+1] )*(r);
					if( xt[0] > xlarge ) xlarge = xt[0];
					if( yt[0] > ylarge ) ylarge = yt[0];
					if( xt[0] < xsmall ) xsmall = xt[0];
					if( yt[0] < ysmall ) ysmall = yt[0];
			}

			/*	Set up window according to pattern's size	*/
			xsmall =- 0.20;
			ysmall =- 0.20;
			xlarge =+ 0.20;
			ylarge =+ 0.20;
			uwindow( xsmall, xlarge, ysmall, ylarge );

			genpat();

			ask(20, 50, "do you want that pattern? ", ans);
			if( *ans == 'y' )
			{
				ask( 20, 30, "what would you like to call it? ", ans);
				if( (fd = fopen( ans, "w" )) == NULL )
				{
					perror( ans );
				}
				else
				{
					dump( fd );
					fflush( fd );
					fclose( fd );
				}
			}
			ask(20, 10, "do you want to enter a new pattern? ", ans);
			if( *ans == 'y' ) break;


		}
		erase();
	}
	finish();
}

genpat()
{

	register int	j, k;

	erase();
	/*	replace co-ords		*/
	for( j=0; j <= numpoi; j++ )
	{
		xt[j] = x[j];
		yt[j] = y[j];
	}
	/*	Got to start somewhere		*/
	umovea( xt[0], yt[0] );

	for( k = 1; k < s;  k++ )
	{

			for( j = 1; j <= numpoi; j++ )
			{
				udrawa( xt[j], yt[j] );
				xt[j] = xt[j] + ( xt[j-1] - xt[j] ) * (r);
				yt[j] = yt[j] + ( yt[j-1] - yt[j] ) * (r);
			}

			udrawa( xt[0], yt[0] );
			xt[0] = xt[0] - ( xt[0] - xt[numpoi+1] )*(r);
			yt[0] = yt[0] - ( yt[0] - yt[numpoi+1] )*(r);
	}



}
dump( file )
FILE *file;
{

	putw( magicnumber, file );
	putw( numpoi, file );
	fwrite( &r, sizeof( r ), 1, file );
	fwrite( &s, sizeof( s ), 1, file );
	fwrite( x, sizeof( *x ), numpoi+1, file );
	fwrite( y, sizeof( *y ), numpoi+1, file );
}
reload( file )
FILE *file;
{

	if(getw(file) != magicnumber)
		return(-1);
	numpoi = getw( file );

	fread( &r, sizeof( r ), 1, file );
	fread( &s, sizeof( s ), 1, file );
	fread( x, sizeof( *x ), numpoi+1, file );
	fread( y, sizeof( *y ), numpoi+1, file );
	if(ferror(file))
		return(-1);
	return(0);
}
