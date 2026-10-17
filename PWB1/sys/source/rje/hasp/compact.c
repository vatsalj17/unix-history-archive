#define SIZEQ 200
#define QUELOG "/usr/rje/sque/log"
#define QUELOG2 "/usr/rje/sque/log2"

char buf[SIZEQ];
int i,j;
int stbuf[18];

main() {
	if ((i=open(QUELOG))<0)
		exit(1);
	seek(i,0,2);
	if (seek(i,-SIZEQ,1) < 0)
		exit(0);
	do {
		read(i,buf,2);
		if (buf[0] == ':' && buf[1] == ':') {
			seek(i,SIZEQ,1);
			break;
		}
	} while (seek(i,-(SIZEQ + 2),1) >= 0);
	seek(i,-2,1);
	j = creat(QUELOG2,0644);
	while(read(i,buf,SIZEQ) == SIZEQ)
		write(j,buf,SIZEQ);
	close(j); close(i);
	stat(QUELOG,stbuf);
	unlink(QUELOG);
	link(QUELOG2,QUELOG);
	unlink(QUELOG2);
	chown(QUELOG,stbuf[4]<<8 | ((stbuf[3]>>8)&0377));
}
