#define NMOUNT 20
#define MTABSIZE 26

struct mtab {
	char	fsys[10];
	char	node[10];
	int	ro_flg;
	int	date[2];
}	mtab[NMOUNT];
main()
{

	register struct mtab *mp;
	int	rec,i,date[2], SETCOUNT;
	char	*cp,*cpfsys,inbuf[25];

	SETCOUNT = 0;
	if ((rec = creat("/etc/mnttab",0644)) < 0) {
		write(1,"creat error\n",12);
		exit(1);
	}
	time(date);
	while(1) {
		for(cp = inbuf;;cp++) {
			if (read(0,cp,1) != 1) {
				write(rec,&mtab, MTABSIZE * SETCOUNT);
				exit(1);
			}
			if (*cp == '\n') {
				break;
			}
		}
		SETCOUNT++;
		for(cp = inbuf; (*cp != ' ') && (*cp != '\n'); cp++);
		*cp = '\0';
		for(mp = mtab; mp <&mtab[NMOUNT]; mp++)  {
			if(mp->fsys[0] == 0) {
				for(i = 0; inbuf[i] != '\0'; i++) {
					mp->fsys[i] = inbuf[i];;
				}
				for(i = 0; *++cp != '\n'; i++) {
					mp->node[i] = *cp;
				}
				mp->date[0] = date[0];
				mp->date[1] = date[1];
				mp->ro_flg = 0;
				for(i = 0; i < 25; i++) {
					inbuf[i] = '\0';
				}
			}
		}
	}
}
