		/*	cat [file...]	*/

int fin[131];
int fout[131]	{ 1 };

char *dash { "-" };

main(argc, argv)
char **argv;
{
	register c, count;
	register char *file, **filep;

	if (argc == 1) {
		filep = &dash;
		count = 1;
	} else {
		filep = argv + 1;
		count = argc - 1;
	}

	while (count--) {
		if (*(file = *filep++) == '-')
			fin[0] = dup(0);
		else
			if ((fin[0] = open(file, 0)) < 0)
				continue;
		while ((c = getc(fin)) >= 0)
			putc(c, fout);
		close(fin[0]);
	}
	flush(fout);
}
