/*
 * mv file1 file2
 */
struct
{
	int	dev;
	int	inum;
	int	imode;
	char	nlink;
	char	uid;
	char	gid;
	char	siz0;
	char	siz1;
	int	addr[8];
	int	adate[2];
	int	mdate[2];
} sbuf1, sbuf2;
char strbuf[100];

main(argc, argv)
char *argv[];
{
	char **argp;
	char *p1, *p2, *source, *target;
	int i;
	int b;

	argp = argv;
	if(argc != 3) {
		printf("mv: Usage: mv name1 name2\n");
		return(1);
	}
	source = argp[1];
	target = argp[2];
	if(stat(source, &sbuf1) < 0) {
		printf("mv: %s non-existent\n", source);
		return(1);
	}
	if((sbuf1.imode & 060000) == 040000)
		goto mvdir;
	setuid(getuid());
	if(stat(target, &sbuf2) >= 0) {
		if((sbuf2.imode & 060000) == 040000) {
			p2 = strbuf;
			while(*p2++ = *target++)
				;
			p2[-1] = '/';
			target = argp[1];
			p1 = argp[1];
			while(*target)
				if(*target++ == '/')
					p1 = target;
			while(*p2++ = *p1++)
				;
			target = strbuf;
		}
		if(stat(target, &sbuf2) >= 0) {
			if(sbuf1.dev == sbuf2.dev && sbuf1.inum == sbuf2.inum) {
				printf("mv: %s and %s are identical\n", source, target);
				return(1);
			}
			if(access(target, 2)<0  && ttyn(0)!='x') {
				printf("mv: %s: %o mode ", target,
					sbuf2.imode & 07777);
				i = b = getchar();
				while(b != '\n' && b > 0)
					b = getchar();
				if(i != 'y')
					return(1);
			}
			if(unlink(target) < 0) {
				printf("mv: Cannot unlink %s\n", target);
				return(1);
			}
		}
	}
	if(link(source, target) < 0) {
		i = fork();
		if(i == -1) {
			printf("mv: Try again\n");
			return(1);
		}
		if(i == 0) {
			execl("/bin/cp", "cp", source, target, 0);
			printf("mv: No cp\n");
			return(1);
		}
		while(wait(&b) != i)
			;
		if(b != 0)
			return(1);
	}
	if(unlink(source) < 0) {
		printf("mv: Cannot unlink %s\n", source);
		return(1);
	}
	return(0);

mvdir:
	if(stat(target, &sbuf2) >= 0) {
		printf("mv: Directory %s exists\n", target);
		return(1);
	}
	p1 = source;
	p2 = target;
	while(*p1 == *p2) {
		p1++;
		if(*p2++ == 0) {
			printf("mv: ?? source == target, source exists and target doesnt\n");
			return(1);
		}
	}
	while(*p1)
		if(*p1++ == '/') {
			printf("mv: Directory rename only\n");
			return(1);
		}
	while(*p2)
		if(*p2++ == '/') {
			printf("mv: Directory rename only\n");
			return(1);
		}
	if (*--p1=='.' && p1==source
	 || *--p1=='.' && p1==source) {
		printf("mv: Cannot rename %s\n", source);
		return(1);
	}
	b = 0;
	p1 = source;
	while(*p1)
		if(*p1++ == '/') {
			p2 = p1;
			b++;
		}
	p1 = source;
	if(b == 0) {
		p1 = ".";
		p2 = p1+2;
	}
	*--p2 = '\0';
	if(access(p1, 2) < 0) {
		printf("mv: No write access to %s\n", p1);
		return(1);
	}
	*p2 = '/';
	if(link(source, target) < 0) {
		printf("mv: ?? directory link failed\n");
		return(1);
	}
	if(unlink(source) < 0) {
		printf("mv: ?? directory unlink failed\n");
		return(1);
	}
	return(0);
}

getchar()
{
	int c;

	c = 0;
	read(0, &c, 1);
	return(c);
}
