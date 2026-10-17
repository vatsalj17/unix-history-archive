#include	<sys/types.h>
#include	<ndir.h>

main()
{
	DIR *dirf;
	struct direct *e;

	if(dirf = opendir("."))
		while(e = readdir(dirf))
			printf("%s\n", e->d_name);
	exit(0);
}
