#define SIGINT 2
#define SIGQIT 3
#define EOR '\n'
#define X 0
/*
tk.c
*/

#define OBSZ 512
#define MAXY 3071
#define LINE 47
#define XOFF 248
#define US 037
#define GS 035
#define ESC 033
#define CR 015
#define FF 014
#define SO 016
#define SI 017

int pl 66*LINE;
int yyll -1;
int mpy 1;
int div 1;
char *ap;
int ch;
int nonumb;
int dfact 1;
int ibuf[259];
char obuf[OBSZ];
char *obufp obuf;
int esc;
int escd;
int verd;
int esct;
int xx XOFF;
int yy MAXY;
int ohy -1;
int ohx -1;
int oxb -1;
int oly -1;
int olx -1;
int tflag;
int ksize ';';
int od 1;
int first 1;
int alpha;
int somode;
int ry;
int xxx;
int sigint;
int sigqit;

main(argc,argv)
int argc;
char **argv;
{
	register i, j;
	extern ex();

	while(--argc > 0 && (++argv)[0][0]=='-')
		switch(argv[0][1]){
			case 'p':
				ap = &argv[0][2];
				if(i = atoi())
					pl = i;
					yyll = MAXY + 1 - pl;
				break;
			case 't':
				tflag++;
				break;
			default:
				ap = &argv[0][1];
				if(i = atoi())mpy = i;
				ch = 0;
				if(i = atoi())div = i;
				break;
		}
	if(argc){
		if(fopen(argv[0], ibuf) < 0){
			prstr("Cannot open: ");
			prstr(argv[0]);
			prstr("\n");
			exit(1);
		}
	}
	sigint=signal(2, ex);
	sigqit=signal(SIGQIT,1);
	init();
	while((i = getc(ibuf)) >= 0){
		switch(i){
			case '\014': /*form feed*/
				goto x0;
			case '\n':
				xx = XOFF;
				yy =- LINE;
				alpha = 0;
				if(yy < yyll){
			x0:
					yy = 0;
					sendpt(0);
					oput(US);
					flusho();
					kwait();
					init();
				}
				continue;
			case CR:
				xx = XOFF;
				alpha = 0;
				continue;
			case ' ': /*space*/
				xx =+ 31;
				alpha = 0;
				continue;
			case '\t': /*tabstops at 8*31=248*/
				j = ((xx-XOFF)/248) + 1;
				xx =+ j*248 - (xx-XOFF);
				alpha = 0;
				continue;
			case '\b':
				xx =- 31;
				alpha = 0;
				continue;
			case ESC:
				switch(i = getc(ibuf)){
					case '\014':
						goto x0;
					case '7':
						yy =+ LINE;
						alpha = 0;
						continue;
					case '8':
						yy =+ (LINE + ry)/2;
						ry = (LINE + ry)%2;
						alpha = 0;
						continue;
					case '9':
						yy =- (LINE - ry)/2;
						ry = -(LINE - ry)%2;
						alpha = 0;
						continue;
					default:
						continue;
				}
			case SO:
				somode++;
				continue;
			case SI:
				somode = 0;
				continue;
			default:
				j = 0;
				if(somode){
					switch(i){
					case 'C': j='W';break;/*omega*/
					case 'S': j='E';break;/*epsilon*/
					case 'K': j='R';break;/*rho*/
					case 'A': j='A';break;/*alpha*/
					default: j = ' ';
					}
					if(j){
						sendpt(alpha);
						oput(US);
						oput(ESC); oput(SO);
						oput(j);
						oput(ESC); oput(SI);
						alpha++;
						xx =+ 31;
					}
				continue;
				}
				sendpt(alpha);
				oput(US);
				oput(i);
				if(i>040)xx =+ 31;
				alpha++;
				continue;
		}
	}
	xx = XOFF;
	yy = 0;
	sendpt(0);
	oput(US);
	flusho();
	kwait();
	ex();
}
init(){
	register i;

	ohx = oxb = olx = ohy = oly = -1;
	xx = XOFF;
	yy = MAXY;
	oput(ESC); oput(FF); oput(ESC); oput(ksize);
/*	for(i = 1; i > 0; i--)oput(GS); /*delay about .001 second*/
	sendpt(0);
}
ex(){
	yy = MAXY;
	xx = 0;
	sendpt(0);
	oput(ESC);
	oput(';');
	oput(US);
	flusho();
	exit(0);
}
kwait(){
	char buf[128]; char *bptr; char c;
next:
	bptr=buf;
	while((c=readch())&&(c!='\n')) *bptr++=c;
	*bptr=0;
	if(bptr!=buf){
		bptr = buf;
		if(*bptr == '!'){unix(&buf[1]); prstr("!\n"); goto next;}
		else switch(*bptr++){
			default:
				prstr("?\n");
				goto next;
		}
	}
	else if (c==0) ex();
	else	return;
}
unix(line)
char line[];
{
	int rc, status, unixpid;
	if( (unixpid=fork())==0 ) {
		signal(SIGINT,sigint); signal(SIGQIT,sigqit);
		close(0); dup(2);
		execl("/bin/sh", "-sh", "-c", line, 0);
		exit(255);
	}
	else if(unixpid == -1)
		return(0);
	else{	signal(SIGINT,1); signal(SIGQIT,1);
		while( (rc = wait(&status)) != unixpid && rc != -1 ) ;
		signal(SIGINT,ex); signal(SIGQIT,sigqit);
	}
}
readch(){
	char c;
	if (read(2,&c,1)<1) c=0;
	return(c);
}
oput(i)
char i;
{
	*obufp++ = i;
	if(obufp >= (obuf + OBSZ))flusho();
}
flusho(){
	write(od,obuf,obufp-obuf);
	obufp = obuf;
}
sendpt(a)int a;{
	register zz;
	int hy,xb,ly,hx,lx;

	if((zz = yy) < 0)zz = 0;
	hy = ((zz>>7) & 037);
	xb = ((xx & 03) + ((zz<<2) & 014) & 017);
	ly = ((zz>>2) & 037);
	hx = ((xx>>7) & 037);
	lx = ((xx>>2) & 037);
	if(!a){
	oput(GS);
	if(hy != ohy)oput(hy | 040);
	if(xb != oxb)oput(xb | 0140);
	if((ly != oly) || (hx != ohx) || (xb != oxb))
		oput(ly | 0140);
	if(hx != ohx)oput(hx | 040);
	oput(lx | 0100);
	ohy = hy;
	oxb = xb;
	oly = ly;
	ohx = hx;
	olx = lx;
	alpha = 0;
	}
	return;
}
prstr(s)
char *s;
{
	register i;

	for(i=0;*s;i++)s++;
	write(2,s-i,i);
}
atoi()
{
	register i, j, acc;
	int field, digits, aa[2];

	field = digits = acc = 0;
a1:
	while(((j = (i = getch()) - '0') >= 0) && (j <= 9)){
		field++;
		digits++;
		acc = 10*acc + j;
	}
	if(i == '.'){
		field++;
		digits = 0;
		goto a1;
	}
	if(!(ch = i))ch = 'x';
	switch(i = getch()){
		case 'i': /*inches*/
			j = 6 * LINE;
			break;
		case 'l': /*lines*/
			j = LINE;
			break;
		default:
			j = LINE;
			ch = i;
	}
	aa[0] = acc * j;
	aa[1] = hmul(acc,j);
	acc = aa[0];
	if((field != digits) && (digits > 0)){
		j = 1;
		while(digits--)j =* 10;
		acc = ldiv(aa[1],aa[0],j);
	}
	nonumb = !field;
	return(acc);
}
getch(){
	register i;

	if(ch){
		i = ch;
		ch = 0;
		return(i);
	}
	return(*ap++);
}
