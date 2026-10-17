# define NPAGES 1
# define SPACE 4
# define W "  "

char *sep "**********************************************";
char *fill "\n\n\n\n\n\n\n\n\n\n";

char *C0[10] {
	"  *****  ",
	" ******* ",
	"**     **",
	"**     **",
	"**     **",
	"**     **",
	"**     **",
	"**     **",
	" ******* ",
	"  *****  "
};

char *C1[10] {
	"     *   ",
	"    **   ",
	"   ***   ",
	"    **   ",
	"    **   ",
	"    **   ",
	"    **   ",
	"    **   ",
	"  ****** ",
	"  ****** "
};

char *C2[10] {
	" ******* ",
	"*********",
	"**     **",
	"      ** ",
	"     **  ",
	"    **   ",
	"   **    ",
	"  **     ",
	" ********",
	"*********"
};

char *C3[10] {
	"*********",
	"*********",
	"**    ** ",
	"     **  ",
	"    **** ",
	"   ******",
	"       **",
	"**     **",
	"*********",
	" ******* "
};

char *C4[10] {
	"      *  ",
	"     **  ",
	"    ***  ",
	"   ****  ",
	"  ** **  ",
	" **  **  ",
	"*********",
	"*********",
	"     **  ",
	"     **  "
};

char *C5[10] {
	"*********",
	"*********",
	"**       ",
	"******** ",
	" ********",
	"       **",
	"       **",
	"**     **",
	"*********",
	" ******* "
};

char *C6[10] {
	" ******* ",
	"*********",
	"**     **",
	"**       ",
	"******** ",
	"*********",
	"**     **",
	"**     **",
	"*********",
	" ******* "
};

char *C7[10] {
	"*********",
	"*********",
	"**    ** ",
	"     **  ",
	"    **   ",
	"   **    ",
	"  **     ",
	"  **     ",
	"  **     ",
	"  **     "
};

char *C8[10] {
	" ******* ",
	"*********",
	"**     **",
	"**     **",
	" ******* ",
	" ******* ",
	"**     **",
	"**     **",
	"*********",
	" ******* "
};

char *C9[10] {
	" ******* ",
	"*********",
	"**     **",
	"**     **",
	"*********",
	" ********",
	"       **",
	"**     **",
	"*********",
	" ******* "
};

char *Cb[10] {
	"******** ",
	"*********",
	"**     **",
	"**     **",
	"******** ",
	"******** ",
	"**     **",
	"**     **",
	"*********",
	"******** "
};

char *Ci[10] {
	" ******  ",
	" ******  ",
	"   **    ",
	"   **    ",
	"   **    ",
	"   **    ",
	"   **    ",
	"   **    ",
	" ******  ",
	" ******  "
};

char *Cn[10] {
	"*      **",
	"**     **",
	"***    **",
	"****   **",
	"** **  **",
	"**  ** **",
	"**   ****",
	"**    ***",
	"**     **",
	"**      *"
};

char *Cr[10] {
	"******** ",
	"*********",
	"**     **",
	"**     **",
	"*********",
	"******** ",
	"**  **   ",
	"**   **  ",
	"**    ** ",
	"**     **"
};

char *blank[10] {
	"         ",
	"         ",
	"         ",
	"         ",
	"         ",
	"         ",
	"         ",
	"         ",
	"         ",
	"         "
};


main(argc,argv)
char *argv[];
{
	register char *p;
	register int i, j;
	char **line[9];
	int tv[2];
	char *tim;
	char *person;
	char *message;
	extern int fout;

	fout = dup(1);
	time(tv);
	tim = ctime(tv);
	if (argc<2) p = "r711";
	else p = argv[1];
	argc--;
	if (argc<2) person = logname();
	else person = argv[2];
	argc--;
	if (argc<2) message = "";
	else message = argv[3];
	line[0] = Cb;
	line[1] = Ci;
	line[2] = Cn;
	line[3] = blank;
	for (i=4; i<8 && *p; i++,p++)
		switch (*p) {
			case '0':	line[i] = C0;
					continue;
			case '1':	line[i] = C1;
					continue;
			case '2':	line[i] = C2;
					continue;
			case '3':	line[i] = C3;
					continue;
			case '4':	line[i] = C4;
					continue;
			case '5':	line[i] = C5;
					continue;
			case '6':	line[i] = C6;
					continue;
			case '7':	line[i] = C7;
					continue;
			case '8':	line[i] = C8;
					continue;
			case '9':	line[i] = C9;
					continue;
			case 'B':
			case 'b':	line[i] = Cb;
					continue;
			case 'I':
			case 'i':	line[i] = Ci;
					continue;
			case 'N':
			case 'n':	line[i] = Cn;
					continue;
			case 'R':
			case 'r':	line[i] = Cr;
					continue;
			default:	line[i] = blank;
					continue;
		}
	line[i] = 0;
	for (i=1; i<=NPAGES; i++) page(line,person,tim,message);
	flush();
}


page(line,person,tim,message)
{
	register int j,i;

	printf("%s%s\n%s%s\n",sep,sep,sep,sep);
	printf(fill);
	for (j=1; j<=3; j++) {
		wrt(line);
		printf("\n\n\n");
		for (i=1; i<=8; i++) printf("%-11s",person);
		printf("\n\n%s %s\n\n\n\n",message,tim);
	}
	printf(fill);
	printf("%s%s\n%s%s\n",sep,sep,sep,sep);
}


wrt(ap)
char ***ap;
{
	register char ***p;
	register int i;

	for (i=0; i<10; i++) {
		for (p=ap; *p; p++)
			printf("%s%s",(*p)[i],W);
		putchar('\n');
	}
}


# define NSMUDGE 100
# define WSMUDGE 200*8

# define PLOT 0200
# define TERMCOM 040

smudge()
{
	register int i,j,*st;
	int gs[3],ogs;

	st = gs;
	gtty(1,st);
	ogs = *st;
	flush();
	*st = PLOT;
	stty(1,st);
	*st =| TERMCOM;
	for (i=0; i<NSMUDGE; i++) {
		for (j=0; j<WSMUDGE/8; j++)
			putchar(0377);
	}
	flush();
	*st = ogs;
	stty(1,st);
}
