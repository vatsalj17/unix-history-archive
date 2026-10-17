#

	/*  fsdb - file system debugger    */
	/*  by Marc Pucci - PY x4441   */

#include   "stat.h"
#include   "stdio.h"
#define    NBUF     3
#define    MODE     0
#define    LINK     2
#define    UID      3
#define    GID      4
#define    S0       5
#define    S1       6
#define    A0       8
#define    MIN      8
#define    MAJ      9
#define    AT      24
#define    MT      28
#define    NUMB    10
#define    INODE   11
#define    BLOCK   12
#define    DIR     13
#define    NM      14

struct {
	int hiword;
	int loword;
};
struct {
	char lobyte;
	char hibyte;
};
struct {
	long dblwrd;
};
struct buf {
	struct  buf  *fwd;
	struct  buf  *back;
	char    *blkaddr;
	int     valid;
	unsigned blkno;
} buf[NBUF], bhdr;
char buffers[NBUF][512];
struct dir {
	int  inumb;
	char nm[14];
} *dirp;
struct inode {
	int   md;
	char  ln;
	char  uid;
	char  gid;
	char  s0;
	int   s1;
	int   a[8];
	long  at;
	long  mt;
}  *ip;
long  addr, value, temp, oldaddr, oldvalue, erraddr;
long  curi 02000;
int  psize  2;
int  fd, c_count, i, j, k, oldpsize, error, type;
int  count, pid, rpid, retcode, mode, cflag, prflag;
unsigned  isize, fsize;
char  *p;
int  override;

/* main - continuously looping input scanner.  Lines are scanned
 * a character at a time and are processed as soon as
 * sufficient information has been collected. When an error
 * is detected, the remainder of the line is flushed.
 */

main(argc,argv)
	int  argc;
	char  **argv;
{

	register  char  *cptr;
	register char c;
	register  int  *iptr;
	extern  long get();
	extern  long getnumb();
	extern  err();
	register struct buf *bp;
	unsigned block;
	int offset;

	setbuf(stdin,NULL);
	if(argc!=2 && argc!=3) {
		printf("usage: fsdb /dev/rp??\n");
		exit(1);
	}

	if((fd = open(argv[1],2)) < 0) {
		printf("cannot open %s\n",argv[1]);
		exit(1);
	}

	bhdr.fwd = bhdr.back = &bhdr;
	for(i=0; i<NBUF; i++) {
		bp = &buf[i];
		bp->blkaddr = buffers[i];
		bp->valid = 0;
		insert(bp);
	}

	override++;
	reload();
	if(argc == 3) printf("error checking off\n");
	else override = 0;

	signal(2,err);
	setexit();

	for(;;) {
		if(error) {
			if(c != '\n') while (getc(stdin) != '\n');
			c_count = 0;
			prflag = 0;
			cflag = 0;
			error = 0;
			addr = erraddr;
			printf("?\n");
			/* type = -1; allows "d31 + +" to work */
		}
		c_count++;

		switch(c = getc(stdin)) {

		case '\n': /* command end */
			erraddr = addr;
			if(c_count == 1) addr =+ psize;
			c_count = 0;
			if(prflag) {
				prflag = 0;
				continue;
			}
			temp = get(psize);
			/* if an error has been flagged, it is probably
			 * due to allignment.  This will have set psize
			 * to 1 hence the next get should work.
			 */
			if(error) temp = get(psize);
			switch(psize) {
			case 1: cptr = ".B"; break;
			case 2: cptr = ""; break;
			case 4: cptr = ".D"; break;
			case 14:
			case 16:
				if(bcomp(addr,erraddr)) continue;
				fprnt('d', 1);
				prflag = 0;
				continue;
			case 32: fprnt('i',1);
				curi = addr;
				prflag = 0;
				continue;
			}
			printf("%6O%s: %6O (%D)\n",addr,cptr,temp,temp);
			continue;

		default: /* catch absolute addresses, b and i#'s */
			if(c<='9' && c>='0') {
				ungetc(c,stdin);
				addr = getnumb();
				psize = 2;
				value = addr;
				type = NUMB;
				continue;
			}
			if(feof(stdin)) exit(0);
			error++;
			continue;

		case 'i': /* i# to inode conversion */
			if(c_count == 1) {
				addr = curi;
				value = get(32);
				type = INODE;
				continue;
			}
			if(type==NUMB)value = addr;
			addr = ((value - 1) << 5) + 1024;
			if(icheck(addr)) continue;
			curi = addr;
			value = get(32);
			type = INODE;
			continue;

		case 'b': /* block conversion */
			if(type == NUMB)value = addr;
			addr = value << 9;
			value = get(2);
			type = BLOCK;
			continue;

		case 'd': /* directory offsets */
			value = getnumb();
			if(error||(value > 31)) {
				error++;
				continue;
			}
			if(value != 0) if(dircheck()) continue;
			addr = (addr & ~0777) + (value << 4);
			value = get(16); /* i-number */
			type = DIR;
			continue;

		case '\t':
		case ' ':
		case '.': continue;

		case '+': /* address addition */
			c = getc(stdin);
			ungetc(c,stdin);
			if(c > '9' || c < '0') temp = psize;
			else temp = getnumb() * psize;
			if(error) continue;
			if(psize == 14 || psize == 16)
				if(bcomp(addr,addr+temp)) {
					c = '+';
					continue;
				}
			addr =+ temp;
			value = get(psize);
			continue;

		case '-': /* address subtraction */
			c = getc(stdin);
			ungetc(c,stdin);
			if(c > '9' || c < '0') temp = psize;
			else temp = getnumb() * psize;
			if(error) continue;
			if(psize == 14 || psize == 16)
				if(bcomp(addr,addr-temp)) {
					c = '-';
					continue;
				}
			addr =- temp;
			value = get(psize);
			continue;

		case '*': temp = getnumb();
			if(error) continue;
			addr =* temp;
			value = get(psize);
			continue;

		case '/': temp = getnumb();
			if(error) continue;
			addr =/ temp;
			value = get(psize);
			continue;

		case 'q': /* quit */
			if(c_count != 1 || (c = getc(stdin)) != '\n') {
				error++;
				continue;
			}
			exit(0);

		case '>': /* save current address */
			oldaddr = addr;
			oldvalue = value;
			oldpsize = psize;
			continue;

		case '<': /* restore saved address */
			addr = oldaddr;
			value = oldvalue;
			psize = oldpsize;
			continue;

		case 'a': /* access time */
			if((c = getc(stdin)) == 't') {
				addr = curi + AT;
				type = AT;
				value = get(4);
				continue;
			}
			ungetc(c,stdin);
			/* data block addresses */
			value = getnumb();
			if(error||(value > 7)) {
				error++;
				continue;
			}
			addr = curi + A0 + (value << 1);
			value = get(2);
			type = A0;
			continue;

		case 'm': /* mt, md, maj, min */
			addr = curi;
			mode = get(2);
			if(error) continue;
			switch(c = getc(stdin)) {
			case 't': /* modification time */
				addr =+ MT;
				type = MT;
				value = get(4);
				continue;
			case 'd': /* mode */
				addr =+ MODE;
				type = MODE;
				value = get(2);
				continue;
			case 'a': /* major device number */
				if((c = getc(stdin)) != 'j') {
					error++;
					continue;
				}
				if(devcheck(mode)) continue;
				addr =+ MAJ;
				value = get(1);
				type = MAJ;
				continue;
			case 'i': /* minor device number */
				if((c = getc(stdin)) != 'n') {
					error++;
					continue;
				}
				if(devcheck(mode)) continue;
				addr =+ MIN;
				value = get(1);
				type = MIN;
				continue;
			}
			error++;
			continue;

		case 's': /* file size */
			switch(c = getc(stdin)) {
			case '0': /* high byte of file size */
				addr = curi + S0;
				value = get(1);
				type = S0;
				continue;
			case '1': /* low word of file size */
				addr = curi + S1;
				value = get(2);
				type = S1;
				continue;
			default: error++;
				continue;
			}

		case 'l': /* link count */
			if((c = getc(stdin)) != 'n') {
				error++;
				continue;
			}
			addr = curi + LINK;
			value = get(1);
			type = LINK;
			continue;

		case 'g': /* group id */
			if((c=getc(stdin))!= 'i' || (c=getc(stdin)) != 'd') {
				error++;
				continue;
			}
			addr = curi + GID;
			value = get(1);
			type = GID;
			continue;

		case 'u': /* user id */
			if((c=getc(stdin))!= 'i' || (c=getc(stdin)) != 'd') {
				error++;
				continue;
			}
			addr = curi + UID;
			value = get(1);
			type = UID;
			continue;

		case 'n': /* directory name */
			if((c = getc(stdin)) != 'm') {
				error++;
				continue;
			}
			if(dircheck()) continue;
			type = NM;
			psize = 14;
			addr = (addr & ~017) + 2;
			continue;

		case '=': /* assignment operation  */
			switch(c = getc(stdin)) {
			case '"': /* character string */
				puta();
				continue;
			case '+': /* =+ operator */
				temp = getnumb();
				value = get(psize);
				if(!error) put(value+temp,psize);
				continue;
			case '-': /* =- operator */
				temp = getnumb();
				value = get(psize);
				if(!error) put(value-temp,psize);
				continue;
			default: /* nm and regular assignment */
				ungetc(c,stdin);
				if((type == NM) && (c > '9' || c < '0')) {
					puta();
					continue;
				}
				value = getnumb();
				if(!error) put(value,psize);
				continue;
			}

		case '!': /* shell command */
			if(c_count != 1) {
				error++;
				continue;
			}
			if((pid = fork()) == 0) {
				execl("/bin/sh", "sh", "-t", 0);
				error++;
				continue;
			}
			while((rpid = wait(&retcode)) != pid && rpid != -1);
			printf("!\n");
			c_count = 0;
			continue;

		case 'F': /* buffer status */
			for(bp=bhdr.fwd; bp!= &bhdr; bp=bp->fwd)
				printf("%6u %d\n",bp->blkno,bp->valid);
			continue;

		case 'f': /* file print facility */
			if((c=getc(stdin)) >= '0' && c <= '9') {
				ungetc(c,stdin);
				temp = getnumb();
				if (error) continue;
				c = getc(stdin);
			}
			else temp = 0;
			count = 0;
			addr = curi;
			mode = get(2);
			if(!override) {
				if((mode & IALLOC)==0)
					printf("warning: inode not allocated\n");
			}

			if(mode & IFCHR) {
				printf("special device\n");
				error++;
				continue;
			}

			if(mode & ILARG) {
				addr = curi + ((temp >> 8) << 1) + A0;
				addr = get(2) << 9;
				if(nullblk()) continue;
				addr = addr + ((temp & 0377) << 1);
				addr = get(2) << 9;
				if(nullblk()) continue;
				fprnt(c,0);
				continue;
			}

			if(temp > 7) {
				printf("small file\n");
				error++;
				continue;
			}
			addr = curi + (temp << 1) + A0;
			addr = get(2) << 9;
			if(nullblk()) continue;
			fprnt(c,0);
			continue;

		case 'O': /* override flip flop */
			if(override = !override)
				printf("error checking off\n");
			else {
				printf("error checking on\n");
				reload();
			}
			prflag++;
			continue;

		case 'B': /* byte offsets */
			psize = 1;
			continue;

		case 'W': /* word offsets */
			psize = 2;
			addr =& ~01;
			continue;

		case 'D': /* double word offsets */
			psize = 4;
			addr =& ~03;
			continue;

		case ',': /* general print facilities */
		case 'p':
			if(( c = getc(stdin)) >= '0' && c <= '9') {
				ungetc(c,stdin);
				count = getnumb();
				if(error) continue;
				c = getc(stdin);
			}
			else count = 1;
			fprnt(c,count);
		}
	}
}

/* getnumb - read a number from the input stream.  A leading
 * zero signifies octal interpretation. If the first character
 * is not numeric this is an error, otherwise continue
 * until the first non-numeric.
 */

long
getnumb()
{

	extern  int  error;
	long  number, base;
	register  char  c;

	if(((c = getc(stdin)) < '0')||(c > '9')) {
		error++;
		ungetc(c,stdin);
		return(-1);
	}
	if(c == '0') base = 8;
	else base = 10;
	number = c - '0';
	while(((c = getc(stdin)) >= '0' )&&( c <= '9')) {
		if((base == 8) && ((c =='8')||(c == '9'))) {
			error++;
			return(-1);
		}
		number = number * base + c - '0';
	}
	ungetc(c,stdin);
	return(number);
}

/* get - read a byte, word or double word from the file system.
 * The entire block containing the desired item is read
 * and the appropriate data is extracted and returned. 
 * Inode and directory size requests result in word
 * fetches. Directory names (psize == 14) result in byte
 * fetches.
 */

long
get(lngth)
	int  lngth;
{

	long  vtemp;
	char *bptr;
	unsigned block;
	int offset;

	psize = lngth;
	if(allign(psize)) return(-1);
	block = addr >> 9;
	if((bptr = getblk(block)) == 0) return(-1);
	vtemp = 0;
	offset = addr & 0777;
	bptr =+ offset;
	if(offset + lngth > 512) {
		error++;
		return(-1);
	}
	switch(psize) {
	case 4: return(bptr->dblwrd);
	case 32:
	case 16:
	case 2: vtemp.loword = bptr->hiword;
		return(vtemp);
	case 14:
	case 1: vtemp.loword.lobyte = *bptr;
		return(vtemp);
	}
	error++;
	return(-1);
}

/* icheck - check if the current address is within the I-list.
 * The I-list extends for isize blocks beyond the 
 * super block, i.e., from block 2 to block isize + 1.
 */

icheck(address)
	long  address;
{
	unsigned blk;

	if(override) return(0);
	blk = address >> 9;
	if((blk >= 2) && (blk < isize + 2))
		return(0);
	printf("inode out of range\n");
	error++;
	return(1);
}

/* putf - print a byte as an ascii character if possible.
 * The exceptions are tabs, newlines, backslashes
 * and nulls which are printed as the standard c
 * language escapes. Characters which are not
 * recognized are printed as \?.
 */

putf(c)
	register  char  c;
{

	if(c<=037 || c>=0177 || c=='\\') {
		putc('\\',stdout);
		switch(c) {
		case '\\': putc('\\',stdout);
			break;
		case '\t': putc('t',stdout);
			break;
		case '\n': putc('n',stdout);
			break;
		case '\0': putc('0',stdout);
			break;
		default: putc('?',stdout);
		}
	}
	else {
		putc(' ',stdout);
		putc(c,stdout);
	}
	putc(' ',stdout);
}

/* put - write an item into the buffer for the current address
 * block.  The value is checked to make sure that it will
 * fit in the size given without truncation.  If successful,
 * the entire block is written back to the file system.
 */

put(item,lngth)
	long item;
	int lngth;
{

	register char *bptr, *sbptr;
	int offset;
	unsigned block;

	psize = lngth;
	if(allign(psize)) return(-1);
	block = addr >> 9;
	if((sbptr = getblk(block)) == 0) return;
	offset = addr & 0777;
	if(offset + lngth > 512) {
		error++;
		printf("block overflow\n");
		return;
	}
	bptr = sbptr + offset;
	switch(psize) {
	case 4: bptr->dblwrd = item;
		goto rite;
	case 32:
	case 16:
	case 2: if(item & ~0177777L) break;
		bptr->hiword = item.loword;
		goto rite;
	case 14:
	case 1: if(item & ~0377) break;
		*bptr = item.loword.lobyte;
	rite:   if(seek(fd,block,3)||write(fd,sbptr,512)!=512)
			error++;
		return;
	default: error++;
		return;
	}
	printf("truncation error\n");
	error++;
}

/* getblk - check if the desired block is in the file system.
 * Search the incore buffers to see if the block is already
 * available. If successful, unlink the buffer control block
 * from its position in the buffer list and re-insert it at
 * the head of the list.  If failure, use the last buffer
 * in the list for the desired block. Again, this control
 * block is placed at the head of the list. This process
 * will leave commonly requested blocks in the in-core buffers.
 * Finally, a pointer to the buffer is returned.
 */

getblk(block)
	unsigned block;
{

	register struct buf *bp;

	if(!override)
		if(block >= fsize) {
			printf("block out of range\n");
			error++;
			return(0);
		}
	for(bp=bhdr.fwd; bp!= &bhdr; bp=bp->fwd)
		if(bp->blkno==block && bp->valid) goto xit;
	bp = bhdr.back;
	bp->blkno = block;
	bp->valid = 0;
	if(seek(fd,block,3)||read(fd,bp->blkaddr,512)!=512) {
		error++;
		return(0);
	}
	bp->valid++;
xit:	bp->back->fwd = bp->fwd;
	bp->fwd->back = bp->back;
	insert(bp);
	return(bp->blkaddr);
}

/* insert - place the designated buffer control block
 * at the head of the linked list of buffers.
 */

insert(bp)
	register struct buf *bp;
{

	bp->back = &bhdr;
	bp->fwd = bhdr.fwd;
	bhdr.fwd->back = bp;
	bhdr.fwd = bp;
}

/* allign - before a get or put operation check the 
 * current address for a boundary corresponding to the 
 * size of the object.
 */

allign(ment)
	int ment;
{

	switch(ment) {
	case 4: if(addr & 03L) break;
		return(0);
	case 14: if((addr & 017) != 02) break;
		return(0);
	case 16:
	case 32:
	case 2: if(addr & 01L) break;
	case 1: return(0);
	}
	error++;
	psize = 1;
	printf("allignment\n");
	return(1);
}

/* err - called on interrupts.  Set the current address
 * back to the last address stored in erraddr. Reset all
 * appropriate flags.  If the prflag is set, the interrupt
 * occured while transferring characters to a buffer. 
 * These are "erased" by invalidating the buffer, causing
 * the entire block to be re-read upon the next reference.
 * A reset call is made to return to the main loop;
 */

err()
{
	signal(2,err);
	addr = erraddr;
	error = 0;
	c_count = 0;
	if(cflag) {
		bhdr.fwd->valid = 0;
		cflag = 0;
	}
	prflag = 0;
	printf("\n?\n");
	fseek(stdin, 0L, 2);
	reset();
}

/* devcheck - check that the given mode represents a 
 * special device. The IFCHR bit is on for both
 * character and block devices.
 */

devcheck(md)
	register int md;
{
	if(override) return(0);
	if(md & IFCHR) return(0);
	printf("not char or block device\n");
	error++;
	return(1);
}

/* nullblk - return error if address is zero.  This is done
 * to prevent block 0 from being used as an indirect block
 * for a large file or as a data block for a small file.
 */

nullblk()
{
	if(addr != 0) return(0);
	printf("non existent block\n");
	error++;
	return(1);
}

/* dircheck - check if the current address can be in a directory.
 * This means it is not in the I-list, block 0 or the super block.
 */

dircheck()
{
	unsigned block;

	if(override) return (0);
	if((block = (addr >> 9)) > isize) return(0);
	error++;
	printf("block in I-list\n");
	return(1);
}

/* puta - put ascii characters into a buffer.  The string
 * terminates with a quote or newline.  The leading quote,
 * which is optional for directory names, was stripped off
 * by the assignment case in the main loop.  If the type
 * indicates a directory name, the entry is null padded to
 * 14 bytes.  If more than 14 characters have been given
 * with this type or, in any case, if a block overflow
 * occurs, the current block is made invalid. See the 
 * description for err.
 */

puta()
{
	register char *bptr, c;
	register offset;
	char *sbptr;
	unsigned block;

	if((sbptr = getblk(block = addr >> 9)) == 0) return;
	cflag++;
	offset = addr & 0777;
	bptr = sbptr + offset;

	while((c = getc(stdin)) != '"') {
		if(offset++ == 512) {
			bhdr.fwd->valid = 0;
			error++;
			cflag = 0;
			printf("block overflow\n");
			return;
		}
		if(c == '\n') {
			ungetc(c,stdin);
			break;
		}
		if(c == '\\') {
			switch(c = getc(stdin)) {
			case 't': *bptr++ = '\t'; break;
			case 'n': *bptr++ = '\n'; break;
			case '0': *bptr++ = '\0'; break;
			default: *bptr++ = c; break;
			}
		}
		else *bptr++ = c;
	}
	cflag = 0;
	if(type == NM) {
		c = offset - (addr & 0777);
		if(c > 14) {
			bhdr.fwd->valid = 0;
			error++;
			cflag = 0;
			printf("name too long\n");
			return;
		}
		while(c++ < 14) *bptr++ = '\0';
	}
	if(seek(fd,block,3) || write(fd,sbptr,512) != 512)
		error++;
}

/* fprnt - print data as characters, octal or decimal words, octal
 * bytes, directories or inodes  A total of "count" entries
 * are printed. A zero count will print all entries to the 
 * end of the current block.  If the printing would cross a
 * block boundary, the attempt is aborted and an error returned.
 * This is done since logically sequential blocks are generally
 * not physically sequential.  The error address is set
 * after each entry is printed.  Upon completion, the current
 * address is set to that of the last entry printed.
 */

fprnt(style,count)
	register char style;
	register int  count;
{
	int offset;
	unsigned block;
	char *cptr;
	int *iptr;

	prflag++;
	block = addr >> 9;
	offset = addr & 0777;
	if((cptr = iptr = ip = dirp = getblk(block)) == 0)
		return;
	erraddr = addr;

	switch (style) {

	case 'c': /* print as characters */
	case 'b': /* or octal bytes */
		if(count == 0) count = 512 - offset;
		if(offset + count > 512) break;
		psize = 1;
		cptr =+ offset;
		for(i=0; count--; i++) {
			if(i % 16 == 0)
				printf("\n%6O: ",addr);
			if(style == 'c') putf(*cptr++);
			else printf("%4o",*cptr++ & 0377);
			erraddr = addr;
			addr++;
		}
		addr--;
		putc('\n',stdout);
		return;

	case 'd': /* print as directories */
		if(dircheck()) return;
		addr =& ~017;
		offset =>> 4;
		if(count == 0) count = 32 - offset;
		if(count + offset > 32) break;
		type = DIR; 
		psize = 16;
		for(dirp =+ offset; count--; dirp++) {
			printf("d%d: %4d  ",offset++,dirp->inumb);
			cptr = &(dirp->nm);
			for(j=0; j<14; j++) {
				if(*cptr == '\0') break;
				putf(*cptr++);
			}
			putc('\n',stdout);
			erraddr = addr;
			addr =+ 16;
		}
		addr = erraddr;
		return;

	case 'o': /* print as octal words */
	case 'e': /* print as decimal words */
		addr =& ~01;
		offset =>> 1;
		if(count == 0) count = 256 - offset;
		if(offset + count > 256) break;
		psize = 2;
		iptr =+ offset;
		for(i=0; count--; i++) {
			if(i % 8 == 0)
				printf("\n%6O:",addr);
			if(style == 'o')printf("  %6o",*iptr++);
			else printf("  %6d",*iptr++);
			erraddr = addr;
			addr =+ 2;
		}
		addr = erraddr;
		putc('\n',stdout);
		return;

	case 'i': /* print as inodes */
		if(icheck(addr)) return;
		addr =& ~037;
		offset =>> 5;
		if(count == 0) count = 16 - offset;
		if(count + offset > 16) break;
		type = INODE;
		psize = 32;
		ip =+ offset;
		temp = (addr - 1024) / 32 + 1;
		for(i=0; count--; ip++) {
			printf("i#:%5D  md: ",temp++);
			p = " lugtrwxrwxrwx";
			mode = ip->md;
			if(mode & IALLOC) putc('a',stdout);
			else putc('-',stdout);
			switch(mode & IFMT) {
			case IFDIR: putc('d',stdout); break;
			case IFCHR: putc('c',stdout); break;
			case IFBLK: putc('b',stdout); break;
			default: putc('-',stdout); break;
			}
			for(mode =<< 3; *++p; mode =<< 1) {
				if(mode & IALLOC) putc(*p,stdout);
				else putc('-',stdout);
			}
			printf("  ln:%4d  uid:%4d  gid:%4d",
				ip->ln&0377,ip->uid&0377,ip->gid&0377);
			printf("  s0:%4d  s1:%6u\n",ip->s0&0377,ip->s1);
			if(ip->md & IFCHR) 
				printf("maj:%3o  min:%3o  ",ip->a[0].hibyte,
					ip->a[0].lobyte);
			else {
				printf("a0:%5u  a1:%5u  a2:%5u  a3:%5u  ",
				ip->a[0],ip->a[1],ip->a[2],ip->a[3]);
				printf("a4:%5u  a5:%5u  a6:%5u  a7:%5u\n",
				ip->a[4],ip->a[5],ip->a[6],ip->a[7]);
			}
			p = ctime(&ip->at);
			p[24] = '\0';
			printf("at: %s  ",p);
			printf("mt: %s",ctime(&ip->mt));
			if(count) putc('\n',stdout);
			curi = erraddr = addr;
			addr =+ 32;
		}
		addr = erraddr;
		return;


	default: error++;
		printf("no such print option\n");
		return;
	}
	error++;
	printf("block overflow\n");
}

/* reload - read new values for isize and fsize. These are
 * the basis for most of the error checking procedures.
 */

reload()
{
	long saddr;
	int  spsize;

	saddr = addr;
	spsize = psize;
	addr = 01000;
	isize = get(2);
	addr = 01002;
	fsize = get(2);
	addr = saddr;
	psize = spsize;
	if(error) printf("cannot read superblock\n");
}

/* bcomp - compare the block numbers of two long addresses.
 * Used to check for block over/under flows when stepping through
 * a file system.
 */

bcomp(addr1,addr2)
	long addr1;
	long addr2;
{
	if(override) return(0);
	if((addr1 & ~0777L) == (addr2 & ~0777L)) return(0);
	error++;
	printf("block overflow\n");
	return(1);
}
