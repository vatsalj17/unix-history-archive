#include <sys/pex.h>

main(ac, av)
char **av;
{
	if(ac>1)
		ioctl(1, FIOANPX, (struct pexclude*)0);
	else
		ioctl(1, FIOAPX, (struct pexclude*)0);
}
