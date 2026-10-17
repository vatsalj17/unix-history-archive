#
/*
 *	mtabin
 *
 *	Initialize "mtab" file with an entry for '/'
 *	with a device name found in '/dev' and a mounted
 *	time of "now".
 */

#include	<mtab.h>
#include	<stat.h>

char	devd[]	"/dev/xxxxxxxxxxxxxx";

main()
{
	register struct mtab *mp;
	struct	statb	sb;
	extern long time();
	register int mf;

	mp = mtab;
	mp->m_file[0] = '/';
	stat(mp->m_file, &sb);
	strcpy(mp->m_spec, dsearch(sb.i_dev));
	mp->m_time = time();
	mp->m_prot = 0;
	if((mf = creat(mtabf, 0604)) < 0)
	{
		perror(mtabf);
		return 1;
	}
	write(mf, mp, sizeof mtab[0]);
	return 0;
}

dsearch(device)
register int device;
{
	struct statb sb[1];
	static char buf[18];
	register int df, *bp;

	bp = buf;
	if( (df = open("/dev", 0)) < 0)
	{
		perror("/dev");
		return 0;
	}
	while(read(df, bp, 16) == 16)
	{
		if(*bp && strcmp(bp+1, "swap") != 0)
		{
			devd[5] = 0;
			stat(strcat(devd, bp+1), sb);
			if((sb->i_mode&IFMT) == IFBLK && sb->i_addr[0] == device)
			{
				close(df);
				return bp+1;
			}
		}
	}
	close(df);
	return 0;
}
