#include <libc.h>
#include <ipc.h>
#include <sys/pex.h>

extern char *pexpw();
extern char *xs();

int pexipc();
char *getkey();
char *slurp();

main(ac,av)
char **av;
{
	char *s;
	int rv;

	if(ac<2)
		goto usage;

	if(strcmp(av[1], "enroll")==0) {
		rv = enrollfn(ac-2, av+2);
	} else if(strcmp(av[1], "sign")==0) {
		s = getkey("Checksumkey:");
		print("%s\n", slurp(ac>2?av[2]:0, s));
		rv = 0;
	} else if(strcmp(av[1], "key")==0) {
		rv = key();
	} else if(strcmp(av[1], "verify")==0) {
		rv = verifyfn(ac-2,av+2);
	}
	if(rv>=0) return rv;
usage:
	print("usage: notary enroll [-n] name\n");
	print("or: notary sign string\n");
	print("or: notary sign <file\n");
	print("or: notary key\n");
	print("or: notary verify name checksum text\n");
	return 1;
}

char *
getkey(s)
char *s;
{
	s = pexpw(3, s);
	if(s==0) {
		print("cannot pex for key; quitting\n");
		exit(1);
	}
	return s;
}

key()
{
	register fd, n;
	register char *s;
	char buf[1024], k1[8+1], k2[8+1];

	s = getkey("enter secret notary master key:");
	strncpy(k1, s, 8);
	s = getkey("again:");
	if(strncmp(s,k1,8)!=0) {
badmatch:
		print("master keys don't match; quitting\n");
		exit(1);
	}


	s = getkey("new key or CR:");
	if(*s != 0) {
		strncpy(k2, s, 8);
		s = getkey("again:");
		if(strncmp(s,k2,8)!=0) goto badmatch;
	}
	k1[8] = k2[8] = 0;
	fd = keynotary(k1, k2);
	while((n=read(fd,buf,1024))>0)
		write(1,buf,n);
	close(fd);
	return 0;
}
enrollfn(ac, av)
char **av;
{
	int nflag = 0;
	char *opw, *pw;

	if(ac>1 && strcmp("-n", av[0])==0) {
		ac--;
		av++;
		nflag = 1;
	}
	
	if(ac <1) {
		return -1;
	}
	if(nflag == 0) {
		opw = strdup(getkey("Old signature key:"));
	} else
		opw = "";

	pw = strdup(getkey("New signature key:"));
	if(strcmp(pw,getkey("Again:"))!=0)exit(1);

	if(enroll(*av, opw, pw) == 0) {
		print("BAD enroll of %s\n", *av);
		return 1;
	} else {
		print("GOOD enroll of %s\n", *av);
		return 0;
	}
}



char *
slurp(f, key)
char *f, *key;
{
	char *s;
	unsigned ntotal;
	int k, n, nleft;

	if(f!=0) return xs(key, f, strlen(f));
	ntotal = 0;
	nleft = 0;
	n = 0;
	s = calloc(2,1);

	for(;;) {
		if(4096>nleft) {
			k = 4096 + ntotal/2;
			ntotal += k;
			nleft += k;
			s = realloc(s, ntotal);
		}
		k = read(0, s+n, 4096);
		if(k < 1) break;
		n += k;
		nleft -= k;
	}
	return xs(key, s?s:"", n);
}


verifyfn(ac,av)
char **av;
{
	if(ac != 3)
		return -1;

	if(verify(av[0], av[1], av[2], strlen(av[2]))) {
		printf("OK\n");
		return 0;
	} else {
		printf("BAD\n");
		return 1;
	}
}
