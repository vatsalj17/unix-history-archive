
int	buf[256];

main(argc,argv)
{
	register	fi,fo,l;
	fi = open("../l1100",0);
	fo = creat("a.out",0604);
	l=read(fi,buf,512);
	buf[0]= 0410;
	buf[1]=buf[4];
	buf[4]= buf[6];
	do write(fo,buf,l); while((l=read(fi,buf,512))>0);
}
