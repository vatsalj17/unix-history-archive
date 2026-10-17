/* wc line and word count */

int	buf[256];
int	file;
int	lflg;
long int wordct;
long int linect;

main(argc,argv)
char **argv;
{
	int i, token;
	register char *p1, *p2;
	register int c;

	if(--argc>0 & **++argv=='-') {
		if(argv[0][1]=='l')
			lflg++;
		argc--;
		argv++;
	}
	i = 0;
	do {
		if(argc<=0) buf[0] = 0;
		else if((file=open(argv[i],0))<0) {
			diag(argv[i]);
			diag(": cannot open\n");
			continue;
		}
		p1 = 0;
		p2 = 0;
		linect = 0;
		wordct = 0;
		token = 0;
		for(;;) {
			if(p1 >= p2) {
				p1 = &buf;
				c = read(file, p1, 512);
				if(c <= 0)
					break;
				p2 = p1+c;
			}
			c = 0;
			c =| *p1++;
			if(' '<c&&c<0177) {
				if(!token++) {
					wordct++;
				}
			} else {
				if(c=='\n') {
					linect++;
				}
				else if(c!=' '&&c!='\t')
					continue;
				token = 0;
			}
		}
		printf("%7s ",locv(linect));
		if(lflg==0)
		printf("%7s ",locv(wordct));
		printf("%s\n", argc<=0?"":argv[i]);
		close(file);
	} while(++i<argc);
}

diag(s)
char *s;
{
	while(*s)
		write(2,s++,1);
}

putchar(c)
{
	write(1,&c,1);
}
