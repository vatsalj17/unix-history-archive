struct
{
	int integer;
};

char macro[]	"/bin/macro";
char link[]	"/bin/link";
char execute[]	"m.out";
char sdump[]	"/bin/sdump";

char stat[2];

char *arg[12]
{
	link+5,
	"-o=m.out",
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
int n	2;		/* first empty space in arg */

main(c,v)
char *v[];
{
	register i;

	if( c < 2 )
	{
		prints(2, "Usage: ass [-options] name1[.mac] ...\n");
		exit(1);
	}
	if( !fork())
	{
		v[c] = 0;
		v[0] = macro+5;
		execv( macro, v);
		efail(macro);
	}
	waitx(stat);
	if( stat->integer)
	{
		prints(2, "Assembly errors - execution cancelled\n");
		exit(1);
	}
	if( !fork())
	{
		register char *p,*q;

		v[c] = 0;
		for(i = 1; v[i]; i++)
		{
			if(v[i][0] != '-')
			{
				arg[n++] = q = v[i];
				p = 0;
				while( *q)
				{
					if( *q++ == '.')
					{
						p = q-1;
					}
				}
				if( p)
					*p = 0;
				break;
			}
			else
			{
				if(v[i][1] == 'l' && v[i][2] == 's')
					arg[n++] = "-ls";
			}
		}
		arg[n++] = "/lib/m.obj";
		arg[n] = 0;
		execv(link, arg);
		efail(link);
	}
	waitx(stat);
	if(stat->integer)
	{
		prints(2, "Linkage failed - execution cancelled\n");
		exit(1);
	}
	if( !fork())
	{
		prints(2, "Execution begins -\n");
		tlimit(15);
		execl(execute, execute, 0);
		efail(execute);
	}
	waitx(stat);
	if(stat[0] & 0200)	/* core? */
	{
		execl( sdump, sdump+9, 0);
		efail(sdump);
	}
}


efail(s)
char *s;
{
	prints(2, s);
	prints(2, ": Exec failed\n");
	exit(1);
}
