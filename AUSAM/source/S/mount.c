#include	<mtab.h>

/*
 *	Mount
 *
 *	Returns exit codes as follows...
 *
 *	0 - Success
 *	1 - Failure, no mount performed.
 *	2 - Failure after mount has succeeded.
 *
 *	In the last case the file system is mounted, but "mtab" is
 *	incorrect.
 */

main(argc, argv)
register char **argv;
{
	register struct mtab *mp;
	register char *np;
	int n, mf, ro;
	extern long time();


	if((mf = open(mtabf, 0)) < 0)
	{
		perror(mtabf);
		return 1;
	}

	n = read(mf, mtab, sizeof mtab);
	if(n%sizeof mtab[0])
	{
		prints(2, mtabf);
		prints(2, ": Wrong format\n");
		return 1;
	}
	if(argc == 1)
	{
		for(mp = mtab; mp < &mtab[NMOUNT]; mp++)
		{
			if(mp->m_file[0])
			{
				prints(1, mp->m_spec);
				prints(1, " on ");
				prints(1, mp->m_file);
				np = ctime(mp->m_time);
				np[3] = '\t';
				np[16] = 0;
				prints(1, np + 3);
				if(mp->m_prot)
					prints(1, " (Read only)\n");
				else
					prints(1, "\n");
			}
		}
		return 0;
	}
	if(argc < 3)
	{
		prints(2, "Usage: mount [ special pathname ]\n");
		return 1;
	}
	ro = (argc > 3);
	if(mount(argv[1], argv[2], ro) < 0)
	{
		perror("mount");
		return 1;
	}
	np = argv[1];
	while(*np++);
	np--;
	while(*--np == '/')
		*np = '\0';
	while(np > argv[1] && *--np != '/');
	if(*np == '/')
		np++;
	argv[1] = np;
	for(mp = mtab; mp < &mtab[NMOUNT]; mp++)
	{
		if(mp->m_file[0] == 0)
		{
			for(np = mp->m_spec; np < &mp->m_spec[15]; )
				if((*np++ = *argv[1]++) == 0)
					argv[1]--;
			for(np = mp->m_file; np < &mp->m_file[NAMSIZ-1]; )
				if((*np++ = *argv[2]++) == 0)
					argv[2]--;
			mp->m_time = time();
			mp->m_prot = ro;
			mp = &mtab[NMOUNT];
			while((--mp)->m_file[0] == 0);
			close(mf);
			if((mf = creat(mtabf, 0604)) < 0)
			{
				perror(mtabf);
				return 2;
			}
			n = (mp-mtab+1)*(sizeof mtab[0]);
			if(write(mf, mtab, n) != n)
			{
				perror("write");
				return 2;
			}
			return 0;
		}
	}
	prints(2, "Mount table full\n");
	return 2;
}
