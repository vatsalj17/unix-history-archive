#
#include	<local-system>

/*
 *	Unix disc usage program:
 *		du -s [dir1 .....]	sums in each directory
 *		du -a [dir1 .....]	prints usage for all entries
 *		du -u [name1 .....]	prints usage for users name1 ,...
#indef	AUSAM
 *		du -g [group1 .....]	for each group
#endif
 *		du -s -[ug]		requests total for group/user
 *			K Birman
 */

/*
 *	AUSAM --	remove options ( needs to be restored to former glory )
 */

long	du();
long	pdu();
int	oflush();
#ifndef	AUSAM
struct	{
	char who[10];
	long	owned;
} *nlist;
#endif
int	*table;
long	fsize;
#ifndef	AUSAM
int	nuse;
#endif
unsigned uid;
int	fout;
int	fin;
int	maxword -1;
char	*name;
int	aflg;
int	sflg;
int	uflg;
#ifndef	AUSAM
int	gflg;
#endif
int	restrict;
int	count;
char	**vec;
int	mfile;

struct	mtab {
	char where[32];
	char m_name[32];
} mtab;

#ifndef	AUSAM
int	ulist[256];
#endif

/* list of resident discs: /etc/mtab doesn't mention these */
char	*ress[] {
	"/dev/hp0",
	0
};
char	**res	ress;

main(argc, argv)
char **argv;
{
	char place[10];

	table = sbrk(0);
	if(argc > 1) {
		++argv;
		while(**argv == '-') {
			argc--;
			switch((*argv++)[1]) {
			case 'a':
				aflg++;
				break;
			case 's':
				sflg++;
				break;
#ifndef	AUSAM
			case 'u':
				uflg++;
				break;
			case 'g':
				gflg++;
				break;
#endif
			default:
				type("Unknown option");
				exit(1);
			}
		}
		if(argc > 1) {
			restrict++;
			count = argc;
			vec = argv;
		}
	}
	fout = dup(1);
	signal(2, oflush);
	signal(3, oflush);
#ifndef	AUSAM
	if((uflg == 0 && gflg == 0) || aflg)
#endif
		if(restrict) while(argc-- > 1)
			pdu(*argv++);
		else
			pdu(".");
#ifndef	AUSAM
	else {
		build();
		while(*res)
			dui(*res++);
		mfile = open("/etc/mtab", 0);
		if(mfile > 0) while(read(mfile, &mtab, 64) == 64) {
			append("/dev", mtab.m_name, place, 5, 10);
			dui(place);
		}
	}
	if(sflg && (uflg||gflg))
		ptot();
#endif
	flush();
}

long pdu(s)
{
	long n;

	n = du(s);
	typen(n);
	type(s);
	return(n);
}

typen(n)
long n;
{
	register char *s;
	register int i;
	char *locv();

	s = locv(n);
	i = 0;
	while(*s) {
		putchar(*s++);
		i++;
	}

	while(i++ < 12)
		putchar(' ');
}

type(s)
register char *s;
{
	if(*s == '/' && *(s+1) == '/') s++;
	while(*s)
		putchar(*s++);
	putchar('\n');
}
#ifndef	AUSAM
build()
{
	register int *i;
	for(i = ulist; i<&ulist[256]; *i++ = -1);
	fin = open(gflg ? "/etc/group":"/etc/passwd", 0);
	nlist = sbrk(sizeof *nlist);
	table =+ sizeof *nlist;
	while(getent()) if(useent())
		ulist[uid] = nuse++;
	close(fin);
}
#define	NAME	0
#define	USER	2
#endif
#define	DIR	1
#ifndef	AUSAM
getent()
{
	register int state;
	register char  *s;
	int c;
	state = 0;
	s = name = nlist[nuse].who;
	uid = 0;

	while(c = getchar())
	switch(c) {
	case ':':
		if(!state++) *s++ = 0;
		continue;
	case '\n':
		sbrk(sizeof *nlist);
		table =+ sizeof *nlist;
		nlist[nuse].owned = 0;
		return(1);
	default:
		switch(state) {
		case NAME:
			if(s == &name[9])
				continue;
			*s++ = c;
			break;
		case USER:
			uid = uid*10 + c - '0';
			break;
		}
		continue;
	}
	return(0);
}

useent()
{
	int argc;
	register char **argv, *s, *ss;

	if(restrict == 0) return(1);

	argc = count;
	argv = vec;
	while(argc-- > 1){
		s = *argv++;
		ss = name;
		while(*s == *ss && *s++) ss++;
		if(*ss) continue;
		return(1);
	}
	return(0);
}
#endif

mark(bit)
long bit;
{
	register int word, mask;
	word = (bit>>4);
	if(word > maxword) {
		sbrk((word - maxword)<<1);
		maxword = word;
	}

	mask = 1<<(bit & 017);
	if(table[word] & mask)
		return(1);
	table[word] =| mask;
	return(0);
}
long du(dir)
{
	register int fd, loc;
	long size, total;
	char string[250];
	struct dent {
		int ino;
		char d_name[14];
	} dent;

	if(ftype(dir) != DIR || (fd = open(dir, 0)) < 0)
		return(fsize);
	loc = 0;
	total = fsize;

	while(read(fd, &dent, 16) == 16) {
		loc =+ 16;
		if(dent.ino == 0)
			continue;
		if(dent.d_name[0] == '.' && ((dent.d_name[1] == '.' && dent.d_name[2] == 0) || dent.d_name[1] == 0))
			continue;
		if(fd == 14)
			close(fd);
		append(dir, dent.d_name, string, 15, 50);
		if(ftype(string) == DIR)
			if(sflg == 0) {
				size = pdu(string);
			} else {
				size = du(string);
				if(aflg) {
					typen(size);
					type(string);
				}
			}
		else {
			size = fsize;
			if(aflg) {
				typen(size);
				type(string);
			}
		}
		if(mark(0, dent.ino) == 0)
			total =+ size;
		if(fd == 14) {
			fd = open(dir, 0);
			seek(fd, loc, 0);
		}
	}
	close(fd);
	return(total);
}
#ifndef	AUSAM
dui(disk)
{
	long len;
	int *aadr;
	struct {
		int flags;
		char nlinks;
		char iuid;
		char igid;
		char sizeo;
		int sizel;
		int crap[12];
	} inode;
	register int t, dfile, i;
	int isize;

	type(disk);
	dfile = open(disk, 0);
	if(dfile < 0) return;
	len = 0;
	seek(dfile, 1, 3);
	read(dfile, &isize, 2);
	seek(dfile, 2, 3);
	isize =<< 4;

	while(isize--) {
		read(dfile, &inode, 32);
		if((inode.flags & 0100000) == 0) continue;
		if(restrict) {
			if(gflg && ulist[t = inode.igid] < 0)
				continue;
			else if(uflg && ulist[t = inode.iuid] < 0)
				continue;
		} else
			if(gflg)
				t = inode.igid;
			else
				t = inode.iuid;
		i = inode.sizeo;
		len = ((inode.sizel>>9) & 0177) + (i<<7);
		if(inode.sizel & 0777) len++;
		if(inode.flags & 010000) {
			aadr = inode.crap;
			for(i = 0; i<8; i++)
				if(*aadr++)
					len++;
		}
	nlist[ulist[t]].owned =+ len;
	}
	close(dfile);
	if(sflg)
		return;
	ptot();
}
#endif

ftype(what)
{
	register int is0, *p, f;
	struct	{
		char major;
		char minor;
		int inumber;
		int iflags;
		char nlinks;
		char suid;
		char sgid;
		char size0;
		int size1;
		int addr[8];
		long time[2];
	} status;

	if(stat(what, &status) == -1)
		return(0);
	is0 = status.size0;
	fsize = ((status.size1>>9)&0177) + (is0<<7);
	if(status.size1 & 0777) fsize++;
	f = status.iflags;
	if(f & 010000) {
		p = status.addr;
		do
			if(*p) fsize++;
		while(p++ < &status.addr[7]);
	}
	if((f & 060000)==040000)
		return(DIR);
	return(0);
}
append(s1, s2, s3, l2, l3)
register char *s1, *s2, *s3;
{
	while(*s1)
	if(--l3)
		*s3++ = *s1++;
	else break;

	*s3++ = '/';
	while(--l2 && --l3 && (*s3++ = *s2++));
	*s3 = '\0';
}

oflush()
{
	/* after a ^C or ^P */
	flush();
	exit(0);
}

#ifndef	AUSAM
ptot()
{
	register int i,t;
	for(i = 0; i<256; i++)
		if((t = ulist[i]) >= 0) {
			typen(nlist[t].owned);
			nlist[t].owned = 0;
			type(nlist[t].who);
		}
}
#endif
