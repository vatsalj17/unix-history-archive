#
char copr[] "Copyright (c) 1976 Thomas S. Duff";
/* "Thomas S. Duff is a shit-head"*/
/* "RAW devices are as safe as playing with a tactical thermonuclear device" */

#define	AUSAM	/* mainly 16-bit uid in inodes */

struct{
	int flags;
	char nlinks;
#ifndef	AUSAM
	char uid;
	char gid;
#endif
#ifdef	AUSAM
	char uidlob;
	char uidhib;
#endif
	char size0;
	int size1;
	int addr[8];
	long actime;
	long modtime;
}inode;
int f;
int pflag;
char lastc;
int isize;
char peekc '\0';
getc(){
	if(peekc!='\0'){
		lastc=peekc;
		peekc='\0';
	}
	else if(read(0, &lastc, 1)!=1)
		lastc='\0';
	return(lastc);
}
endline(){
	register char c;
	while(any(c=getc(), " \t"));
	if(c=='p'){
		pflag++;
		while(any(c=getc(), " \t"));
	}
	if(c!='\n')
		error();
}
error(){
	printf("\7?\n");
	if(lastc!='\n')
		while(getc()!='\n');
	reset();
}
any(ac, as)
char as[];
{
	register char c, *s;
	c=ac;
	s=as;
	while(*s)
		if(c == *s++)
			return(1);
	return(0);
}
#define INTR 2
#define QUIT 3
intr(){
	signal(INTR, intr);
	lastc='\n'; /* cryptic */
	error();
}
main(argc, argv)
char *argv[];
{
	register j, i;
	int n,inum;
	char c;
	int savintr, savquit;
	int buf[18]; /*	needed for fstat */

	if(argc!=3){
		write(2,"arg count\n",10);
		exit(-1);
	}
	if((f=open(argv[1],2))<0){
		perror(argv[1]);
		exit(-1);
	}
	if( (fstat(f,buf)<0) || ((buf[2]&060000)!=060000) ) {
		/* it ain't a fu..ing block device  !!  */
		/* using raw devices is fatal !!        */
		/* enable for raw if ya game           */
		printf("%s is not a block device !!\n",argv[1]);
		exit(-1);
	}
	seek(f, 1, 3);
	if(read(f, &isize, sizeof isize)!=sizeof isize){
		perror("super block");
		exit(-1);
	}
	if(!iget(inum=atoi(argv[2]))){
		write(2, "can't read inode\n", 16);
		exit(-1);
	}
	setexit();
	signal(INTR, intr);
	for(;;){
		write(2,".",1);
		while(any(c=getc(), " \t"));
		switch(c){
		default:
			error();
		case 'q':
			endline();
		case '\0':
			exit(0);
		case '\n':
			break;
		case 'e':
			i=rin();
			endline();
			if(iget(i))
				inum=i;
			else
				error();
			break;
		case '!':
			savintr=signal(INTR, 1);
			savquit=signal(QUIT, 1);
			i=fork();
			if(i== -1)
				printf("Try again.\n");
			else if(i==0){
				execl("/bin/sh", "sh", "-t", 0);
				perror("/bin/sh");
				exit(-1);
			}
			else{
				wait();
				printf("!\n");
				signal(INTR, savintr);
				signal(QUIT, savquit);
			}
			break;
		case 'p':
			endline();
			pflag++;
			break;
		case 'f':
			i=rin();
			endline();
			inode.flags=i;
			break;
		case 'l':
			i=rin();
			endline();
			inode.nlinks=i;
			break;
		case 'u':
			i=rin();
			endline();
#ifndef	AUSAM
			inode.uid=i;
#endif
#ifdef	AUSAM
			inode.uidhib=(i>>8)&0377;
			inode.uidlob=(i&0377);
#endif
			break;
#ifndef	AUSAM
		case 'g':
			i=rin();
			endline();
			inode.gid=i;
			break;
#endif
		case 's':
			i=rin();
			j=rin();
			endline();
			inode.size0=i;
			inode.size1=j;
			break;
		case 'a':
			n=rin();
			if(n<0 || n>7)
				error();
			i=rin();
			endline();
			inode.addr[n]=i;
			break;
		case 'w':
			endline();
			seek(f,-sizeof inode,1);
			if(write(f,&inode,sizeof inode)!=sizeof inode)
				perror("write");
			break;
		}
		if(pflag){
			pflag=0;
#ifndef	AUSAM
			printf("flags: %o links: %l uid: %l gid: %l  size: %s (%o,%o)\n",
#endif
#ifdef	AUSAM
			printf( "flags: %o links: %l uid: %l size: %s (%o,%o)\n",
#endif
				inode.flags,inode.nlinks,
#ifndef	AUSAM
				inode.uid,inode.gid,
#endif
#ifdef	AUSAM
				(inode.uidhib<<8)+(inode.uidlob&0377),
#endif
				locv(inode.size0&0377,inode.size1),
				inode.size0&0377,inode.size1);
			printf("addr:\n");
			for(i=0;i<8;i++)
				printf("%l\t(%o)\n",inode.addr[i],inode.addr[i]);
			printf("\nactime:  %s", ctime(inode.actime));
			printf("modtime: %s",ctime(inode.modtime));
		}
	}
}
rin(){
	char c;
	register n, base;
	n=0;
	while(any(c=getc(), " \t"));
	if(c<'0' || c>'9'){
		peekc=c;
		error();
	}
	base=c=='0'?8:10;
	do
		n=n*base+c-'0';
	while('0'<=(c=getc()) && c<='9');
	peekc=c;
	return(n);
}
iget(inum){
	register block;
	register nread;
	block=(inum+31)/16;
	if(block>=isize+2 || block<2)
		return(0);
	seek(f, block, 3);
	seek(f, 32*((inum+31)%16), 1);
	nread=read(f, &inode, sizeof inode);
	if(nread== -1){
		perror("iget");
		return(0);
	}
	else if(nread<sizeof inode){
		write(2, "iget: eof on read\n", 18);
		return(0);
	}
	return(1);
}
