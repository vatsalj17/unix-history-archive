# include	"/usr/sys/ildr.h"
# define	I1MASK	0377
struct {
	char	name[8];
	int	type;
	char	*value;
} nl[3];
int	ildev, mem;
main()
{
	int	res,k;
	int	ind;		/* index in Locktable */
	char	opt;		/* option flag */
	struct Lockreq	buf;
	char	*sp;

	givehelp();
	ildev = open("/dev/lock",1);
	mem = open("/dev/mem",0);
	setup(nl[0].name,"_Locktab");
	setup(nl[1].name,"_Lockset");
	nlist("/unix",nl);
	for ( ; ; )
	{
		printf(" type in option ");
		opt = getchar();
		getchar();
		switch (opt)
		{
		case 'a':
			dumpall();
			break;

		  case 'w':
			sp = buf.lr_key;
			for (k = 0; k < KEYSIZE; k++)
				*sp++ = 0;
			printf("Type in Action (1,2 set lock;3 release lock; 4 release all locks for pid ");
			buf.lr_act = readint();
			if (buf.lr_act < 4)
			{
				printf("Type in lock type (0,1,2,3) ");
				buf.lr_type = readint();
				if (buf.lr_act <3)
				{
					printf("Type in mod (1,2) ");
					buf.lr_mod = readint();
				}
				printf("Type in key ");
				sp = buf.lr_key;
				readkey(sp);
			}
			sp = &buf.lr_act;
			for (k = 0; k < 10; k++)
				printf("%d,",sp[k]);
			printf("\n");
			res = write(ildev,sp,KEYSIZE + 3);
			printf(" Lockresult %d \n", res);
			break;

		  case 'h':
			givehelp();
			break;

		  case 'r':
			close(ildev);
			break;

		  case 'e':
			exit();
			break;
		  
		  default:
			printf( " bad option \n");
		}
	}
}
givehelp()
{
	printf(" the following options are supported: \n");
	printf("	a-dump all entries\n");
	printf("	w-write to locktable, e-end\n");
	printf("	r-release all locks, h-help\n");
}
/*	test support for ILOCK driver.
 *
 */
dumptable(ll)
struct	Lockform	*ll;
{
	register int	j;

	printf(" pid %d wflag %d type %d mod %d \nkey ", ll->l_pid,
		ll->l_wflag,ll->l_type,
		ll->l_mod);    

	for (j = 0; j < KEYSIZE; j++)
	{
		printf(" %d",ll->l_key[j]);
	}
	printf("\n");
}
dumpall()
{
	int	k;
	char	tabbuf[KEYSIZE + 5];
	char	*ll;
	int nlox[4];

	seek(mem,nl[0].value,0);
	for (k = 0; k < NLOCKS; k++)
	{
		ll = tabbuf;
		read(mem,ll,KEYSIZE + 6);
		dumptable(ll);
	}
	seek(mem,nl[1].value,0);
	read(mem,nlox,8);
	printf(" nlocks=%d plocks=%d",nlox[0],nlox[1]);
	printf(" rlocks=%d dlocks=%d",nlox[2],nlox[3]);
	printf("\n");
}
/*
 *	read in one int number
 */
readint()
{ int l;
  char c, word[10];

	for (l = 0; l < 10; l++)
		word[l] = 0;
	l = 0;
	while ((c = getchar()) != '\n' )
	  { word[l] = c;
	    l++;
	  }
	word[++l] = '\0';
	l = atoi(word);
	return(l);
	}
/*
 *	read in a string of characters
 */
readstring(p)
char	*p;
{


	while (( *p++ = getchar()) != '\n');
	*p--;
	*p = '\0';
}
wakeup(kkk)
int kkk;
{
}
setup(p,s)
char *p, *s;
{
	while (*p++ = *s++);
}
readkey(s)
char	*s;
{
	int	i,j,k;
	printf(" length ");
	i = readint();
	for (k = 0; k < i; k++)
	{
		printf(" next ");
		j = readint();
		*s++ = j & I1MASK;
	}
}
