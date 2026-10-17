#include	<libc.h>
#include	"worm.h"
#include	"sym.h"
#include	<fio.h>
#include	"../backup/backup.h"

#define		N		64
long blks[N];

int nb = 0;
long *allblks;
long nallblk;

cmp(a, b)
	int *a, *b;
{
	return(*a - *b);
}

main(argc, argv)
	char **argv;
{
	char *dev = "/dev/worm0", *e, *vol_id;
	int c, i, j;
	Superblock sb;
	void blkfn();
	extern char *optarg;
	extern int optind;
	extern long atol();

	while((c = getopt(argc, argv, "f:")) != -1)
		switch(c)
		{
		case 'f':	dev = optarg; break;
		case '?':	usage();
		}
	if(optind != argc-1)
		usage();
	vol_id = argv[optind];
	dev = mapdev(dev);
	if((sb.fd = open(dev, 0)) < 0){
		perror(dev);
		exit(1);
	}
system("echo reading inodes `date` 1>&2");
	if(e = openinode(&sb, DO_INODE|SPIN_DOWN)){
		fprint(2, "%s: %s\n", dev, e);
		exit(1);
	}
	if(strcmp(vol_id, sb.vol_id)){
		fprint(2, "wanted volid '%s'; got '%s'\n", vol_id, sb.vol_id);
		exit(1);
	}
	if((allblks = (long *)malloc(sizeof(long)*(int)numinodes)) == 0){
		fprint(2, "out of memory (%d)\n", numinodes);
		exit(1);
	}
	nallblk = 0;
	inodetraverse(blkfn);
system("echo sorting block numbers `date` 1>&2");
	qsort((char *)allblks, (int)nallblk, sizeof(long), cmp);
system("echo reading blocks `date` 1>&2");
	for(j = 0; j < nallblk;){
		for(i = 0; (i < N) && (j < nallblk); i++){
			blks[i] = allblks[j++];
			if(blks[i] >= blks[0]+N){
				block(sb.fd, i);
				blks[0] = blks[i];
				i = 0;
			}
		}
		block(sb.fd, i);
	}
	fprint(2, "%ld blocks\n", nb);
system("echo done `date` 1>&2");
	exit(0);
}

usage()
{
	fprint(2, "Usage: wormupd [-fdevice] vol_id\n");
	exit(2);
}

block(fd, n)
{
	register i;
	register struct backfile *h;
	struct backfile buf[N];

	nb++;
	lseek(fd, blks[0]*1024, 0);
	read(fd, (char *)buf, 1024*(int)(blks[n-1]-blks[0]+1));
	for(i = 0; i < n; i++){
		h = &buf[blks[i]-blks[0]];
		Fprint(1, "%s%c%s%c%ld%c%d%c%ld%c%s%c%s\n", h->oname, 0, h->bname,
			0, h->sbuf.st_ctime, 0, h->sbuf.st_mode, 0, h->sbuf.st_size,
			0, h->uname, 0, h->gname);
	}
}

void
blkfn(i)
	Inode *i;
{
	if(strchr(i->name.n, '/'))
		allblks[nallblk++] = i->block;
}
