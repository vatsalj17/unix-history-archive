	/* chgrp gid file [file...]	*/

struct {
	int idev;
	int ino;
	int flags;
	char nlinks;
	char uid;
	char gid;
	char size0;
	char *size1;
	int addr[8];
	int atime[2];
	int mtime[2];
} statbuf;

char buff[100];

main(argc, argv)
char *argv[];
{
	register char **filep;
	register count, owner;
	register char *p;

	if ((count = argc-2) <= 0) {
		printf("Usage: chgrp gid file [file...]\n");
		exit(1);
	}
	if ((owner = atoi(argv[1])) <= 0 && argv[1][0] != '0')
		if (getpw(argv[1], buff)) {
			printf("%s: illegal group name\n", argv[1]);
			exit(1);
		} else {
			p = buff;
			while (*p++ != ':')
				;
			while (*p++ != ':')
				;
			owner = atoi(p);
		}
	owner =& 255;

	for (filep = &argv[2]; count--; filep++) {
		if (stat(*filep, &statbuf) < 0) {
			printf("%s: can't find\n", *filep);
			continue;
		}
		if (chown(*filep, owner<<8 | statbuf.uid) < 0)
			perror("Chown");
	}
}

getpw(name, buf)
char *name, *buf;
{
	static int pwbuff[131];
	register r, c;
	register char *gnp, *rnp;

	r = 1;
	if((pwbuff[0] = open("/etc/group", 0)) < 0)
		return(1);
loop:
	gnp = name;
	rnp = buf;
	while((c=getc(pwbuff)) != '\n') {
		if(c <= 0)
			goto ret;
		*rnp++ = c;
	}
	*rnp++ = '\0';
	rnp = buf;
	while (*gnp++ == *rnp++);
	if (*--gnp != '\0' || *--rnp != ':')
		goto loop;
	r = 0;
ret:
	close(pwbuff[0]);
	pwbuff[1] = 0;
	pwbuff[2] = 0;
	return(r);
}
