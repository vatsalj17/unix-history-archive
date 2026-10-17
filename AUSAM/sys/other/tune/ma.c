/*
 *	ma [a] [s] [u] [c] [r]		default is s
 *
 *	print a histogram of malloc usage for both swap and core.
 *
 *	'a' option:	print information for all entries.
 *			default is off, i.e. only non-zero entries printed.
 *	's' option:	turns off histogram. prints only statistical summary.
 *			default is on.
 *	'u' option:	read previosly obtained values from
 *			"malloc.total". add these to values read from core
 *			write the latest obtained values to the file
 *			"malloc.total". always create file first.
 *			if update successful clear kernal core.
 *			default is no update
 *	'r' option:	combine prior stats with newly obtained values
 *			default is read
 *	'c' option:	update every 'NSEC' seconds
 *			no output given
 *			default is off but it implies update
 *
 */
#define MAXS	100l	/* max stars per line */
#define	NSEC	600	/* seconds between updates */
error(s1,s2)
char *s1,*s2;
{
	printf(s1,s2);
	flush();
	exit(-1);
}

double ldoub(i)
long i;
{
	return(i);
}
double doub(i)
unsigned i;
{
	return(i);
}

/************************/
unsigned mcore[2064];
unsigned mswap[258];
unsigned mdkmiss;
unsigned mcomiss;
unsigned mrdlen { sizeof mcore + sizeof mswap + sizeof mdkmiss + sizeof mcomiss };
/************************/
long dcore[2064];
long dswap[258];
long dkmiss;
long comiss;
unsigned rdlen { sizeof dcore + sizeof dswap + sizeof dkmiss + sizeof comiss };
/************************/
unsigned zero [ sizeof mcore + sizeof mswap + sizeof mdkmiss + sizeof mcomiss ];
int sflag 1;
int aflag 0;
int uflag 0;
int rflag 1;
int cflag 0;
char *mt "/etc/malloc.total";

struct {
	char name[8];
	int type;
	char *value;
	} nl[5] {
		"_mcorec",0,0,
		"_mdkrec",0,0,		/* assumed contigous with above */
		"_mcomiss",0,0,		/* assumed contigous with above */
		"_mdkmiss",0,0,		/* assumed contigous with above */
		"",0,0
	};

main(argc,argv)
char **argv;
int argc;
{
	register mem,mtfd,i;
	extern fout;
	fout = dup(1);

/*
 *	get options
 */
	for(argv++; --argc; argv++)
	    if(**argv == 'a') aflag++;
	    else if(**argv == 's') sflag=0;
		 else if(**argv == 'u') uflag++;
		      else if(**argv == 'c') cflag++,uflag++;
			   else if(**argv == 'r') rflag=0;

	nlist("/unix",nl);
	if( (nl[0].type==0) || (nl[1].type==0) || (nl[2].type==0) || (nl[3].type==0) ) error("namelist error\n");
	mem = open("/dev/kmem", ( uflag ? 2 : 0 ) );
	if( mem<0 ) error("can't open /dev/kmem\n");

	if( uflag || rflag ) {
		if( (mtfd=open(mt,0)) < 0 )
			error(" can't open %s\n",mt);
		if( read(mtfd,dcore,rdlen)!=rdlen)
			error(" read error %s\n",mt);
		close(mtfd);
	}

loop:
	disable();
	lseek( mem, 0, nl[0].value, 0);
	if( read( mem, mcore, mrdlen)!=mrdlen ) error("core read error\n");

	for(i=0;i<2064;i++) dcore[i] =+ mcore[i];
	for(i=0;i< 258;i++) dswap[i] =+ mswap[i];
	dkmiss =+ mdkmiss;
	comiss =+ mcomiss;
	if( uflag ){
		if( (mtfd=creat(mt,0600)) < 0 )
			error(" can't create %s\n",mt);
		if( write(mtfd,dcore,rdlen)!=rdlen)
			error(" write error %s\n",mt);
		close(mtfd);
		if( !cflag ) {
			printf(" malloc.total updated\n");
			flush();
		}

		lseek( mem, 0, nl[0].value, 0);
		if( write( mem, zero, mrdlen)!=mrdlen ) error("core write error\n");
		if( !cflag ) {
			printf("  kernal core cleared\n");
			flush();
		}
	}

	enable();
	if( cflag ) {
		sleep(NSEC);
		goto loop;
	}
	prit(dcore,2064,"core",16,comiss);

	prit(dswap,258,"swap",2,dkmiss);

}

prit(aray , nelt, s, nk, misses)
long aray[];
int nelt,nk;
char *s;
long misses;
{
	register i,j,k;	long scale;
	double num ; double sum ; double nkb;
	double dev;
	extern double sqrt();

	dev = sum = num = 0.0;
	nkb = nk;

	for(i= -1;aray[++i]==0;);
	for(j=nelt;aray[--j]==0;);

	scale = 0;
	for(k=i;k<=j;k++) {
			if(aray[k] > scale ) scale = aray[k];
			sum = sum + ldoub(aray[k]) * doub(k);
			num = num + ldoub(aray[k]);
			dev = dev + ldoub(aray[k]) * doub(k) * doub(k);
	}
	if ( num==0 ) {
		printf("\014\n	%s  *** no allocations  ***	\n",s);
		flush();
		return;
	}
	printf("\014\n	%s	\n",s);

	if( !sflag)
	for(;i<=j;i++){
		if((aray[i] == 0) && (!aflag)) continue;
		printf("%4d  %6.3fk   %5ld | ",i,i/nkb,aray[i]);
		k = aray[i];
		k = (k*MAXS)/scale;
		while((k--)>0) putchar('*');
		putchar('\n');
	}
	printf("\n      mean = %5.1f  %6.1fk  sample = %5.0f \n",sum/num,(sum/num)/nkb,num);
	dev = sqrt( (dev - sum*sum/num)/num );
	printf("\n      standard deviation = %5.1f  %6.1fk \n\n",
		dev,(dev)/nkb);
	if( misses )
	  printf("\tmisses = %ld	%5.1f%%\n",misses,misses*100./num);
	flush();
}
int s1,s2,s3;
disable()
{
	s1 = signal(1,1); s2 = signal(2,1); s3 = signal(3,1);
}
enable()
{
	signal(1,s1); signal(2,s2); signal(3,s3);
}
