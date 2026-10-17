/*	Check Sum File
 *
 *	Modified : - 4/76 - by:  P. Wandzilak
 */

main(argc,argv)
int	argc;
char	**argv;
{

	struct sum {
			int	high;
			int	low;
	};

	int	dev, blcnt, num;
	register i;
	long	sum, temp;
	char	buf[512];

	if(argc < 2)
		error("USAGE: sum [ arguments . . . ]");

	while(argc-- > 1) {
		argv++;
		blcnt = sum.low = sum.high = 0;
		if((dev = open(*argv,0)) < 0)
			error("Can't open %s",*argv);

		while(( num = read(dev,&buf[0], 512)) > 0) {
			blcnt =+ 1;
			for(i=0;i<num;i++) {
				temp.low = buf[i];
				sum =+ temp;
			}
		}
		if( num < 0 )
			error("Read Error -- %s",*argv);
		else {
			while(sum.high) { /* add carry bits */
				temp.low = sum.high;
				sum.high = 0;
				sum =+ temp;
			}
			printf("%s %l %l \n",*argv,blcnt,sum.low);
		}

		close(dev);
	}
	exit(0);
}

error(s1,s2)
char	*s1,	*s2;
{

	printf(s1,s2);
	printf("\n");
	exit(1);
}
