/*
 *	df [-i] [specials]
 *
 *	prints out number of free inodes (-i option) and number of free
 *	blocks on the file systems listed (current file system by default).
 *	Certain local abreviations for the file system names are accepted.
 *	It is assumed that if a cooked device name is prefixed by
 *	the letter `r', the corresponding raw device name results.
 */ 

#include	<local-system>
#include	<filsys.h>
#include	<ino.h>

/*
 *	`prefix' is added to the given arguments - if this cannot be opened,
 *		the argument itself is tried.
 *	`rawp' is the corresponding raw device name.
 *	OFFSET is the char offset to the X's (cooked dev).
 */ 

#ifdef	EECF
char prefix[] "/dev/msXX";
char rawp[] "/dev/rmsXX";
#define	OFFSET	7
#endif

#ifdef	AGSM
char prefix[] "/dev/hpXX";
char rawp[] "/dev/rhpXX";
#define	OFFSET	7
#endif

#ifdef	EECF1140
char prefix[] "/dev/rkX";
char rawp[] "/dev/rrkX";
#define	OFFSET	7
#endif


#define	IBLK	16	/* blocks of inodes per read */

struct
{
	int ino;
	char name[14];	/* ino and name MUST be in this order */ 
	int stbuf[18];
};

struct filsys sblock;
int fi;	/* file descriptor */ 
int cookedfd;	/* file desc. of cooked dev */ 
int rawfd;	/* file desc. of raw dev */ 

struct inode *inode;
struct
{
	unsigned unsd;
};

main(argc, argv)
char **argv;
{
	register i, j;
	register char *fp;



	fp = argv[1];
	if(argc >= 2 && *fp++ == '-' && *fp++ == 'i' && *fp == 0)
	{
		if((inode.unsd = sbrk(IBLK*16*sizeof *inode)) == -1)
		{
			inode = 0;
			printf("Not enough memory to count inodes\n");
		}
		argc--;
		argv++;
	}
	if(argc <= 1)	/* use current file system */ 
	{
		stat("", sblock.stbuf);
		i = sblock.stbuf[0];
		chdir("/dev");
		j = open("", 0);
		fp = 0;
		while(read(j, &sblock, 16) > 0)
		{
			if(sblock.ino == 0)
				continue;
			stat(sblock.name, sblock.stbuf);
			if(((sblock.stbuf[2]&IFMT) == IFBLK) && (sblock.stbuf[6] == i))
			{
				fp++;
				break;
			}
		}
		if(fp == 0)
		{
			printf("Can't find file system\n");
			exit(1);
		}
		close(j);
		if((cookedfd = open(sblock.name, 0)) < 0)
		{
			printf("Cannot open /dev/%s\n", sblock.name);
			exit(1);
		}
		printf("/dev/%s ", sblock.name);
		if(inode)
		{
			sblock.name[-1] = 'r';
			if((rawfd = open(&sblock.name[-1], 0)) < 0)
				rawfd = cookedfd;
		}
		dfree();
		exit(0);
	}
	for(i = 1; i < argc; i++)
	{
		fp = argv[i];
		j = OFFSET;
		while((prefix[j++] = *fp++) && j < sizeof prefix);
		prefix[j-1] = 0;
		if((cookedfd = open(prefix, 0)) < 0)
		{
			if((cookedfd = rawfd = open(argv[i], 0)) < 0)
			{
				printf("Cannot open %s\n", argv[i]);
				continue;
			}
			fp = argv[i];
		}
		else
		{
			if(inode)
			{
				fp = argv[i];
				j = OFFSET+1;
				while((rawp[j++] = *fp++) && j < sizeof rawp);
				rawp[j-1] = 0;
				if((rawfd = open(rawp, 0)) < 0)
					rawfd = cookedfd;
			}
			fp = prefix;
		}
		printf("%s ", fp);
		dfree();
	}
}

dfree()
{
	register i, j, ino;
	int cnt, ninodes;



	sync();
	fi = cookedfd;
	bread(1, &sblock, 512);
	cnt = 0;
	ninodes = sblock.s_isize*16;
	if(inode)
	{
		fi = rawfd;
		for(i = 0, ino = 0; ino < ninodes; i =+ IBLK)
		{
			bread(i+2, inode, IBLK*16*sizeof *inode);
			for(j = 0; j < IBLK*16 && ino < ninodes; j++)
			{
				ino++;
				if((inode[j].i_mode&IALLOC) == 0)
					cnt++;
			}
		}
		printf("%d ", cnt);
		fi = cookedfd;
	}

	i = 0;
	while(alloc())
		i++;
	printf("%d\n", i);
	close(fi);
}

alloc()
{
	int b, i, buf[256];



	i = --sblock.s_nfree;
	if(i < 0 || i >= 100)
	{
		printf("bad free count ");
		return(0);
	}
	b = sblock.s_free[i];
	if(b == 0)
		return(0);
	if(b < sblock.s_isize+2 || b >= sblock.s_fsize)
	{
		printf("bad free block (%d)\n", b);
		return(0);
	}
	if(sblock.s_nfree <= 0)
	{
		bread(b, buf, 512);
		sblock.s_nfree = buf[0];
		for(i = 0; i < 100; i++)
			sblock.s_free[i] = buf[i+1];
	}
	return(b);
}

bread(bno, buf, cnt)
{
	int n;
	extern errno;



	seek(fi, bno, 3);
	if((n = read(fi, buf, cnt)) != cnt)
	{
		printf("read error %d\n", bno);
		printf("count = %d; errno = %d\n", n, errno);
		exit(1);
	}
}


#ifdef	SPRINTF
#define	NUMBERS

#include	<sprintf.h>
#endif
