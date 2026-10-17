#include	<libc.h>
#include	<fio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	"worm.h"

static int bad = 0;
static long nbytes;
static long nfiles;
char *argout;

main(argc, argv)
	char **argv;
{
	Superblock s;
	char *e;
	char buf[4096];
	int n;
	int c;
	char *dev = "/dev/worm0";
	extern char *optarg;
	extern int optind;

	argout = argv[0];
	while((c = getopt(argc, argv, "f:")) != -1)
		switch(c)
		{
		case 'f':	dev = optarg; break;
		case '?':	usage();
		}

	if(optind >= argc)
		usage();
	dev = mapdev(dev);
	if((s.fd = open(dev, 2)) < 0){
		perror(*argv);
		exit(1);
	}
	if(e = openinode(&s, SPIN_DOWN)){
		fprint(2, "%s: %s\n", *argv, e);
		exit(1);
	}
	if(strcmp(s.vol_id, argv[optind])){
		fprint(2, "vol_id mismatch: wanted %s, got %s\n", argv[optind], s.vol_id);
		exit(1);
	}
	if(s.nfree == 0){
		fprint(2, "%s: can't write any more!\n", dev);
		exit(1);
	}
	if(s.version != VLINK){
		fprint(2, "%s: can't write on a b-tree disk\n", s.vol_id);
		exit(1);
	}
	for(n = 1; n <= NSIG; n++)
		signal(n, SIG_IGN);
	ininit();
	if(++optind < argc)
		while(optind < argc)
			proc(&s, argv[optind++]);
	else
		while(e = Frdline(0))
			proc(&s, e);
	if(bad)
		exit(1);
	nfiles = nbytes = 0;
	inwrite(&s);
	if(bad)
		exit(1);
	fprint(1, "%d files, %.6fMb\n", nfiles, nbytes/1e6);
	exit(0);
}

usage()
{
	fprint(2, "Usage: worm write [-fdevice] vol_id [files]\n");
	exit(1);
}

proc(s, file)
	Superblock *s;
	char *file;
{
	struct stat sbuf;
	unsigned short mode;
	Inode i;

	memset((char *)&i, 0, sizeof(i));
	if(stat(file, &sbuf) < 0){
		perror(file);
		return;
	}
	mode = sbuf.st_mode&S_IFMT;
	if((mode == S_IFREG) || (mode == S_IFDIR)){
		i.magic = DMAGIC;
		i.block = 0;
		i.nbytes = sbuf.st_size;
		nbytes += i.nbytes;
		i.ctime = sbuf.st_ctime;
		i.name.n = file;
		i.mode = sbuf.st_mode;
		i.uid = sbuf.st_uid;
		i.gid = sbuf.st_gid;
		if(inadd(s, &i))
			bad = 1;
	} else
		fprint(2, "%s is not a file\n", file);
}

static Inode *inodes;
static long ip;
static long ninodes = 0;
static char *nameb;
static long np;
static long nnameb = 0;
static long nblocks, blkdone;
#define		IINC		1024
#define		NINC		(64*IINC)

ininit()
{
	if(nnameb == 0){
		nameb = malloc((unsigned)(nnameb = NINC));
		if(nameb == 0){
			fprint(2, "wwrite: malloc fail, %d bytes\n", nnameb);
			exit(1);
		}
	}
	np = 0;
	if(ninodes == 0){
		inodes = (Inode *)malloc(sizeof(Inode)*(unsigned)(ninodes = IINC));
		if(inodes == 0){
			fprint(2, "wwrite: malloc fail, %d inodes %d bytes\n", ninodes, ninodes*sizeof(Inode));
			exit(1);
		}
	}
	ip = 0;
	nblocks = 0;
}

inadd(s, i)
	Superblock *s;
	register Inode *i;
{
	register long len;

	len = strlen(i->name.n)+1;
	if(np+len > nnameb){
		while(np+len > nnameb)
			nnameb += NINC;
		nameb = realloc(nameb, (unsigned)nnameb);
		if(nameb == 0){
			fprint(2, "wwrite: realloc fail, %d bytes\n", nnameb);
			exit(1);
		}
	}
	strcpy(nameb+np, i->name.n);
	i->name.o = np;
	np += len;
	i->block = s->nextffree + nblocks;
	if(ip == ninodes){
		ninodes += IINC;
		inodes = (Inode *)realloc((char *)inodes, (unsigned)ninodes*sizeof(Inode));
		if(inodes == 0){
			fprint(2, "wwrite: realloc fail, %d inodes %d bytes\n", ninodes, ninodes*sizeof(Inode));
			exit(1);
		}
	}
	inodes[ip++] = *i;
	nblocks += (i->nbytes+s->blocksize-1)/s->blocksize;
	return(0);
}

inwrite(s)
	Superblock *s;
{
	int i, j;
	long next = s->nextffree;
	char *e;

	if(e = lkwri(s, inodes, ip, nameb, np, nblocks)){
		fprint(2, "%s\n", e);
		bad = 1;
		return;
	}
	Seek(s, next);
	argout[2] = ' ';
	for(i = 0; i < ip; i++){
		inodes[i].block = next;
		writeout(s, &inodes[i], &next);
		j = (blkdone*100)/nblocks;
		argout[0] = j/10+'0';
		argout[1] = j%10+'0';
	}
}

writeout(s, i, blk)
	Superblock *s;
	Inode *i;
	long *blk;
{
	char b[63*1024L];
	int fd;
	long n, len, blen;
	char *name;

	n = (i->nbytes+s->blocksize-1)/s->blocksize;
	*blk += n;
	blkdone += n;
	blen = sizeof b/s->blocksize;
	len = blen*s->blocksize;
	nbytes += i->nbytes;
	nfiles++;
	name = i->name.o + nameb;
	if((fd = open(name, 0)) < 0)
		goto out;
	for(n = i->nbytes; n > len; n -= len){
		if(read(fd, (char *)b, (int)len) != len){
	out:
			perror(name);
			bad = 1;
			return;
		}
		if(Write(s, b, blen)){
fprint(2, "nb=%d, n=%d len=%d blen=%d\n", i->nbytes, n, len, blen);
			perror("data write");
			exit(1);
		}
	}
	if(n){
		memset(b, 0, sizeof b);
		if(read(fd, (char *)b, (int)n) != n)
			goto out;
		n += s->blocksize-1;
		n /= s->blocksize;
		if(Write(s, b, n)){
			perror("data write");
			exit(1);
		}
	}
	close(fd);
}
