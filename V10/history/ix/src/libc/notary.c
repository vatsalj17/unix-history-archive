#include <libc.h>
#include <ipc.h>
#include <sys/pex.h>

extern char *pexpw();
extern char *xs();

static char mtpt[] = "/cs/notary";
static char nambuf[20+sizeof(mtpt)];

static pexipc(name)
char *name;
{
	int fd;
	struct pexclude pexbuf;

	strcpy(nambuf, mtpt);
	strcat(nambuf, name);
	if((fd = ipcopen(nambuf, "light"))==-1)
		return -1;
	if(pex(fd, 60, &pexbuf)!=0
	    ||pexbuf.farpid<=0
	    ||pexbuf.farcap==0) {
		close(fd);
		return -1;
	}
	return fd;
}

static yesno(fd)
{
	char buf[4];
	int i;

	for(i=0; i<4; i++) {
		buf[i] = 0;
		if(read(fd,&buf[i],1)!=1)return 0;
		if(strncmp(buf, "yes\n",4)==0)return 1;
		if(strncmp(buf, "no\n",3)==0)return 0;
	}
	return 0;
}

keynotary(k1, k2)
char *k1, *k2;
{
	int fd = pexipc("!key");
	if(fd != -1)
		fprint(fd, "%s\n%s\n", k1, k2);
	return fd;
}

rverify(name, xsum, body, n)
char *name, *xsum, *body;
{
	static int vfd = 0;
	
	if(name==0) {
		if(vfd>0)close(vfd);
		vfd = 0;
		return 0;
	}
	if(vfd == 0)
		vfd = pexipc("");
	if(vfd == -1) return 0;

	fprint(vfd, "%s\n%s\n%d\n", name, xsum, n);
	write(vfd, body,n);
	return yesno(vfd);
}

verify(name, xsum, body, n)
char *name, *xsum, *body;
{
	int r, fd = pexipc("");
	if(fd==-1)return 0;
	fprint(fd, "%s\n%s\n%d\n", name, xsum, n);
	write(fd, body,n);
	r = yesno(fd);
	close(fd);
	return r;
}

enroll(name, opw, npw) 
char *name, *opw, *npw;
{
	char buf[9];
	int r, fd, n = strlen(npw);

	strncpy(buf,opw,9);
	fd = pexipc("!register");
	if(fd==-1)return 0;
	fprint(fd, "%s\n%s\n%d\n", name, xs(buf,npw,n), n);
	write(fd, npw,n);
	r = yesno(fd);
	close(fd);
	return r;
}
