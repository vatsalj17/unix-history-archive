	int buff[256+8] 0;
	int k;
stop()
{
	printf("%l blocks !!\n\n",k);
	exit();
}
main(argc,argv)
int argc; char **argv;
{		int register i,j,l;	char *cp;	int tpofst,nrec;
	if(argc!=3) err("arg count\n\n	useage:	mksatp bootprogram output-tape\n");
	if((i=open(argv[1],0))<0) err("input??");
	if((j=creat(argv[2],0666))<0) err("output??");
	if(read(i,buff,sizeof buff)<0) err("read error");
	if(buff[0]!=0407) err("not magic");
	if((l=buff[1]+buff[2])>512) err("too big");
	if(write(j,&buff[8],512)!=512) err("write error");
	if(write(j,&buff[8],512)!=512) err("write error");
	tpofst = 2;
for(;;) {
	close(i);
	printf(" next file system to be dumped ?? ");
	if( (l=read(0,buff,sizeof buff))<=1 ) exit();;
	cp = &buff; cp[l-1]=0;
	if((i=open(buff,0))<0) continue;
	printf(" how many blocks to be dumped  ?? ");
	if( (l=read(0,buff,sizeof buff))<=1 ) nrec = 0177777;
		else {
			nrec = l = 0;
			while( (cp[l]<='9') && ( cp[l]>='0') ) nrec = nrec * 10  + cp[l++] - '0';
		}
	k = 0;  printf("%5l blocks to be written at tape offset %5l  --  ", nrec,tpofst);
	signal(2,stop);
	while( k != nrec )  { read(i,buff,512) ; write(j,buff,512);  k++; }
	signal(2,0);
	printf("%5l blocks written\n",k);
	tpofst =+ k;
	}
}
err(s)
char *s;
{	printf("-- %s\n",s);	exit(); 	}
