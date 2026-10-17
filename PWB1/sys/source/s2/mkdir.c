/*
 * mkdir -- make directories
 */

main(argc, argv)
char **argv;
{
	static char dname[128], pname[128];
	static nerror;
	register char *p1, *p2, *p3;

	while (--argc > 0) {
		argv++;
		p1 = argv[0];
		p2 = dname;
		p3 = pname;
		while (*p2++ = *p3++ = *p1++);
		p2[-1] = '/';
		*p2++ = '.';
		*p2 = '\0';
		while (p3>pname && *--p3!='/');
		if (p3 > pname)
			*p3 = '\0';
		else if (*p3 == '/')
			p3[1] = '\0';
		else {
			*p3++ = '.';
			*p3++ = '\0';
		}
		if (access(pname, 02)) {
			perror("mkdir");
			nerror++;
			continue;
		}
		if (mknod(argv[0], 0140755, 0) < 0) {
			perror("mkdir");
			nerror++;
			continue;
		}
		chown(argv[0], ((getgid()&0377)<<8) + (getuid()&0377));
		link(argv[0], dname);
		*p2++ = '.';
		*p2 = '\0';
		link(pname, dname);
	}
	return(nerror);
}
