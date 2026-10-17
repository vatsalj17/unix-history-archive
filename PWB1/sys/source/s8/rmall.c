struct stbuf	{
	int dev;
	int inum;
	int mode;
	char nlink;
	char uid;
	};
int fflg, rflg, dflg;

main(argc, argv)
char *argv[];
{
	char *arg;

	if (ttyn(0) == 'x')
		fflg++;
	while(--argc > 0) {
		arg = *++argv;
		if(arg[0] == '-') switch(arg[1]) {
			case 'f':
				fflg++;
				continue;
			case 'd':
				dflg++;
			case 'r':
				rflg++;
				continue;
		}
		rm(arg);
	}
}

rm(arg)
char arg[];
{
	int buf[20];
	register i, b;

	if(stat(arg, buf)) {
		printf("%s: non existent\n", arg);
		return;
	}
	if((buf->mode & 060000) == 040000)	{
		if(rflg) {
			i = fork();
			if(i == -1) {
				printf("%s: try again\n", arg);
				return;
			}
			if(i) {
				while(wait(0) != i);
				return;
			}
			if(chdir(arg)) {
				printf("%s: cannot chdir\n", arg);
				exit(1);
			}
			close(3);
			if((i=open(".",0))<0) {
				printf("%s: cannot open\n", arg);
				exit(1);
			}
			buf[8] = 0;
			while(read(i,buf,16)==16) {
				if(buf[0]==0)
					continue;
				if(buf[1]=='.')
					continue;
				if((buf[1]=='..') && (buf[2]==0))
					continue;
				rm(&buf[1]);
			}
			if(dflg) {
				if(chdir("..")) {
					printf("..: cannot chdir\n");
					exit(1);
				}
				execl("/bin/rmdir","rmdir",arg,0);
				printf("%s: no rmdir\n", arg);
			}
			exit(1);
		}
		printf("%s: directory\n", arg);
		return;
	}
	if(!fflg) {
		if (access(arg, 02)<0) {
			printf("%s: %o mode ", arg, buf->mode);
			i = b = getchar();
			i = b;
			while(b != '\n' && b != '\0')
				b = getchar();
			if(i != 'y')
				return;
		}
	}
	if(unlink(arg))
		printf("%s: not removed\n", arg);
}

putchar(c)
{
	write(2, &c, 1);
}

getchar()
{
	char c;

	if(read(0, &c, 1) != 1) return(0);
	return(c);
}
