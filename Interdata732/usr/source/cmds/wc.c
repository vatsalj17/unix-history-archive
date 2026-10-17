#

/* wc line and word count */

#define	LONG	int		/***/

int	buf[131];			/***/
LONG wordct;
LONG twordct;
LONG linect;
LONG tlinect;

main(argc,argv)
char **argv;
{
	int i, token;
	register char *p1, *p2;
	register int c;

	i = 1;
	do {
		if(argc<=1) buf[0] = 0;
		else if(fopen(argv[i],buf)<0) {
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
				p1 = &buf[1];
				c = read(buf[0], p1, 512);
				if(c <= 0)
					break;
				p2 = p1+c;
			}
			c = 0;
			c =| *p1++;
			if(' '<c&&c<0177) {
				if(!token++) {
					++wordct;
				}
			} else {
				if(c=='\n') {
					++linect;
				}
				else if(c!=' '&&c!='\t')
					continue;
				token = 0;
			}
		}
		printf("%7s ",locv(linect));
		printf("%7s ",locv(wordct));
		printf("%s\n", argc<=1?"":argv[i]);
		close(buf[0]);
		tlinect =+ linect;
		twordct =+ wordct;
	} while(++i<argc);
	if(argc > 2) {
		printf("%7s ",locv(tlinect));
		printf("%7s ",locv(twordct));
		printf("total\n");
	}
}

diag(s)
char *s;
{
	while(*s)
		write(2,s++,1);
}

/*** locv() for integers ***/
locv(x)
int x;
{
	register n;
	register char *p;
	static cvbuf[12];

	n = x;
	p = &cvbuf[11];
	do {
		*--p = n%10 + '0';
	} while (n =/ 10);

	return(p);
}
