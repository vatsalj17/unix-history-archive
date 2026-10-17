#
/* mail command usage
	mail
		prints your mail
	mail people
		sends standard input to people
 */

#include	<stdio.h>
#define myname	"PWB"
#define	SIGINT	2

struct	passwd {
	char	*pw_name;
	char	*pw_passwd;
	int	pw_uid;
	char	*pw_gecos;
	char	*pw_dir;
	char	*pw_shell;
};

char	lettmp[] "/tmp/maXXXXX";
FILE	*pwfil;
FILE	*mfil;
long	msiz;
long	ftell(), prlet();
char	*cat(), *whoami();
extern	delexit();

static saved;

main(argc, argv)
char **argv;
{

	mktemp(lettmp);
	if(argc==1 || argv[1][0]=='-' && (
			argc==2 || argc==3 && argv[1][1]=='f')) {
		printmail(argc, argv);
		delexit();
	}
	if (signal(SIGINT,1) == 0)
		signal(SIGINT, delexit);
	if ((mfil = fopen(lettmp, "w")) == NULL) {
		fprintf(stderr, "Cannot create temporary\n");
		delexit();
	}
	bulkmail( argc, argv, whoami() );
	}

char *
whoami(){
	return(logname());
}

printmail(argc, argv)
char **argv;
{
	register c;
	register char *my_ldir;
	char mail[100], mbox[100];

	if( argc==3 ){
		cat( mail, argv[2], "" );
		cat( mbox, "", "" );
		if ((mfil=fopen(mail,"r")) == NULL) {
			fprintf(stderr, "Can't open %s\n", mail);
			return;
			}
		}
	else {
		cat( mail, ".mail", "" );
		cat( mbox, "mbox", "" );
		if( (mfil=fopen(mail, "r")) == NULL ){
			my_ldir = logdir();
			cat( mail, my_ldir, "/.mail" );
			cat( mbox, my_ldir, "/mbox" );
			mfil = fopen( mail, "r" );
			}
		}
	if (mfil!=NULL && getc(mfil)>=0) {
		fseek( mfil, 0L, 0 );
		prlet();
		c = 'y';
		if (argc<2) {
			if (ttyn(0)!='x') {
				fprintf(stderr, "Save?");
				c = getchar();
			}
		} else
			c = argv[1][1];
		if (c=='y') {
			if( *mbox == '\0' || !append( mbox ) ){
				fprintf( stderr, "cannot append; didn't destroy\n" );
				return;
				}
			}
		fseek(mfil,0L,2);
		if(ftell(mfil) > msiz) {
			fprintf( stderr, "More mail has arrived: nothing done\n" );
			return;
		}
		if(argc<=2)
			fclose(fopen(mail, "w"));
	} else
		fprintf(stderr, "No mail\n");
}

long
prlet()
{
	long beg, end;
	register flag;

	flag = 1;
	beg = ftell( mfil );
	for(;;) {
		switch(getc(mfil)) {
		case EOF:
			msiz = beg+1;
			return( msiz+1 );
		case '\n':
			if(flag==8) {
				end = prlet();
				goto brk1;
			}
			flag = 1;
			beg = ftell(mfil);
			continue;
		case 'F':
			if(flag!=1)
				goto dflt;
			break;
		case 'r':
			if(flag!=2)
				goto dflt;
			break;
		case 'o':
			if(flag!=3)
				goto dflt;
			break;
		case 'm':
			if(flag!=4)
				goto dflt;
			break;
		case ' ':
			if(flag!=5)
				goto dflt;
			break;
		case ':':
			if(flag>5)
				break;
		default:
		dflt:
			if(flag<=5)
				flag = 0;
			continue;
		}
		flag++;
	}

/*	plausible postmark found
 *	(line beginning with "From " and containig 2 :'s
*/
brk1:
	fseek(mfil, beg, 0);
	while( beg<end-- && (flag=getc(mfil))>=0 ) putchar(flag);
	return( beg );
}

bulkmail(argc, argv, from)
char **argv, *from;
{
	long tbuf;
	register c;
	char line[128];

	time(&tbuf);
	fprintf(mfil, "From %s %s", from, ctime(&tbuf));
	while (fgets(line, 128, stdin) != NULL) {
		if (line[0]=='.' && line[1]=='\n')
			break;
		fputs(line, mfil);
	}
	putc('\n', mfil);
	fclose(mfil);
	if( (mfil=fopen(lettmp, "r")) == NULL ){
		fprintf( stderr, "horrible mail problem: goodbye\n" );
		delexit();
		}
	while (--argc > 0)
		sendto(*++argv);
	delexit();
}

sendto(person)
char *person;
{
	register struct passwd *p;
	char target[100];

	setpw();
	while (p = getpwent()) {
		if (equal(p->pw_name, person)) {
			if (append( cat( target, p->pw_dir, "/.mail") )==0)
				break;
			return;
		}
	}
	fprintf(stderr, "Can't send to %s\n", person);
	deadsv();
}


deadsv()
{
	if (ttyn(0)!='x' && saved==0) {
		unlink("dead.letter");
		saved++;
		fprintf(stderr, "Letter saved in 'dead.letter'\n");
		append("dead.letter");

		}
}
append(to)
char *to;
{
	register FILE *pf;
	register c;

	if ((pf = fopen(to, "a")) == NULL)
		return(0);
	fseek( mfil, 0L, 0 );
	while ((c = getc(mfil)) >= 0)
		putc(c, pf);
	fclose(pf);
	return(1);
}

setpw()
{
	if (pwfil == NULL)
		pwfil = fopen("/etc/passwd", "r");
	rewind(pwfil);
}

getpwent()
{
	register char *p;
	register c;
	static struct passwd passwd;
	static char line[200];

	if (pwfil==NULL)
		return(0);
	p = line;
	while((c=getc(pwfil)) != '\n') {
		if(c <= 0)
			return(0);
		if(p < line+198)
			*p++ = c;
	}
	*p = 0;
	p = line;
	passwd.pw_name = p;
	p = pwskip(p);
	passwd.pw_passwd = p;
	p = pwskip(p);
	passwd.pw_uid = atoi(p);
	p = pwskip(p);
	passwd.pw_gecos = p;
	p = pwskip(p);
	passwd.pw_dir = p;
	p = pwskip(p);
	passwd.pw_shell = p;
	return(&passwd);
}

pwskip(ap)
char *ap;
{
	register char *p;

	p = ap;
	while(*p != ':') {
		if(*p == 0)
			return(p);
		p++;
	}
	*p++ = 0;
	return(p);
}

delexit()
{
	unlink(lettmp);
	exit(0);
}

equal(as1, as2)
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ == *s2)
		if (*s2++ == 0)
			return(1);
	return(0);
}

char *
cat( target, ap1, ap2)
char *target, *ap1, *ap2;
{
	register char *p1, *p2;
	p1 = ap1;
	p2 = target;
	while (*p2++ = *p1++);
	p2--;
	p1 = ap2;
	while (*p2++ = *p1++);
	return(target);
}
