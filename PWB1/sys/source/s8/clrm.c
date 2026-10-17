/*  	Clear I-mode
 *
 * 	Modified: 5/76
 */

#define	NINODES 16	/* Inodes/Block */
#define NULL	0	/* Zero Entry */

struct	inode
{
	int	i_mode;
	char	i_nlink;
	char	i_uid;
	char	i_gid;
	char	i_size0;
	char	*i_size1;
	int	i_addr[8];
	int	i_atime[2];
	int	i_mtime[2];
}iptr[NINODES];

/* modes */
#define	IALLOC	0100000
#define	IFMT	060000
#define		IFDIR	040000
#define		IFCHR	020000
#define		IFBLK	060000
#define	ILARG	010000
#define	ISUID	04000
#define	ISGID	02000
#define ISVTX	01000
#define	IREAD	0400
#define	IWRITE	0200
#define	IEXEC	0100


	int	dev, bno, offset, ino;
main(argc,argv)
char	**argv;
{


	if(argc < 3) 
		error("USAGE: clrm device name [I-number ...]\n");

	if((dev = open(*(++argv),2)) < 0)
		error("Can't open %s\n",argv);

	argc--;
	while(--argc > 0) {
		argv++;
		if((ino = num(*argv)) <= 0) continue;
		bno = (((ino -1)>>4)&07777)+2;
		offset = ((ino - 1)&017);
		if(bread(&iptr)) {
						/* Directory Check  */

			if(iptr[offset].i_mode &IFDIR) {
				printf("I-number %l Directory -- ",ino);
						printf("Can't clear\n");
				continue;
			}
						/* Special Device Check */

			if(iptr[offset].i_mode &IFBLK ||
						iptr[offset].i_mode &IFCHR) {
				printf("I-number %l Special Device -- ",ino);
						printf("Can't clear\n");
				continue;
			}
		iptr[offset].i_mode = NULL;
		}else {
			printf("READ ERROR - inode: %l not processed\n",ino);
			continue;
		}
		if(!bwrite(&iptr)) {
			printf("WRITE ERROR - inode: %l not processed\n",ino);
		}
	}
}
num(s)
char	*s;
{
	register char	*sp;
	register i;

	sp = s;
	if(*sp == '-')return(-1);
	while(*sp) {
		if(*sp < '0' || *sp > '9') {
			printf("I-number is a non-numeric argument\n");
			return(-1);
		}
		i = i * 10 +(*sp - '0');
		++sp;
	}
	return(i);
}
bread(buf)
struct inode *buf;

{
	int n;
	extern errno;
	
	seek(dev,0,0);	/* seek to beginning of file */
	seek(dev,bno,3);
	if((n = read(dev,buf,512)) != 512) {
		printf("read error %o\n",bno);
		printf("count = %d;errno = %d\n",n,errno);
		return(0);
	}
	return(1);
}

bwrite(buf)
struct inode *buf;
{
	int n;
	extern errno;

	seek(dev,0,0);	/* seek to beginning of file */
	seek(dev,bno,3);
	if((n = write(dev,buf,512)) != 512) {
		printf("write error %l\n",bno);
		printf("count = %d;errno = %d\n",n,errno);
		return(0);
	}
	return(1);
}

error(s1,s2)
char	*s1, *s2;
{
	printf(s1,s2);
	exit(1);
}
