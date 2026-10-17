#include <stdio.h>
#include <stat.h>

main(argc, argv)
int argc;
char *argv[];
{
	register int i;
	register int force = 1;

	for(i = 1; i < argc; ++i)
		if(strcmp(argv[i], "-c"))
			touch(force, argv[i]);
		else
			force = 0;
}

touch(force, name)
register int force;
register char *name;
{
	struct statb stbuff;
	char junk[1];
	register int fd;

	if(stat(name, &stbuff) < 0)
		if(force)
			goto create;
		else
		{
			fprintf(stderr,
				"touch: file %s does not exist.\n", name);
			return;
		}

	if( (stbuff.i_size1 == 0) && (stbuff.i_size0 == 0) )
		goto create;

	if((fd = open(name, 2)) < 0)
		goto bad;

	if(read(fd, junk, 1) < 1)
	{
		close(fd);
		goto bad;
	}
	lseek(fd, 0L, 0);
	if(write(fd, junk, 1) < 1)
	{
		close(fd);
		goto bad;
	}
	close(fd);
	return;

bad:
	fprintf(stderr, "Cannot touch %s\n", name);
	return;

create:
	if((fd = creat(name, 0666)) < 0)
		goto bad;
	close(fd);
}
