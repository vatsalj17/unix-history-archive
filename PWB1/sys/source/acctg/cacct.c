#define WTMP "/usr/actg/data/wtmp"
#define FEE "/usr/actg/data/fee"
#define LOG "/usr/actg/data/log"
#define LOGSIZE 512
#define UACCT "/usr/actg/data/uacct"
#define USIZE 256
#define UELTSIZE 44
#define SSIZE 10 /* size of system table */
#define SYST '?'
/*
 *
 * MODIFICATION OF acct.c SOURCE
 * 
 */

#define	TSIZE	80

/*
 * entry in wtmp file
 *	connect time is in seconds from login
 *	later converted to minutes (float)
 *
 */
struct wtmprec {
	char	name[8];
	char	tty;
	char	fill1;
	float	time;
	int	fill2;
} ibuf;
/*
 *
 *  structure of UACCT file
 *
 */
struct uacct {
	float cutm;
	float cstm;
	float crtm;
	float cctm[2];
	int dc;
	float du;
	int msamp;
	float msize;
	float mproc;
	int fee;
	int fill;
	float spare;
	}  uacct[USIZE];


struct utab {
	char name[8];
	float conn[2];
	} utab[USIZE];


struct systab {
	char system[8];
	int boots;
	} systab[SSIZE]; /* info by system */




/*
 * information by line
 * 	ttime  --  last time line on/off
 *	userp -- pointer to entry in utab of user on the line
 */
struct tbuf {
	struct	utab	*userp;
	float	ttime;
} tbuf[TSIZE];

int holidays[8] { /* day of year (less 1) */
	1,	/* New Year's */
	51,	/* Washington's Birthday (76) */
	149,	/* Memorial Day */
	184,	/* Indep. Day */
	247,	/* Labor Day */
	327,	/* Thanksgiving */
	328,	/* day after  */
	359	/* Christmas */
	};

char	*wtmp	WTMP;
char	*syst	SYST;
int	wtmpfd;
int	pflag;
double	midnight;
double  dtime;
double	lastime;
double	day	1440.;/* minutes per day */
int	pcount;
char	**pptr;
double bmins,emins,atof();
char *now "";
int reboots;
int	line, page;
int	log[USIZE];
int ndate;
int	*thistime, *localtime();
int ffee[USIZE], zeros[256];


main(argc, argv) 
char **argv;
{
	int i, c, fl;
	char *ip;
	int f;

/*  get begin/end times for prime shift in minutes */
bmins = 9*60.0 + 0.0;
emins = 16*60.0 + 59.0;


	while (--argc > 0) if (**++argv == '-') switch(*++*argv) {


	case 'w':
		if (--argc>0)
			wtmp = *++argv;
		continue;

	case 'p':
		pflag++;
		pcount = --argc;
		argc = 0;
		pptr = ++argv;
		continue;
	default:
		syst = **argv;
		continue;
	}
	if ( (wtmpfd=open(wtmp,0)) < 0 ) {
		printf("No %s\n", wtmp);
		return;
	}
	if ((i=open(FEE,0))<0) {
		printf("can't open fee\n");
		exit(9);
	}
	if (read(i,ffee,USIZE*2) != USIZE*2) {
		printf("can't read fee\n");
		exit(9);
	}
	close(i);
	/*
	 * open the UACCT file
	 */
	if ( (i=open(UACCT,0)) < 0)
		{printf("can't open %s\n",UACCT);
		exit(9);
		}
	if ( read(i,uacct,USIZE*UELTSIZE) != USIZE*UELTSIZE)
		{printf("error reading %s\n",UACCT);
		 exit(9);
		}
	if ((i=open(LOG,0))<0) printf("can't open date log\n");
	else if (read(i,log,LOGSIZE) != LOGSIZE) 
		printf("can't read date log\n");
	for(;;) {
		if( read(wtmpfd,&ibuf,16) != 16) goto brk;
		fl = 0;
		for (i=0; i<8; i++) {
			c = ibuf.name[i];
			if ('0'<=c&&c<='9'||'a'<=c&&c<='z'||'A'<=c&&c<='Z') {
				if (fl)
					goto skip;
				continue;
			}
			if (c==' ' || c=='\0') 
				{
				fl++;
				ibuf.name[i] = '\0';
				}
			else  goto skip;
		}
		loop();
    skip:;
	}
    brk:
	ibuf.name[0] = '\0';
	ibuf.tty = 'x';
	time(&ibuf.time);
	loop();
	reboots--; /* EOF -- not really a reboot */
	output();
}

loop()
{
	double ltod();
	int i;
	struct tbuf *tp;
	struct utab *up;

	if(ibuf.fill2) return;
	if( ibuf.fill1 && (ibuf.fill1 != 'h') )
		return;
	thistime = localtime(&ibuf.time);
	if (*now == '\0') hdg(); /* first time, print heading */
/*  change time()-produced time to float minutes */
	ibuf.time = ltod(&ibuf.time)/60.0;
	if(ibuf.tty == '|') {
		dtime = ibuf.time;
		return;
	}
	if(ibuf.tty == '}') {
		if(dtime == 0.)
			return;
		for(tp = tbuf; tp < &tbuf[TSIZE]; tp++)
			tp->ttime =+ ibuf.time-dtime;
		dtime = 0.;
		return;
	}
	if (lastime>ibuf.time || lastime+(1.5*day)<ibuf.time)
		midnight = 0.0;
	if (midnight==0.0)
		newday();
	lastime = ibuf.time;
	/*
	 * if a reboot, update all lines not yet hung up
	 */
	if (ibuf.tty=='x') {
		reboot();
		ibuf.name[0] = '\0';
		upall();
		return;
	}
	/*
	 * get an index # from the tty letter
	 */
	i = ibuf.tty;
	i =- '0';
	if (i<0 || i>=TSIZE)
		i = TSIZE-1;
	tp = &tbuf[i];
	update(tp);
}

/*
 * proceed through utab
 *	a)  calculate today's total connect minutes
 *	b)  update user connect time in UACCT
 *	c)  if -p, print connect time by user
 */
output()
{
	int i,j, idx;
	float ttime, t, tptime, tnptime;

	if (pflag)
	{
	printf("\n\tuser    \t  prime hrs      non-prime hrs\n\n");
	line =+ 3;
	}
	ttime = tptime = tnptime = 0.0;
	for (i=0; i<USIZE; i++) {
		t = utab[i].conn[0] + utab[i].conn[1];
		if (ffee[i] > 0) uacct[i].fee =+ ffee[i];
		if (t>0.0)
		{
		ttime = ttime + t;
		utab[i].name[7] = '\0';
		if((idx = cvtlid(utab[i].name))<0) continue;
		uacct[idx].cctm[0] = uacct[idx].cctm[0] + utab[i].conn[0];
		uacct[idx].cctm[1] = uacct[idx].cctm[1] + utab[i].conn[1];
		log[idx] = ndate;
		tptime = tptime + utab[i].conn[0];
		tnptime = tnptime + utab[i].conn[1];
		}
		if (among(i) && t > 0.0) {
			/* print connect hours
			   leave minutes in utab */
			printf("\t%-9s\t%9.2f\t%9.2f\n",
			     utab[i].name,
			     utab[i].conn[0]/60.0,
			     utab[i].conn[1]/60.0);
			if (++line > 56) /* do the page eject */
			{while (line < 66)
				{putchar('\n');
				 line++;
				}
			printf("\n\n\t\t\tPage%3d\n\n",++page);
			line = 5;
			}
		}
	}
	if (ttime > 0.0) {
		/* print connect time in hours */
		printf("\n\ttotal    \t%9.2f \t%9.2f \t%9.2f\n",
		tptime/60.0,tnptime/60.0,ttime/60.0);
		line =+ 2;
	}
	printf("\n\tnumber of reboots: \t%2d\n",reboots);
	line =+ 2;
	/* dump out system table */
	for (i=0;i<SSIZE;i++)
	{if (systab[i].system[0] == '\0') break;
	 printf("\t\t%-8s\t%2d\n",
		systab[i].system,
		systab[i].boots);
	 line++;
	}
	if ((i=creat(LOG,0644))<0) printf("can't creat date log\n");
	else { write(i,log,LOGSIZE); close(i);
	     }
	while(line < 66)
		{putchar('\n');
		 line++;
		}
	/*
	 * write out updated UACCT info
	 */
	if (  (i=creat(UACCT,0644)) < 0)
		{printf("can't creat %s\n",UACCT);
		 exit(9);
		}
	if ((j=creat(FEE,0644))<0) {
		printf("can't creat fee\n");
		exit(9);
	}
	write(j,zeros,512);
	close(j);
	write(i,uacct,USIZE*UELTSIZE);
	close(i);
}

upall()
{
/*
 * logoff all tty's in use
 */
	struct tbuf *tp;

	for (tp=tbuf; tp < &tbuf[TSIZE]; tp++)
		update(tp);
}

update(tp)
struct tbuf *tp;
{
	int i, j;
	struct utab *up;
	double t, t1;

	t = ibuf.time;
	/*
	 *  if a user at this tty, then he just logged off
	*	update his connect time
	*/
	if (tp->userp) 
		{
		t1 = t - tp->ttime;
		if (t1>0.0 && t1 < 1.5*day)
			{
			connect(tp->userp,t1);
			}
		}
	tp->ttime = t;
	/*
	 * if an actual logoff (name is \0)
	 *	then remove entry from tty table
	 * else set up tty table to point to the new user of this line
	 */
	if (ibuf.name[0]=='\0') {
		tp->userp = 0;
		return;
	}
	for (up=utab; up < &utab[USIZE]; up++) {
		if (up->name[0] == '\0')
			break;
		for (j=0; j<8 && up->name[j]==ibuf.name[j]; j++);
		if (j>=8)
			break;
	}
	for (j=0; j<8; j++)
		up->name[j] = ibuf.name[j];
	tp->userp = up;
}

among(i)
{
	int j, k;
	char *p;

	if (pflag==0)
		return(0);
	if (pcount==0)
		return(1);
	for (j=0; j<pcount; j++) {
		p = pptr[j];
		for (k=0; k<8; k++) {
			if (*p == utab[i].name[k]) {
				if (*p++ == '\0')
					return(1);
			} else
				break;
		}
	}
	return(0);
}

newday()
{
	while (midnight <= ibuf.time)
		midnight = midnight + day;
}

/*
 * determine prime/non-prime connect minutes
 */
connect(up,cmins)
struct utab *up;
double cmins;
{
double tmins,smins,p,np;
/*
 * if this is Saturday or Sunday,
 *	then connect time is non-prime
 */
if( SSH(thistime) )
	{up->conn[1] = up->conn[1] + cmins;
	 return;
	}
/* compute this logoff time in minutes */
tmins = thistime[2]*60.0 + thistime[1];
while(tmins<cmins) tmins = tmins + day; /* connected over 24 hrs*/
/*
 * compute prime/non-prime mins
 */
p=np=0.0;
while(cmins >0)
	
	{
	/* start time of to-be-charged connect time */
	smins = tmins - cmins;
	if(smins<bmins)
		{np = (tmins<bmins?tmins:bmins)-smins;
		 cmins = cmins - np;
		 continue;
		}
	if(smins<emins)
		{p=(tmins<emins?tmins:emins)-smins;
		 cmins = cmins - p;
		 continue;
		}
	{np = np + cmins;
	 cmins = cmins - np;
	}
	}
/* update information in user table */
	up->conn[0] = up->conn[0] + p;
	up->conn[1] = up->conn[1] + np;
}
/*
 * print a heading
 */
hdg()
{int tvec[2];
char save[26];
struct wtmprec lastrec;
page = 1;
strcpy(save,ctime(&ibuf.time));
save[24]= '\0';
time(tvec);
strcpy(now,ctime(tvec));
ndate = (localtime(tvec))[7];
now[24] = '\0';
/*  read the last record */
seek(wtmpfd,-16,2);
read(wtmpfd,&lastrec,16);
seek(wtmpfd,16,0);  /* reset read pointer */
printf("\n\nConnect Time / System %c Date: %s\tPage%3d\n\n%s to %s\n",
	syst,now,page,save,ctime(&lastrec.time));
line = 6;
}
/*
 *
 * get info about loading of new system
 *
 */
reboot()
{
struct systab *sp;
int j;
/*
 * enter system into systab or incr its boot count
 *	if it's there
 */
reboots++; /* incr total reboots */
/* check for not system name in a reboot record */
if (ibuf.name[0] == '\0') return;
for (sp=systab;sp < &systab[SSIZE];sp++)
	{
	 if (sp->system[0] == '\0') break;
	 for(j=0;j<8 && sp->system[j]==ibuf.name[j];j++);
	 if (j>=8) break;
	}
/* put system and count into entry */
for (j=0;j<8;j++)
	sp->system[j] = ibuf.name[j];
sp->boots++;
}

/*  determine if Saturday, Sunday, or Holiday
 *  if so, return 1, else return 0
 */
SSH(ltp) 
int *ltp;
{
int i;
if( ltp[6] == 0 || ltp[6] == 6 ) return(1);
for(i=0;i<8;i++)
	if( ltp[7] == holidays[i] ) return(1);
return(0);
}
