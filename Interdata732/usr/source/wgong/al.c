/*
 *   al:  allocate empty files
 *
 *   usage:  al file1 ... [ -mode ] filen filem [ -mode ] ...
 *
 *   author:  ross nealon  (uow)
 */
int	fd;
int	mode;
char	c;
char	*name;

main(argc, argv)
char	**argv;
{

	if (argc < 2)  {
		printf("usage: al [-mode] filename ...\n");
		exit(0);
		}

	mode = 0666;
	while (--argc)  {
		name = *++argv;
		if (*name == '-')  {
			mode = 0;
			while((c = *++name) >= '0' && c <= '7')
				mode = (mode << 3) + (c - '0');

			if (mode > 0777)  {
				printf("bad file mode: %o \n", mode);
				exit(1);
				}

			continue;
			}

		fd = creat(name, mode);
		if (fd < 0)  {
			printf("can't create %s\n", name);
			exit(1);
			}

		close(fd);
		}
	exit(0);
}
