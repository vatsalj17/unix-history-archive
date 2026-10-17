int	fi;
int	fo;
int	o;
int	n;
int	null;
int	error;
char	ibuf[100];
char	obuf[100];
char	nbuf[100];
int	buf[256];

main()
{

loop1:
	printf("to: ");
	string(obuf);
	if(null)
		exit();
	fo = open(obuf, 1);
	if(fo < 0) {
		fo = creat(obuf, 0666);
		if(fo < 0) {
			perror("creat");
			goto loop1;
		}
		printf("%s created\n", obuf);
	}
	printf("offset: ");
	o = number();
	if(error) {
		close(fo);
		goto loop1;
	}
	seek(fo, o, 3);

loop2:
	printf("from: ");
	string(ibuf);
	if(null) {
		close(fo);
		goto loop1;
	}
	fi = open(ibuf, 0);
	if(fi < 0) {
		perror("open");
		goto loop2;
	}
	printf("offset ");
	o = number();
	if(error)
		goto loop3;
	seek(fi, o, 3);
	printf("count ");
	o = number();
	if(error)
		goto loop3;
	while(o--) {
		n = read(fi, buf, 512);
		if(n < 0) {
			perror("read");
			goto loop3;
		}
		if(n == 0) {
			printf("EOF\n");
			break;
		}
		n = write(fo, buf, n);
		if(n < 0) {
			perror("write");
			goto loop3;
		}
	}

loop3:
	close(fi);
	goto loop2;
}

string(cp)
char *cp;
{
	register char *rcp;

	rcp = cp;
	do
		if(read(0, rcp, 1) != 1)
			exit();
	while(*rcp++ != '\n');
	*--rcp = '\0';
	null = cp == rcp;
}

number()
{
	register char *cp;
	register rn;

	error = 0;
	cp = nbuf;
	string(cp);
	rn = 0;
	while(*cp <= '9' && *cp >= '0')
		rn = rn*10 + *cp++ - '0';
	if(*cp != '\0') {
		printf("bad character in number\n");
		error++;
	}
	return(rn);
}
