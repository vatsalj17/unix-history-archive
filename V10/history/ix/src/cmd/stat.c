#include <sys/types.h>
#include <sys/stat.h>
#include <sys/label.h>
#include <grp.h>
#include <pwd.h>

typedef unsigned long UL;
extern char *ctime();
errcnt = 0;

main(argc, argv)
char **argv;
{
	struct stat st, fst;
	struct label lb, flb;
	int fd;

	nochk(1, 0);
	nochk(2, 0);	/* hygiene; stat doesn't use 2 */
	for( ; argc>1; flush(), argc--, argv++) {
		output(argv[1]);
		output(":\n");
		if(lstat(argv[1], &st) < 0 ||
		   getflab(argv[1], &lb) < 0) {
			error();
			continue;
		}
		fd = open(argv[1], 0);
		if(fd == -1)
			fd = open(argv[1], 1);
		if(fd!=-1 && (fstat(fd, &fst)<0 ||
		    fgetflab(fd, &flb)<0)) {
			error(); /* improbable */
			close(fd);
			continue;
		}
		pstat(&st);
		ptime(&st.st_mtime);
		ptime(&st.st_atime);
		ptime(&st.st_ctime);
		plab(&lb);
		if((st.st_mode&S_IFMT) == S_IFLNK) {
			plink(argv[1]);
			continue;
		} else if(fd == -1)
			continue;
		if(cmp((char*)&st, (char*)&fst,
		   (char*)&st.st_atime-(char*)&st) != 0)
			pstat(&fst);
		if(cmp((char*)&lb, (char*)&flb, sizeof(lb)) != 0)
			plab(&flb);
		close(fd);
	}
	return errcnt;
}
pstat(sp)
struct stat *sp;
{
	output("\t");
	itoa((UL)sp->st_ino, 10); output(" ");
	mode(sp); output(" ");
	itoa((UL)sp->st_nlink, 10); output(" ");
	owner(sp); output(" ");
	group(sp); output(" ");
	size(sp); output("\n\t");
	pdev(sp->st_dev); output(" ");
	output("0"); itoa((UL)sp->st_mode, 8); output("\n");
}
mode(sp)
struct stat *sp;
{
	int ifmt = sp->st_mode & S_IFMT;
	int cctyp = sp->st_mode & S_ICCTYP;
	int setboth = cctyp==(S_ISUID|S_ISGID);
	output (ifmt==S_IFDIR? "d":
		ifmt==S_IFCHR? "c":
		ifmt==S_IFBLK? "b":
		ifmt==S_IFLNK? "l":
		ifmt==S_IFREG? " ":
		"?");
	rwx((sp->st_mode>>6) & 07, cctyp==S_ISUID||setboth);
	rwx((sp->st_mode>>3) & 07, cctyp==S_ISGID||setboth);
	rwx((sp->st_mode>>0) & 07, 0);
	output (cctyp==S_ISYNC? "y":
		cctyp==S_IEXCL? "e":
		cctyp==S_IAPPEND? "a":
		cctyp==S_IBLIND? "b":
		"");
}
owner(sp)
struct stat *sp;
{
	struct passwd *pw = getpwuid(sp->st_uid);
	if(pw == 0)
		itoa((UL)sp->st_uid, 10);
	else
		output(pw->pw_name);
	setpwent();
}
group(sp)
struct stat *sp;
{
	struct group *gr = getgrgid(sp->st_gid);
	if(gr == 0)
		itoa((UL)sp->st_gid, 10);
	else
		output(gr->gr_name);
	setgrent();
}
size(sp)
struct stat *sp;
{
	int ifmt = sp->st_mode & S_IFMT;
	if(ifmt==S_IFCHR || ifmt==S_IFBLK) 
		pdev(sp->st_rdev);
	else
		itoa((UL)sp->st_size, 10);
}
plink(alias)
char *alias;
{
	char name[1024];
	int n = readlink(alias, name, sizeof(name)-1);
	if(n < 0)
		error();	/* improbable */
	name[n] = 0;
	output("\t-> ");
	output(name); output("\n");
}	 
plab(lbp)
struct label *lbp;
{
	extern char *labtoa();
	output("\t");
	output(labtoa(lbp));
	output("\n");;
}
pdev(dev)
dev_t dev;
{
	itoa((UL)major(dev), 10);
	output(",");
	itoa((UL)minor(dev), 10);
}
short shortone = -1;
itoa(n, b)
UL n;
{
	char s[2];
	if(n == shortone) {
		output("-1");	/* special dispensation */
		return;
	}
	s[0] = n%b + '0';
	s[1] = 0;
	n /= b;
	if(n) itoa(n, b);
	output(s);
}
rwx(n, suid)
unsigned n;
{
	output(n&4? "r": "-");
	output(n&2? "w": "-");
	output(suid? "s": n&1? "x": "-");
}
ptime(tp)
long *tp;
{
	output("\t");
	output(ctime(tp)+4);
}

char buffer[1024];
char *next = buffer;
output(s)
char *s;
{
	while(*s)
		*next++ = *s++;
}
flush()
{
	write(1, buffer, next-buffer);
	next = buffer;
}
error()
{
	extern errno, sys_nerr;
	extern char *sys_errlist[];
	output("\t");
	if(errno<0 || errno>=sys_nerr) {
		output("error ");
		itoa((UL)errno, 10);
	} else
		output(sys_errlist[errno]);
	output("\n");;
	flush();
	errno = 0;
	errcnt++;
}
cmp(a,b,n)
char *a, *b;
{
	while(--n >= 0)
		if(*a++ != *b++)
			return 1;
	return 0;
}
