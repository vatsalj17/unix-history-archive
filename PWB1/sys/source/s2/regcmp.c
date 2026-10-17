int __i_size;
char ofile[64];
char a1[1024];
char a2[64];
int iobuf[259];
main(argc,argv) char **argv;
{
	register char *name, *str, *v;
	char *bp, *cp, *sv;
	int i,j,k,cflg;
	char c;

	if (*argv[1] == '-') {
		cflg++;
		++argv;
		argc--;
	}
	else cflg = 0;
	while(--argc) {
		++argv;
		bp = *argv;
		if ((i=fopen(*argv,iobuf))<0) {
			write(2,"can not open ",13);
			write(2,*argv,size(*argv));
			write(2,"\n",1);
			continue;
		}
		cp = ofile;
		while(*++bp)
			if(*bp == '/') *bp = '\0';
		while(*--bp == '\0');
		while(*bp != '\0' && bp > *argv) bp--;
		while(*cp++ = *bp++);
		cp--; *cp++ = '.';
		if(cflg) *cp++ = 'c';
		else *cp++ = 'i';
		*cp = '\0';
		close(1);
		if (creat(ofile,0644)<0)  {
			write(2,"can not create .i file\n",23);
			exit(1);
		}
	   while(1) {
		str = a1;
		name = a2;
		while(((c=getc(iobuf)) == '\n') || (c == ' '));
		if(c==-1) break;
		*name++ = c;
		while(((*name++=c=getc(iobuf)) != ' ') && (c != -1) && (c != '\n'));
		*--name = '\0';
		while(((c=getc(iobuf)) == ' ') || (c == '\n'));
		if(c != '"') {
			if (c==-1) {write(2,"unexpected eof\n",15); exit(1);}
			write(2,"missing initial quote for ",26);
			write(2,a2,size(a2));
			write(2," : remainder of line ignored\n",29);
			while((c=getc(iobuf)) != '\n');
			continue;
		}
	keeponl:
		while((c=getc(iobuf)) >= 0) {switch(c) {
		case '"':
			break;
		case '\\':
			switch(c=getc(iobuf)) {
			case 't':
				*str++ = '\011';
				continue;
			case 'n':
				*str++ = '\012';
				continue;
			case 'r':
				*str++ = '\015';
				continue;
			case 'b':
				*str++ = '\010';
				continue;
			case '\\':
				*str++ = '\\';
				continue;
			default:
				if (c<='7' && c>='0') 
						*str++ = getnm(c);
				else *str++ = c;
				continue;
			}
		default:
			*str++ = c;
		}
		if (c=='"') break;
		}
		if (c==-1) {write(2,"unexpected eof\n",15); exit(1);}
		while(((c=getc(iobuf)) == '\n') || (c == ' '));
		if (c=='"') goto keeponl;
		else {
			iobuf[1] =+ 1;
			iobuf[2] =- 1;
		}
		*str = '\0';
		if(!(sv=v=regcmp(a1,0))) {
			write(2,"fail: ",6);
			write(2,a2,size(a2));
			write(2,"\n",1);
			continue;
		}
		printf("/* \"%s\" */\n",a1);
		printf("char %s[] {\n",a2);
		while(__i_size > 0) {
			for(k=0;k<12;k++)
				if(__i_size-- > 0) printf("0%o,",*v++);
			printf("\n");
		}
		printf("0};\n");
		free(sv);
	   }
	   close(i);
	}
	exit(0);
}
size(p) char *p;
{
	register i;
	register char *q;

	i = 0;
	q = p;
	while(*q++) i++;
	return(i);
}
getnm(j) char j;
{
	register int i;
	register char c;
	i = j - '0';
	while((c=getc(iobuf)) >= '0' && c <= '7') 
		i = (i*8+(c-'0'));
	iobuf[1] =+ 1;
	iobuf[2] =- 1;
	return(i);
}
