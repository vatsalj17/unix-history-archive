#define	nmag	0177545
#define	omag	0177555

struct
{
	char	name[14];
	long	ndate;
	char	nuid;
	char	ngid;
	int	mode;
	int	siz0;
	int	siz1;
} nh;

struct
{
	char	oname[8];
	long	odate;
	char	ouid;
	char	omode;
	int	siz;
} oh;

char	*tmp;
int	f;
int	tf;
int	buf[256];

main(argc, argv)
char *argv[];

{
	register i;

	tmp = mktemp("/tmp/arcXXXX");
	for(i=1; i<4; i++)
		signal(i, 1);
	for(i=1; i<argc; i++)
		conv(argv[i]);
	unlink(tmp);

	return( 0 );
}

conv(fil)
char *fil;

{
	register i, n;

	f = open(fil, 2);
	if(f < 0)
	{
		perror( fil );
		return;
	}
	close(creat(tmp, 0600));
	tf = open(tmp, 2);
	if(tf < 0)
	{
		perror( tmp );
		close(f);
		return;
	}
	buf[0] = 0;
	read(f, buf, 2);
	if(buf[0] != omag)
	{
		printf("not archive format\n");
		close(tf);
		close(f);
		return;
	}
	buf[0] = nmag;
	write(tf, buf, 2);
loop:
	i = read(f, &oh, sizeof(oh));
	if(i != sizeof oh)
		goto out;
	for(i=0; i<8; i++)
		nh.name[i] = oh.oname[i];
	nh.siz1 = oh.siz;
	nh.nuid = oh.ouid;
	nh.ngid = 1;
	nh.mode = 0666;
	nh.ndate = oh.odate;
	n = ((oh.siz+1) >> 1) & 077777;
	write(tf, &nh, sizeof(nh));
	while(n > 0)
	{
		i = 256;
		if(n < i)
			i = n;
		read(f, buf, i+i);
		write(tf, buf, i+i);
		n =- i;
	}
	goto loop;
out:
	seek(f, 0, 0);
	seek(tf, 0, 0);
	while((i=read(tf, buf, 512)) > 0)
		write(f, buf, i);
	close(f);
	close(tf);
}
