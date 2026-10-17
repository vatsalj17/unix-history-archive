#include <stdio.h>
#define LB 250
int	one;
int	two;
int	three;

char	*ldr[3];

FILE *ib1;
FILE *ib2;
FILE *openfil();
main(argc,argv)
char	*argv[];
{
	extern	fout;
	int	l;
	char	lb1[LB],lb2[LB];

	ldr[0] = "";
	ldr[1] = "\t";
	ldr[2] = "\t\t";
	if(argc > 1)  {
		if(*argv[1] == '-' && argv[1][1] != 0) {
			l = 1;
			while(*++argv[1]) {
				switch(*argv[1]) {
				case'1':
					if(!one) {
						one = 1;
						ldr[1][0] =
						ldr[2][l--] = '\0';
					}
					break;
				case '2':
					if(!two) {
						two = 1;
						ldr[2][l--] = '\0';
					}
					break;
				case '3':
					three = 1;
					break;
				default:
				mesg("comm: illegal flag\n");
				exit(1);
				}
			}
			argv++;
			argc--;
		}
	}

	if(argc < 3) {
		mesg("comm: arg count\n");
		exit(1);
	}

	ib1 = openfil(argv[1]);
	ib2 = openfile(argv[2]);
	fout = dup(1);


	if(rd(ib1,lb1) < 0) {
		if(rd(ib2,lb2) < 0)	exit(0);
		copy(ib2,lb2,2);
	}
	if(rd(ib2,lb2) < 0)	copy(ib1,lb1,1);

	while(1) {

		switch(compare(lb1,lb2)) {

			case 0:
				wr(lb1,3);
				if(rd(ib1,lb1) < 0) {
					if(rd(ib2,lb2) < 0)	fexit();
					copy(ib2,lb2,2);
				}
				if(rd(ib2,lb2) < 0)	copy(ib1,lb1,1);
				continue;

			case 1:
				wr(lb1,1);
				if(rd(ib1,lb1) < 0)	copy(ib2,lb2,2);
				continue;

			case 2:
				wr(lb2,2);
				if(rd(ib2,lb2) < 0)	copy(ib1,lb1,1);
				continue;
		}
	}
}

rd(file,buf)
FILE *file;
char *buf;
{

	register int i;
	i = 0;
	while((*buf = getc(file)) >=0) {
		if(*buf == '\n' || i > LB-2) {
			*buf = '\0';
			return(0);
		}
		i++;
		buf++;
	}
	return(-1);
}

wr(str,n)
	char	*str;
{

	switch(n) {

		case 1:
			if(one)	return;
			break;

		case 2:
			if(two)	return;
			break;

		case 3:
			if(three)	return;
	}
	printf("%s%s\n",ldr[n-1],str);
}

copy(ibuf,lbuf,n)
FILE *ibuf;
{
	do {
		wr(lbuf,n);
	} while(rd(ibuf,lbuf) >= 0);

	exit(0);
}

compare(a,b)
	char	*a,*b;
{
	register char *ra,*rb;

	ra = --a;
	rb = --b;
	while(*++ra == *++rb)
		if(*ra == '\0')	return(0);
	if(*ra < *rb)	return(1);
	return(2);
}
fexit()
{
	exit(0);
}

FILE *openfil(s)
char *s;
{
	FILE *b;
	if(s[0]=='-' && s[1]==0)
		b = stdin;
	else if((b=fopen(s,"r")) == NULL) {
		mesg("comm: cannot open ");
		mesg(s);
		mesg("\n");
		exit(1);
	}
	return(b);
}

mesg(s)
char *s;
{
	while(*s)
		write(2,s++,1);
}
