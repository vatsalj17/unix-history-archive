/*
 *   dsw:  delete files interactivly
 *   author:  ross nealon  (uow)
 */

char	c;
char	cc;
char	buf[40];

main(argc, argv)
char	*argv[ ];
{
	register  fd, i;

	if (argc == 2)
		if (chdir(argv[1]) < 0)  {
			printf("dsw: can't chdir\n");
			exit(1);
			}
	fd = open(".", 0);

	if (fd < 0)  {
		printf("dsw: can't open directory\n");
		exit(1);
		}

	/*
	 *   skip entries for . and ..
	 */
	read(fd, buf, 32);
	for (;;)  {
		i = read(fd, buf, 16);
		if (!i)  {
			printf("dsw: end of directory\n");
			exit(0);
			}

		if (!buf[0] && !buf[1])
			continue;

	  retry:
		printf("%s - ", &buf[2]);
		cc = c = getchar();
		while (cc != '\n')  cc = getchar();

		switch (c)  {
			case 'y':
			case 'Y':
				unlink(&buf[2]);
				break;

			case '\n':
				break;

			case 'x':
			case 'X':
				exit(0);

			default:
				goto retry;
			}
		}
	exit(1);
}
