#include "nd.h"

char selfname[] = "(!notary!)";
bfile *cbt = 0;

snarf(flag)
{
	static long otime;
	struct stat sb;

	if(stat(dbf, &sb)!=-1) {
		if(flag!=0 || sb.st_mtime!=otime) {
			logit( "snarf(%s)", db);
			if(cbt != 0) bclose(cbt);
			if((cbt = bopen(db, 2))==0) {
				logit("death: cannot bopen(%s)", db);
				exit(1);
			}

			otime = sb.st_mtime;
		}
	} else {
		logit("bad stat(%s)", dbf);
		if(flag)exit(1);
	}
}

dokey(client, k1, k2)
char *k1, *k2;
{
	struct item x;
	register int t, i;
	char newkey[PTLEN];

	t = ttyfd;
	ttyfd = client;

	strncpy(masterkey, k1, PTLEN);
	for(i=0; i<PTLEN; i++) masterkey[i] ^= 'M'+i;
	getitem(selfname,&x);
	if(strncmp(x.datum, x.datum+4, 4)!=0) {
		strncpy(masterkey, "", PTLEN);
		logit("Sorry");
		ttyfd = t;
		scrubitem(&x);
		return;
	}
	spatter(x.datum,4);
	strncpy(x.datum+4, x.datum,4);
	putitem(&x);
	scrubitem(&x);

	if(*k2 != 0) {
		strncpy(newkey, k2, PTLEN);
		for(i=0; i<PTLEN; i++) newkey[i] ^= 'M'+i;
		rekey(masterkey, newkey);
		memcpy(masterkey, newkey, PTLEN);
		memset(newkey, 0, PTLEN);
	}
	ttyfd = t;
}

rekey(old, new)
register char *old, *new;
{
	int er;
	struct item x;

	logit("rekeying");
	if(bfirst(cbt)>=0) {
		backup(glue3(db,"",".T"), glue3(odb,"",".T"));
		backup(glue3(db,"",".F"), glue3(odb,"",".F"));
		x.user.mdata = x.name;
		x.val.mdata = x.datum;
		while(bread(cbt, &x.user, &x.val)==0){
			x.color = ciphertext;
			code(&x, old);
			spatter(x.datum+PTLEN, INDLEN);
			code(&x, new);
			if(x.color==plaintext)logit("3crypto botch");
			
			if( bwrite(cbt, x.user, x.val)<0) er = 2;
			else if( bseek(cbt, x.user)!=1) er = 3;
			else if( bread(cbt, &x.user, &x.val)!=0) er = 4;
			else er = 0;
			if(er>0) {
				logit("rekey botch, er %d", er);
				exit(1);
			}
		}
	}
	logit("rekey done");
}

backup(from,to)
register char *from, *to;
{
	char *f, buf[1024];
	register int n, in, out;

	in = open(f=from, 0);
	if(in == -1) {
badopen:
		logit("cannot open %s for backup", f);
		exit(1);
	}
	out = creat(f=to, 0600);
	if(in == -1)
		goto badopen;

	while((n=read(in,buf,1024))>0)
		if(write(out,buf,n)!=n) {
			logit("bad write in backup %s", to);
			exit(1);
		}
	free(from);
	free(to);
	close(in);
	close(out);
}

getitem(s, it)
register char *s;
struct item *it;
{
	scrubitem(it);

	strncpy(it->name, s, sizeof(it->name));
	it->user.mdata = it->name;
	it->user.mlen = namlen(it->name);
	it->val.mdata = it->datum;
	if(bseek(cbt, it->user)==1) {
		if(bread(cbt, (mbuf*)0, &it->val)==0) {
			it->color = ciphertext;
			code(it, masterkey);
		} else logit("alpha");
	} else logit("beta");
}

putitem(it)
register struct item *it;
{
	it->user.mdata = it->name;
	it->user.mlen = namlen(it->name);
	it->val.mdata = it->datum;
	it->val.mlen = sizeof(it->datum);

	pex(cbtfd,0,(struct pexclude*)0);
	if(it->datum[0]==0) {
		memset(it->datum, 0, sizeof(it->datum));
		bdelete(cbt, it->user);
	} else {
		spatter(it->datum+PTLEN, INDLEN);
		code(it, masterkey);
		if(it->color!=ciphertext)
			logit("2crypto botch");
		bwrite(cbt, it->user, it->val);
	}
	snarf(1);
	unpex(cbtfd,0);
}

scrubitem(it)
struct item *it;
{
	if(it==0)return;
	memset((char*)it,0,sizeof(struct item));
	it->color = plaintext;
}

spatter(s,n)
char *s;
{
	while(n-->0) *s++ = 1+nrand(255);
}

code(it, key)
register struct item *it;
register char *key;
{
	register int x, i;
	char *ind, *s, c;
	unsigned long xsum();

	s = it->datum;
	ind = s+PTLEN;
	xinit();
	xstring(ind, INDLEN);
	xstring(key, PTLEN);
	for(i=0; i<PTLEN; i++) {
		xstring(ind, INDLEN);
		xstring(key, PTLEN);
		xstring(it->name, namlen(it->name));
		x = 0xff & *s;
		*s ^= 0xff&(xsum()>>24);
		x *= 0xff & *s;
		s++;
		c = x;
		xstring(&c,1);
	}
	it->color ^= 1;
}

namlen(s)
register char *s;
{
	register i;
	for(i=0; i<NAMLEN; i++)
		if(*s++ == 0) break;
	return i;
}
