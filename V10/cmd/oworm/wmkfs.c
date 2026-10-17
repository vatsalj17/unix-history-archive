#include	<libc.h>
#include	"worm.h"
#include	<sys/types.h>
#include	<sys/udaioc.h>

usage()
{
	fprint(2, "Usage: worm mkfs [-fdevice] [-ccomments] [-bblksize] [-nnblks] [-vnewvol_id] [-ublkno] vol_id\n");
	fprint(2, "e.g. worm mkfs -f1 -c\"512x512x24 movies\" tdmovies1a\n");
	exit(1);
}

main(argc, argv)
	char **argv;
{
	Superblock s, os;
	char *b;
	long sb;
	int c;
	int bflag = 0;
	int cflag = 0;
	int nflag = 0;
	long uflag = -1;
	char *volid;
	char *dev = "/dev/worm0";
	char *nblks = "1600000";
	char *bsize = "1024";
	char *nvolid = 0;
	char *comments = "";
	extern optind;
	extern char *optarg;

	while((c = getopt(argc, argv, "u:f:n:b:c:v:")) != -1)
		switch(c)
		{
		case 'b':	bflag = 1; bsize = optarg; break;
		case 'c':	cflag = 1; comments = optarg; break;
		case 'f':	dev = optarg; break;
		case 'n':	nflag = 1; nblks = optarg; break;
		case 'u':	uflag = atol(optarg); break;
		case 'v':	nvolid = optarg; break;
		case '?':	usage();
		}
	if(nvolid && (uflag < 0)){
		fprint(2, "worm mkfs: -v only allowed with -u\n");
		exit(1);
	}
	if(optind != argc-1)
		usage();
	volid = argv[optind];
	if(strlen(volid) > sizeof(s.vol_id)-1)
		volid[sizeof(s.vol_id)-1] = 0;
	c = volid[strlen(volid)-1];
	if((c != 'a') && (c != 'b')){
		if(nvolid == 0){
			fprint(2, "worm mkfs: vol_id '%s' must end in 'a' or 'b'\n", volid);
			exit(1);
		}
		fprint(2, "worm mkfs: warning: vol_id '%s' should end in 'a' or 'b'\n", volid);
	}
	if(nvolid){
		if(strlen(nvolid) > sizeof(s.vol_id)-1)
			nvolid[sizeof(s.vol_id)-1] = 0;
		c = nvolid[strlen(nvolid)-1];
		if((c != 'a') && (c != 'b')){
			fprint(2, "worm mkfs: vol_id '%s' must end in 'a' or 'b'\n", nvolid);
			exit(1);
		}
	}
	dev = mapdev(dev);
	if((s.fd = open(dev, 2)) < 0){
		perror(dev);
		exit(1);
	}
	strcpy(s.vol_id, volid);
	if(strlen(comments) > sizeof(s.comment)-1)
		comments[sizeof(s.comment)-1] = 0;
	strcpy(s.comment, comments);
	s.magic = SMAGIC;
	s.blocksize = atoi(bsize);
	if((s.blocksize < 512) || (s.blocksize%512)){
		fprint(2, "I don't believe '%s' is a blocksize\n", bsize);
		exit(1);
	}
	s.version = VLINK;
	s.nblocks = atol(nblks);
	if(s.blocksize < 512){
		fprint(2, "I don't believe a nblocks = '%s'\n", nblks);
		exit(1);
	}
	if(s.blocksize % sizeof(Inode)){
		fprint(2, "sizeof(Inode)=%d does not divide blocksize %d\n",
			sizeof(Inode), s.blocksize);
		exit(1);
	}
	sb = 1 + (s.zero = atol("0"));
	if(s.zero < 0){
		fprint(2, "I don't believe a dirblk = '%s'\n", *argv);
		exit(1);
	}
	s.nextsb = sb+1;
	s.nextffree = sb+2;
	s.nfree = s.nblocks - s.nextffree;
	if(s.nfree <= 1){
		fprint(2, "nblocks too small\n");
		exit(1);
	}
	s.ninodes = 0;
	s.ninochars = 0;
	s.binodes = 0;
	time(&s.ctime);
	if((b = malloc(s.blocksize)) == 0){
		fprint(2, "cannot malloc buffer %d bytes\n", s.blocksize);
		exit(1);
	}
	if(uflag >= 0){
		Seek(&s, uflag);
		if(Read(&s, b, 1L))
			exit(1);
		memcpy((char *)&os, b, sizeof os);
		if(os.magic != SMAGIC){
			fprint(2, "mowrm mkfs: block %ld not a superblock!\n", uflag);
			exit(1);
		}
		if(strcmp(os.vol_id, s.vol_id)){
			fprint(2, "worm mkfs: expected volid '%s', got '%s'\n",
				s.vol_id, os.vol_id);
			exit(1);
		}
		if(nvolid)
			strcpy(os.vol_id, nvolid);
		if(bflag)
			os.blocksize = s.blocksize;
		if(cflag)
			strcpy(os.comment, s.comment);
		if(nflag){
			os.nblocks = s.nblocks;
			os.nfree = os.nblocks - os.nextffree;
			if(os.nfree <= 1){
				fprint(2, "worm mkfs: new nblocks too small\n");
				exit(1);
			}
		}
		s = os;
		sb = uflag;
	}
	memset(b, 0, s.blocksize);
	memcpy(b, (char *)&s, sizeof(s));
	Seek(&s, sb);
	if(Write(&s, b, 1L))
		exit(1);
	ioctl(s.fd, UIOSPDW);
	exit(0);
}
