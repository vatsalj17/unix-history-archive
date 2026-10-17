# include "stdio.h"

/* this is the source for /usr/lib/spell3
/* it is recreated by
	cc -O spell3.c -lS -o spell3
/* it takes its argument to be a file with words on it, and the standard
	 input to be descriptions of words that were found in the dictionary,
	 and it outputs the appropriate output on the standard output */

char word[100], suff[100];
int badflag;
FILE *wdfile;
FILE *histfile;
int numb, innumb, inlev;
int vflag;

main( argc, argv ) int argc; char *argv[]; {
	wdfile = fopen( argv[1], "r" );
	histfile = fopen(argv[2],"a");
	if( argc>3 ) vflag = 1;

	numb = 0;

	if( scanf( "%d%d%s%d", &innumb, &inlev, suff, &badflag ) < 0 )
			innumb = 32000;

	while( fgets( word, 100, wdfile ) ) {
		++numb;
		if( innumb>numb && word[0]!='\n' && word[1]!='\n' ) {
			printf( "%s%s", vflag? "****\t" : "", word );
			if(histfile != NULL) fprintf( histfile, "%s", word );
		}
		if( innumb == numb ){
			if( badflag ) printf( "%s%s", vflag?"****\t":"", word );
			else if( inlev && vflag ) printf( "%s\t%s", suff, word );
			while( innumb == numb )
				if( scanf( "%d%d%s%d", &innumb, &inlev, suff, &badflag ) < 0 )
					innumb = 32000;
			}
		}
	fclose(stdout);
	fclose(histfile);
	}

