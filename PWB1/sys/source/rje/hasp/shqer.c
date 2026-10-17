#define USR 180
#define QLEN 200
#define QUEDIR "/usr/rje/sque"
#define PFILE "/usr/rje/sque/pfile"
#define QUELOG	"log"
char *sh, *p2;
char buf[QLEN+4], *ddv;
int ds;
int siz,sb[18];
char fspce[2];
int tout;
int i,j,k;

int cont();


struct qdat {
	char *jnb;	/*job number*/
	char *jnm;	/*job name*/
	char *pnm;	/*programmer name*/
	char *fnm;	/*file name to execl*/
	char *shf;	/*full path to execl*/
	char *dir;	/*directory of execl file*/
	char *unm;	/*login name from usr=*/
	char *qfl;	/*standard input file*/
	char sbf[250];
}

main(argc,argv) int argc; char **argv;
{

	struct qdat q;
	register char *p, *p1;
	register int pse;
	char c;

    signal(1,1);
    signal(2,1);
    if (fork()==0) {
	if (argc > 1) {
		++argv;
		p = *argv;
		pse = 0;
		while(((c = *p++) >= '0') && (c <= '9')) {
			pse =* 10;
			pse =+ c - '0';
		}
	}
	else
		pse = 180;
	if (pse < 15)
		pse = 15;
	buf[0] = buf[1] = ':';
	i = getpid();
	j = dup(1); k = creat(PFILE,0644); close(1); dup(k);
	printf("%d\n",i);
	close(1); dup(j); close(j); close(k);
	chdir(QUEDIR);
	signal(14,cont);
	for(;;) {
		if ((ds = open(QUELOG,2)) < 0) {
			alarm(300);
			pause();
			continue;
		}
		else break;
	}
	for(;;) {
		while(read(ds,&buf[2],QLEN) > 0) {
			if (buf[2] != '/') continue;
			p = &buf[2];
			p1 = q.shf = q.sbf;
			while ((*p1++ = *p++) != ':');
			p1[-1] = '\0';
			sh = p1; q.fnm = p1;
			while(*--sh != '/');
			p2 = sh;
			sh++;
			while(*p1++ = *sh++);
			q.dir = p1;
			sh = q.shf;
			do
				*p1++ = *sh;
			while(sh++ != p2);
			*p1++ = '\0';
			q.qfl = p1;
			while ((*p1++ = *p++) != ':');
			p1[-1] = '\0';
			q.jnm = p1;
			while((*p1++ = *p++) != ':');
			if (*q.jnm == ':')
				p1++;
			p1[-1] = '\0';
			q.pnm = p1;
			while((*p1++ = *p++) != ':');
			if (*q.pnm == ':')
				p1++;
			p1[-1] = '\0';
			q.jnb = p1;
			while((*p1++ = *p++) != ':');
			if (*q.jnb == ':')
				p1++;
			p1[-1] = '\0';
			q.unm = p1;
			while((*p1++ = *p++) != ':');
			if (*q.unm == ':')
				p1++;
			p1[-1] = '\0';
			buf[QLEN-1] = '\n';
			close(0);
			if (open(q.qfl,0) != 0) {
				seek(ds,-QLEN,1);
				write(ds,buf,QLEN);
				continue;
			}
			unlink(q.qfl);
			if (fork() == 0) {
				if (stat(q.shf,sb) < 0)
					exit(1);
				close(1);
				if (open("/dev/null",1) != 1)
					exit(1);
				close(2);
				dup(1);
				ddv = sb[0];
				siz = (077600&(sb[4]>>1))+(0177&(sb[5]>>9));
				if (fsfree(ddv,sb) >= 0)
					if (sb[0] <(siz+1200) || sb[1]<50)
						fspce[0] = '1';
					else
						fspce[0] = '0';
				setuid(USR);
				chdir(q.dir);
			  execl(q.shf,q.fnm,fspce,q.jnm,q.pnm,q.jnb,q.unm,0);
			  execl("/bin/sh","sh",q.shf,fspce,q.jnm,q.pnm,q.jnb,q.unm,0);
				exit(1);
			}
			seek(ds,-QLEN,1);
			write(ds,buf,QLEN);
			alarm(pse);
			while(wait(0) > 0);
			tout = alarm(0);
			if (tout > 10) {
				alarm(tout);
				pause();
			}
		}
		alarm(pse);
		pause();
	}
    }
}
cont() {
	signal(14,cont);
}
