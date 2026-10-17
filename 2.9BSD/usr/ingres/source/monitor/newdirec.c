# include	"../ingres.h"
# include	"../aux.h"
# include	"../fileio.h"
# include	"monitor.h"

/*
**  CHANGE WORKING DIRECTORY
*/

newdirec()
{
	register char	*direc;

	direc = getfilename();
	if (chdir(direc))
		printf("Cannot access directory \"%s\"\n", direc);
}
