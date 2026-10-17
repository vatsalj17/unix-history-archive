# include "stdio.h"

/* this program is the source for /usr/lib/spell2
/* it is recreated by
	cc spell2.c -lS -o spell2
/*	usage is 
		spell2 -dxxxxxx
	where xxxxx is the location of the dictionary...
*/

int debug 0;
char dword[100], sword[100], inword[100];
char insuff[50];
char innumb[10];
char inlev[10];

FILE *sfile NULL;
FILE *dfile NULL;

main( argc, argv ) int argc; char *argv[];{

	register is, id, smore;

	if( argc>1 ) dfile = fopen( argv[1], "r" );
	if( argc>2 ) sfile = fopen(argv[2], "r");

	if( dfile==NULL ){
		fprintf( stderr, "no dictionary\n");
		exit(1);
		}
	smore = (sfile!=NULL);
	for(;;) {
		if(scanf("%s%s%s%s",inword,insuff,innumb,inlev) < 4) {
			fclose(stdout);
			return;
		}
		while((id=comp(inword,dword)) > 0)
			if(getword(dword,dfile) == 0) {
				fclose(stdout);
				return;
			}
		while(smore && (is=comp(inword,sword))>0)
			smore = getword(sword,sfile);
		if(is==0 && smore)
			wword(1);
		else if(id==0)
			wword(0);
	}
}

wword(badflag){
	printf("%s\t%s\t%s\t%d\n", innumb, inlev, insuff, badflag );
	}

comp(s,t)
register char *s, *t;
{
	if(debug) fprintf(stderr, "%s %s\n", s, t);
	if((*t&040) == 0)
		--s,--t;
	else if((*s|040)!=*t)
		return((*s|040) - *t);
	while(*++s == *++t)
		if(*s == 0)
			return(0);
	while((*s|040) == (*t|040)) {
		if(*s == 0)
			return(-1);	/*desperation*/
		s++; t++;
	}
	return((*s|040) - (*t|040));
}

getword(s,f) char *s; FILE *f;{
	while((*s=getc(f)) != '\n')
		if(*s++==-1) return(0);
	*s = 0;
	return(1);
}
