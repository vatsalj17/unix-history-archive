#include <stat.h>

struct inode
{
	int mode;
	int dummy[13];
	long mtime;
} inodes[512];		/* 512 inodes in a slurp */

#define	SKE	"seek"
#define	RDE	"read"
int supblk[256];
int ibuf[259];

int	ncnum;		/* inode number from ncheck output */
char	ncnam[200];	/* file name from ncheck output */

main(argc,argv)
int argc;
char **argv;
{
	register int *ip, *xp, i ,n;
	int ipc;		/* count of inodes updated */
	unsigned int inum;	/* counter for inodes looked at */
	unsigned int ino;	/* number of inodes to look at */
	int fflag;		/* flag for output loop */
	int fd;			/* filesystem access */
	int status;		/* status of child */
	struct statb sbuf;	/* for stat of marker-file */
	int upino[2000];	/* hold updated inode numbers */
	char *tempf = "/tmp/oscaraXXXXX";

	if( argc != 3 )
	{
	usage:
		printf("use:	oscar marker-file raw-file-sys\n");
		return 1;
	}
	sys( stat(argv[1],&sbuf), argv[1] );

	mktemp(tempf);

	switch( fork() )
	{
	case 0:		/* child */
		close(1);
		sys( creat(tempf,0600), tempf );	/* ncheck std output */
		execl( "/etc/bin/ncheck", "ncheck", "-a", argv[2], 0 );
		execl( "/etc/ncheck", "ncheck", "-a", argv[2], 0 );
		perror("ncheck");
		exit(1);
	case -1:		/* error */
		perror("fork");
		exit(1);
	default:		/* parent */
		break;
	}
	sys( fd = open(argv[2],0) , argv[2] );

	sys( seek(fd,1,3) , SKE );
	sys( n = read(fd,supblk,512) , RDE );	/* super block */
	ino = supblk[0] * ( 512 / 32 );		/* total inodes */
	inum = 0;
	ip = upino;
	ipc = 0;
	while( (n = read(fd,inodes,sizeof inodes)) > 0 )
	{
		n =/ 32;	/* number of inodes this slurp */
		for( i = 0 ; i < n ; i++ )
		{
			if( inum++ == ino )
				goto done;
			if( inodes[i].mode&IALLOC == 0 )
				continue;	/* not allocated */
			if( inodes[i].mode&IFMT )
				continue;	/* want ordinary */
			if( sbuf.i_mtime >= inodes[i].mtime )
				continue;	/* not updated */
			*ip++ = inum;		/* remember them */
			ipc++;			/* and count them */
		}
	}
  done:
	waitx( &status );	/* ncheck done ?? */
	if( status )
	{
		printf("ncheck failed\n");
		goto unlnk;
	}
	sys( fopen(tempf,ibuf) , tempf );
	fflag = 0;	/* number found */
	if( ipc )
	{
		while( getline() )
		{
			xp = upino;
			while( xp != ip )
			{
				if( *xp++ == ncnum )
				{
					fflag++;
					printf("%s\n",ncnam);
					xp[-1] = -1; /* not again */
					if( --ipc == 0 ) goto finit;
					break;
				}
			}
		}
	}
finit:
	if( fflag == 0 )
		prints(2,"No files updated\n");
unlnk:
	unlink(tempf);
}

sys( sysret , argindic )
{
	if( sysret != -1 )
		return;
	perror(argindic);
	exit(1);
}

getline()
{
	register char c;
	register char *p;

	while( (c=getc(ibuf)) != -1 )
	{
		ncnum = c - '0';
		while( (c=getc(ibuf)) != '\t' )
			ncnum = ncnum * 10 + c - '0';
		p = ncnam;
		getc(ibuf);	/* no leading slash */
		while( (c=getc(ibuf)) != '\n' )
			*p++ = c;
		*p = 0;
		return 1;
	}
	return 0;
}
