#include "nd.h"

serve(client)
{
	int f;

	if(strcmp(ip->name, "key")==0) {
		char k1[PTLEN], k2[PTLEN];
		if(cpex(client))return;
		if(setjmp(env)) {
			logit("rekey timeout");
		} else {
			arm(120,yyy);
			while(wait((int*)0)!=-1);
			strncpy(k1, getline(client), PTLEN);
			strncpy(k2, getline(client), PTLEN);
			dokey(client, k1, k2);
			memset(k1,0,PTLEN);
			memset(k2,0,PTLEN);
			(void)disarm();
		}
		return;
	}
	if(masterkey[0]==0)
		return;
	if(strcmp(ip->name, "register")==0) {
		if(cpex(client))return;
		if(setjmp(env)) {
			logit("registration timeout");
		} else {
			arm(240,yyy);
			while(wait((int*)0)!=-1);
			(void)handle(client, 1);
			(void)disarm();
		}
		return;
	}


	f = fork();
	if(f==0) {
		pid = getpid();
		if(cpex(client)==0) {
			snarf(1);
			while(handle(client, 0) != -1)
				;
		}
		exit(0);
	}
	else if(f==-1) {
		logit(" fork errno=%d", errno);
		sleep(2);
	}
}

cpex(fd) 
{
	if(pex(fd,120,(struct pexclude*)0)!=0) {
		logit("bad pex");
		return 1;
	} else
		return 0;
}


/*
 * name\n
 * checksum\n
 * bytecount\n			in decimal notation
 * body of certificate
 */

handle(fd, flag)
{
	char *e = 0, *s, *body = 0, csum[100];
	register int wn, n, rv = -1;
	struct item x;

	/*
	 * name
	 */
	if((s=getline(fd))==0||*s==0) {
		e = "x1";
		goto out;
	}
	getitem(s,&x);
	if(x.color!=plaintext) {
		e = "crypto botch 1";
		goto out;
	}

	/*
	 * checksum
	 */
	if((s=getline(fd))==0) {
		e = "x2";
		goto out;
	}
	strncpy(csum, s, sizeof(csum));

	/*
	 * text length
	 */
	if((s=getline(fd))==0) {
		e = "x3";
		goto out;
	}
	n = atoi(s);

	/*
	 * text
	 */
	body = calloc((unsigned)n+1,1);
	if(body==0) {
		e = "bad calloc";
		goto out;
	}
	if(n != read(fd, body, n)) {
		e = "insuf read";
		goto out;
	}
	if(strcmp(csum, s=xs(x.datum, body, n))==0) {
		if(flag) {
			memset(x.datum,0,sizeof(x.datum));
			strncpy(x.datum, body, n);
			arm(0,SIG_IGN);
			putitem(&x);
		}
		wn = write(fd, "yes\n",4);
		rv = 1;
	} else  {
		wn = write(fd, "no\n",3);
		rv = 0;
	}
	logit("%s %s %.8s %s %d", csum, s, rv?".":x.datum, body, n);
	logit("%s %c:%s n=%d", rv?"good":"bad", "vr"[flag], x.name, wn);
out:
	if(e!=0)logit("e=%s",e);
	if(body!=0){
		memset(body,0,n);
		free(body);
	}
	scrubitem(&x);
	return rv;
}

char *
getline(fd)
{
	static char buf[4096];
	register char *s;
	register int n;

	s = buf;
	for(n=0; n<sizeof(buf)-1; n++,s++){
		if(read(fd,s,1)!=1||*s=='\n')break;
	}
	*s = 0;
	return buf;
}
