#include	<mtab.h>
#include	<errnos.h>
#include	<stat.h>

char *devd	"/dev/xxxxxxxxxxxxxx";
int errortotal;

main(argc, argv)
char **argv;
{
	register struct mtab *mp;
	register char *p1, *p2;
	int n, m, mf;
	extern int errno;
	extern char *sys_errlist[];
	struct statb sb[1];

	if(argc < 2)
	{
		prints(2, "Usage: umount devname|rootdir ...\n");
		return 1;
	}
	sys_errlist[EINVAL] = "Device not currently mounted";
	if((mf = open(mtabf, 0)) < 0)
	{
		perror(mtabf);
		return 1;
	}
	n = read(mf, mtab, sizeof mtab);
	if(n%sizeof mtab[0])
	{
		error(mtabf, "Wrong format?\n");
		return 1;
	}
	close(mf);
	sync();
	m = 0;
	for(p2 = *++argv; --argc; p2 = *++argv)
	{
		if(stat(p2, sb) < 0)
		{
			perror(p2);
			errortotal++;
			continue;
		}
		switch(sb->i_mode & IFMT)
		{
	    case IFDIR:
			if(sb->i_ino != 1)
			{
				error(p2, ": Not a filesystem rootdir\n");
				errortotal++;
				continue;
			}
			if( (p1 = dsearch(sb->i_dev)) == 0)
			{
				error(p2, ": Device not found in /dev\n");
				errortotal++;
				continue;
			}
			devd[5] = 0;
			p2 = strcat(devd, p1);
			break;
	    case IFBLK:
			p1 = p2;
			while(*p1++);
			p1--;
			while(*--p1 == '/') *p1 = 0;
			while(p1 > p2 && *--p1 != '/');
			p1++;
			break;
	    default:
			error(p2, ": Not a block device or filesystem root\n");
			errortotal++;
			continue;
		}
		if(umount(p2) < 0)
		{
			perror(p2);
			errortotal = errortotal || (errno != EINVAL);
			continue;
		}
		n = 0;
		for(mp = mtab; mp < &mtab[NMOUNT]; mp++)
		{
			if(strcmp(p1, mp->m_spec) == 0)
			{
				mp->m_file[0] = 0;
				n++,m++;
			}
		}
		if(n == 0)
		{
			error(p2, ": Not in mount table (non-fatal)\n");
		}
	}
	if(m != 0)
	{
		if( (mf = creat(mtabf, 0604)) < 0)
		{
			perror(mtabf);
			return errortotal+1;
		}
		for(mp = mtab; mp < &mtab[NMOUNT]; mp++)
		{
			if(*mp->m_file)
				write(mf, mp, sizeof mtab[0]);
		}
	}
	return errortotal;
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

error(s1, s2)
{
	prints(2, s1);
	prints(2, s2);
}
