/*
 * rmdir -- unlink directory
 *
 * Usage: rmdir name ...
 *
 * Note: name must not end in '.' or '..', otherwise URK!
 */

#define	R	04
#define	W	02
#define	X	01

#define WX	(W|X)

#define	IFMT	060000
#define	IFDIR	040000

char *arg;

main(ac, av)
char *av[];
{

	if(ac-- < 2)
	{
		prints(2, "Usage: rmdir name ...\n");
		exit(1);
	}
	while( ac-- )
	{
		register char *p,*q;

		p = *++av;
		arg = *av;
		q = p+1;
		/*
		 * Problems can arise when the pathname contains
		 * segments of '.' or '..'. To avoid this we will
		 * chdir to the penultimate path and then do a simple
		 * remove there. It's tough if the directory you remove
		 * happens to be your current one, but the system (now)
		 * protects against you making files there. You must not
		 * remove pathnames that terminate in '.' or '..'.
		 * Access permissions must be checked down the path
		 * as this program is setuid to ROOT.
		 */
		while( *q )
		{
			register char *r;

			r = q;
			if( *q++ == '/')
			{
				while( *q == '/' )
					q++;
				if (*q)
					p = q;
				else
					*r = 0;
			}
		}
		/*
		 * Now: Either p points to the front of the name
		 *	(it may have a slash),
		 *	Or it points to the last pathname segment.
		 */
		if( p == *av)
		{
			/*
			 * No chdir necessary
			 *
			 * Must have write permission in
			 * dir containing name
			 */
			if (*p == '/')
				q = "/";
			else
				q = "";
			if(access(q, WX) < 0)
			{
				perror(p);
				continue;
			}
			rem( p );
		}
		else
		{
			register i;

			/*
			 * A fork is required, for chdir.
			 */
			p[-1] = 0;
			if( (access(*av, WX) < 0) || (i = fork()) < 0)
			{
				p[-1] = '/';
				perror(*av);
				continue;
			}
			if( i == 0)
			{
				chdir(*av);
				p[-1] = '/';
				rem(p);
				return 0;
				/*
				 * child dies
				 */
			}
			else
			{
				int status;
				waitx(&status);
			}
		}
	}
	return 0;
}

rem(name)
register char *name;
{
	register f;
	register char *q;
	struct
	{
	int	i_dev;
	int	i_number;
	int	i_node[16];
	} stbuf;

	if( dot(name) )
	{
		prints(2, arg);
		prints(2, ": Ill formed\n");
		return;
	}
	if(access(name, W) < 0 || (f = open(name,0)) < 0)
	{
		perror(arg);
		return;
	}
	fstat(f, &stbuf);
	if(stbuf.i_number == 1)
	{
		prints(2, arg);
		prints(2, ": Root directory\n");
		goto retu;
	}
	if( (stbuf.i_node[0]&IFMT) != IFDIR )
	{
		prints(2, arg);
		prints(2, ": Not a directory\n");
		goto retu;
	}
	stbuf.i_node[8] = 0;
	while( read(f, stbuf.i_node, 16) == 16)
	{
		if(stbuf.i_node[0] == 0)
			continue;
		if( dot( &stbuf.i_node[1]) )
			continue;
		prints(2, arg);
		prints(2, ": Directory not empty\n");
	retu:
		close(f);
		return;
	}
	close(f);
	/*
	 * can finally unlink
	 */
	q = stbuf.i_node;
	while( *q = *name++)
		q++;
	*q++ = '/'; *q++ = '.'; *q++ = '.'; *q = 0;
	unlink(stbuf.i_node);
	*--q = 0;
	unlink(stbuf.i_node);
	--q; *--q = 0;
	unlink(stbuf.i_node);
}

dot(name)
register char *name;
{
	if( *name++ == '.' &&
	    ( *name == 0 || ( *name++ == '.' && *name == 0 )) )
		return 1;
	else
		return 0;
}
