/*
 * coredump /dev/disk blockoffset length targetfile
 * T_NOCHK to read raw disk
 */
 

#define CHUNK	(64*1024)

char buf[CHUNK];

main(ac, av)
char **av;
{
	long offset, len;
	int fd, fd2;
	
	if(ac != 5)
		fatal("usage: coredump dumpdisk blockoffset blocklen corefile");

	offset = atoi(av[2])*512;
	len = atoi(av[3])*512;
	fd = open(av[1], 2);
	if(fd == -1) 
		fatal(av[1]);
	fd2 = creat(av[4], 0644);
	if(fd2 == -1)
		fatal(av[4]);
	if(offset != lseek(fd, offset, 0))
		fatal("lseek");
	pump(fd, fd2, len);
	write(2, "core OK\n", 8);
	exit(0);
}
pump(in, out, count)
{
	int n;

	while(count > 0) {
		n = min(count, CHUNK);
		n = read(in, buf, n);
		if(n <= 0)
			fatal("read");
		if(write(out, buf, n) != n)
			fatal("write");
		count -= n;
	}
}
fatal(s)
char *s;
{
	extern int errno;

	if(errno != 0) perror(s);
	else {
		write(2, s, sizeof(s));
		write(2, "\n", 1);
	}
	exit(1);
}
