#include "stdio.h"

#define CASE 040

int allcap;
int debug 0;
char word1[100], word2[100];
FILE *file1, *file2;

main(argc,argv)
char **argv;
{
	register char *s;
	if(argc!=3) 
		diag("arg count");
	if(argv[1][0]!='-')
		file1 = fopen(argv[1],"r");
	else
		file1 = stdin;
	if(argv[2][0]!='-')
		file2 = fopen(argv[2],"r");
	else
		file2 = stdin;
	if(file1==0||file2==0)
		diag("cannot open input");
	if(!gets(word1,file1))
		exit(0);
	if(gets(word2,file2)) for(;;) {
		if(debug) printf("%s %s\n",word1,word2);
		switch(compare()) {
		case -1:
			output();
		case 0:
			if(!gets(word1,file1))
				exit(0);
			allcap = 1;
			for(s=word1;*s;s++) 
				if(!(allcap=& ('A'<=*s&'Z'>=*s)))
					break;
			continue;
		case 1:
			if(gets(word2,file2))
				continue;
		}
		break;
	}
	while(gets(word1,file1))
		output();
}
compare()
{
	register char *s,*t;
	if(allcap) {
		for(s=word1,t=word2;*s==(*t&~CASE);s++,t++)
			if(*s==0) return(0);
		return(*s<(*t&~CASE)?-1:1);
	}
	s = word1; t = word2;
	if((*t&CASE)==0)
		s--,t--;
	else if((*s|CASE)!=*t)
		return((*s|CASE)<*t?-1:1);
	while(*++s==*++t)
		if(*s==0) return(0);
	while((*s|040) == (*t|040)) {
		if(*s==0) return(-1);
		s++; t++;
	}
	return((*s|040)<(*t|040) ? -1:1);
}

gets(s,f)
char *s;
FILE *f;
{
	while((*s = getc(f))!='\n')
		if(*s++==-1) return(0);
	*s = 0;
	return(1);
}

diag(s)
char *s;
{
	fprintf(stderr,"spell0: %s\n",s);
	exit(1);
}

output()
{
	printf("%s\n",word1);
}
