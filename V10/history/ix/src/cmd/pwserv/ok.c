#include "pwserv.h"

struct entry nono = {(char*)0, (char*)0, (char*)0};

ok(fd, s, type, src)
register char *s;
char *type, *src;
{
	register struct entry *p;
	char *resp;
	char buf[100], challenge[20];
	struct sgttyb old, new;
	int x, n;

	p = getent(s);
	if(p==0)p = &nono;

	ioctl(fd, TIOCGETP, &old);
	new = old;
	new.sg_flags &= ~ECHO;
	resp = atalla(p->atalla, challenge);
	x = !( pex(fd, 10, (struct pexclude*)0) 
		&& pex(fd, -1, (struct pexclude*)0));
	if(autopw(s, src)) {
		if(resp == 0) resp = "";
		strncpy(buf, resp, sizeof(buf));
	}
	else {
		ioctl(fd, TIOCSETP, &new);
		if(x)
			fprint(fd, "Password(%s:%s):", s, challenge);
		else
			fprint(fd, "Password(TAPPED LINE:%s):", challenge);
		for(n=0; n<sizeof(buf)-1; n++) {
			buf[n] = 0;
			if(read(fd, &buf[n], 1)!=1)
				break;
			if(buf[n] == '\n' || buf[n] == '\r')
				break;
		}
		unpex(fd, -1);
		ioctl(fd, TIOCSETP, &old);
		if(buf[n] == '\n' || buf[n] == '\r') {
			fprint(fd, "\n");
		}
		buf[n] = 0;
	}
	close(fd);
	if(strcmp(type, "pex")==0 && x==0)
		return 0;
	if(resp != 0 && strncmp(resp, buf, 5)==0)
		return 1;
	if(x!=0) {
		if(p->pw && strcmp(p->pw, crypt(buf, p->pw))==0)
			return 1;
	}
	return 0;
}

autopw(s, src)
char *s, *src;
{
	char *t;

	t = strchr(src?src:"", ':');
	if(t == 0)
		return 0;
	return !strcmp(t+1,s);
}
