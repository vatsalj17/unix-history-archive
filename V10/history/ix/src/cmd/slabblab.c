#include <fio.h>
#include <libc.h>
#include <nlist.h>

char list[] = "/etc/slabblablist";
struct nlist nl[2000];
int nlx = 0;

main(ac, av)
char **av;
{
	long x, y;
	char *s;
	int i, fd;

	fd = open(list, 0);
	if(fd == 0) perror(list), exit(1);

	Finit(fd, (char *)0);
	while(s = Frdline(fd)) {
		if(s = strtok(s, " \t\n"))
			nl[nlx++].n_name = strdup(s);
		if(nlx >= 2000) break;
	}
	close(fd);
	nlist(ac<2?"/unix":av[1],nl); 
	fd = open("/dev/kmem", 0);

	for(i=0; i<nlx; i++) {
		if(nl[i].n_type == 0) continue;
		x = nl[i].n_value;
		if(x != lseek(fd, x, 0))
			continue;
		if(4 != read(fd, &y, 4))
			perror("kmem");
		printf("%8d	%s\n", y, nl[i].n_name);
	}
}
