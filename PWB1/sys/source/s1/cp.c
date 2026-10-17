/*
 * cp oldfile newfile
 */

main(argc,argv)
char **argv;
{
	static int buf[256];
	int fold, fnew, n;
	register char *p1, *p2, *bp;
	int mode;

	if(argc != 3) {
		prstr("Usage: cp oldfile newfile\n");
		exit(1);
	}
	if((fold = open(argv[1], 0)) < 0) {
		prstr("cp cannot open ");
		prstr(argv[1]);
		prstr("\n");
		exit(1);
	}
	fstat(fold, buf);
	mode = buf[2];
	/* is target a directory? */
	if (stat(argv[2], buf+50)>=0 && (buf[52]&060000)==040000) {
		p1 = argv[1];
		p2 = argv[2];
		bp = buf+100;
		while(*bp++ = *p2++);
		bp[-1] = '/';
		p2 = bp;
		while(*bp = *p1++)
			if(*bp++ == '/')
				bp = p2;
		argv[2] = buf+100;
	}
	if (stat(argv[2], buf+50) >= 0) {
		if (buf[0]==buf[50] && buf[1]==buf[51]) {
			prstr("cp: cannot copy file to itself.\n");
			exit(1);
		}
	}
	if ((fnew = creat(argv[2], mode)) < 0) {
		prstr("cp cannot create ");
		prstr(argv[2]);
		prstr("\n");
		exit(1);
	}
	while(n = read(fold,  buf,  512)) {
	if(n < 0) {
		prstr("cp read error\n");
		exit(1);
	} else
		if(write(fnew, buf, n) != n){
			prstr("cp write error.\n");
			exit(1);
		}
	}
	exit(0);
}
prstr(s)
char *s;
{
	register i;

	for(i=0;*s;i++)s++;
	write(2,s-i,i);
}
