#include	<libc.h>
#include	"worm.h"
#include	"sym.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<pwd.h>
#include	<grp.h>

Inode *inodebase, *inext;
char *namebase, *cnext;

main(argc, argv)
	char **argv;
{
	Superblock s;
	char *e;
	char *dev = "/dev/worm0";
	register c, j;
	register Inode *from, *to;
	extern char *optarg;
	extern int optind;
	char *vlk();
	int cmp();
	int fd;
	char buf[512];
	long ni, nc;

	while((c = getopt(argc, argv, "f:")) != -1)
		switch(c)
		{
		case 'f':	dev = optarg; break;
		case '?':	usage();
		}
	dev = mapdev(dev);
	if((s.fd = open(dev, 0)) < 0){
		perror(dev);
		exit(1);
	}
	if(e = openinode(&s, SPIN_DOWN)){
		fprint(2, "%s: %s\n", dev, e);
		exit(1);
	}
	if(s.version != VLINK){
		fprint(2, "%s: not a vlink disk, no action taken.\n", s.vol_id);
		exit(1);
	}
	if(optind != argc-1)
		usage();
	if(strcmp(argv[optind], s.vol_id)){
		fprint(2, "wanted volid '%s'; got '%s'\n", argv[optind], s.vol_id);
		exit(1);
	}
	if((inodebase = (Inode *)malloc(s.blocksize+(int)numinodes*sizeof(Inode))) == 0){
		fprint(2, "malloc of %ld inodes failed\n", numinodes);
		exit(2);
	}
	if((namebase = malloc(s.blocksize+(int)numnamechars+(int)numinodes)) == 0){
		fprint(2, "malloc of %ld chars failed\n", numnamechars);
		exit(2);
	}
	inext = inodebase;
	cnext = namebase;
	if(e = vlk(&s, 1L)){
		fprint(2, "%s: %s\n", dev, e);
		exit(1);
	}
	j = inext-inodebase;
	qsort((char *)inodebase, j, sizeof(*inodebase), cmp);
	for(to = from = inodebase+1; from < inext; from++)
		if(strcmp(from->name.o+namebase, from[-1].name.o+namebase))
			*to++ = *from;
	inext = to;
	j = inext-inodebase;
	sprint(buf, "/usr/worm/tmp/%s", s.vol_id);
	if((fd = creat(buf, 0666)) < 0){
		perror(buf);
		exit(1);
	}
	ni = j;
	nc = cnext-namebase;
	write(fd, (char *)&s.ctime, 4);
	write(fd, (char *)&ni, 4);
	write(fd, (char *)inodebase, (int)ni*sizeof(Inode));
	write(fd, (char *)&nc, 4);
	write(fd, namebase, (int)nc);
	exit(0);
}

usage()
{
	fprint(2, "Usage: worm tmpdir [-fdevice] vol_id\n");
	exit(2);
}

cmp(a, b)
	Inode *a, *b;
{
	return(strcmp(namebase+a->name.o, namebase+b->name.o));
}

char *
vlk(s, blk)
	register Superblock *s;
	long blk;
{
	register Inode *i;
	short fd = s->fd;
	register long j;
	char *b;
	long nb;
	Inode *iend;
	static char buf[64];

	if((b = malloc(s->blocksize)) == 0){
		sprint(buf, "couldn't malloc buffer (%d bytes)", s->blocksize);
		return(buf);
	}
	/* cheat */
	s->nextsb = blk;
	goto skip;
	for(;;){
		if(s->magic != SMAGIC){
			fprint(2, "bad Superblock at %ld\n", blk);
			exit(1);
		}
		if(s->ninodes){
			nb = (s->ninodes+IPERB-1)/IPERB;
			Seek(s, s->binodes);
			if(Read(s, (char *)inext, nb))
				goto skip;
			j = cnext-namebase;
			for(i = inext, iend = i+s->ninodes; i < iend; i++)
				i->name.o += j;
			inext += s->ninodes;
			nb = (s->ninochars+s->blocksize-1)/s->blocksize;
			if(Read(s, cnext, nb))
				goto skip;
			cnext += (s->ninochars+1)&~1;
		}
	skip:
		blk = s->nextsb;
		Seek(s, blk);
		if(Read(s, b, 1L))
			break;
		*s = *((Superblock *)b);
		s->fd = fd;
		if(s->myblock == 0)
			s->myblock = blk;
	}
	free(b);
	return((char *)0);
}
