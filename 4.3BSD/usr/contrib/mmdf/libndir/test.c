#include <sys/types.h>
#include "dir.h"

main(argc, argv)
char **argv;
{
	int	len;
	DIR	*dirp;
	struct	direct *dp;

	len = strlen( argv[1] );
	dirp = opendir( "." );
	for( dp = readdir(dirp); dp != NULL; dp = readdir(dirp) )
		if( dp->d_namlen == len && !strcmp(dp->d_name, argv[1])){
			closedir(dirp);
			printf("Found %s.\n", dp->d_name);
			exit(0);
		}
	closedir(dirp);
	printf("Not found.\n");
	exit( 1 );
}
