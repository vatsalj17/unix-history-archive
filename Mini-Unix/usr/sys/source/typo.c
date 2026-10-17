int	status;
int eflg;
char w2006[100];
flg 0;
char *fptr;
char *ffptr &ffbuf;
char ffbuf[36];
int	npr;
char nwd[100];
char *buf[3];
file[3];
ptr[3];
char *name[3];
bsp[768];
int ct;

main(argc,argv) int argc; char *argv[]; {
	char let,lt;
	auto arg,t,sw,i,j,er,c;
	register k,l,m;
	int unl();

	nice(-20);
	buf[0] = bsp;
	buf[1] = bsp + 0400;
	buf[2] = bsp + 01000;
	ptr[0] = 0; ptr[1] = 0;
	ptr[2] = 1;
	arg = 1;
	while(argc>1 && argv[arg][0] == '-') {
		switch(argv[arg][1]) {
		default:
			printf("Unrecognizable argument: %c\n",argv[arg][1]);
				exit();
		case '1':
				npr++;
		}
		arg++;
		--argc;
	}
	if((signal(2,1) & 1) != 1)
	signal(2,unl);
	name[0] = "/usr/tmp/ttmpa1";
	name[1] = "/usr/tmp/ttmpa2";
	name[2] = "/usr/tmp/ttmpa3";
	while((file[0] = open(name[0],1)) > 0){
		close(file[0]);
		for(j=0; j < 3; j++)name[j][13]++;
		if(name[0][13] == 'z')err("creat tmp file");
	}
	file[0] = creat(name[0],0666);
	fptr = argv[arg];
	if(argc == 1) {argc = 2; arg = 0;}
	while(--argc){
		if(arg == 0){
			file[2] = 0;
		}else{
			file[2] = open(argv[arg++],0);
			if(file[2] < 0)err("open input file");
		}
		eflg = 1;
		while((j = wdval(2)) != 0){
			put(0,nwd,ct);
		}
		if(file[2]) close(file[2]);
	}
	flsh(0,0);
	close(file[0]);
	sw = fork();
		if(sw == 0){execl("/usr/bin/usort","usort","-o",name[2],name[0],0);
			err("sort"); }
		if(sw == -1)err("fork");
	er = wait(&status);
		if(er != sw)err("probs");
	file[0] = creat(name[0],0666);
		if(file[0] < 0)err("creat tmp");
	file[1] = open("/usr/lib/w2006",0);
		if(file[1] < 0)err("open w2006");
	ptr[1] = 1;
	for(j=0; (w2006[j] = get(1)) != '\n';j++);
	file[2] = open(name[2],0);
		if(file[2] < 0)err("open tmp");
	ptr[2] = 1;

	while(ptr[2]){
		l=0;
		for(k=0;((c = nwd[k] = get(2)) != '\n');k++)
			if(c == -1)goto done;
		for(i=0; i<=k;i++){
			if(nwd[i] < w2006[l]){
				put(0,nwd,k);
				break;
			}
			if(nwd[i] > w2006[l]){
				for(l=0; (w2006[l] = get(1)) != '\n';l++);
					if(l == -1){
						put(0,nwd,k);
						for(k=0;((c = nwd[k] =get(2))!= -1);k++){
							put(0,nwd,k);
							k = -1;
						}
						goto done;
				}
				i = -1;
				l=0;
				continue;
			}
			l++;
		}
	}
done:
	close(file[2]); 
	flsh(0,0);
	close(file[1]);
	close(file[0]);

	sw = fork();
		if(sw == 0){
			if(npr) {
				execl("/bin/cat","cat",name[0],0);
			} else {
				i = 0 ;
				while((c = "Possible typo's in "[i++])!=0)
					*ffptr++ = c;
				i = 0;
				while((c = fptr[i++]) != 0)
					*ffptr++ = c;
				*ffptr = 0;
				execl("/bin/pr","pr","-3", "-h",
				ffbuf,name[0],0);
				err("pr");
		}
	}
		if(sw == -1)err("fork");
	er = wait(&status);
		if(er != sw)err("prob");
	unl();
}

unl() {
	register j;
	j = 2;
	exit();
}


err(c) char c[];{
	register j;
	printf("cannot %s\n",c);
	unl();
}

get(ifile) int ifile;{
	static char *ibuf[10];
	if(--ptr[ifile]){
		return(*ibuf[ifile]++ & 0377);}
	if(ptr[ifile] = read(file[ifile],buf[ifile],512)){
		if(ptr[ifile] < 0)goto prob;
		ibuf[ifile] = buf[ifile];
		return(*ibuf[ifile]++ & 0377);
	}
	ptr[ifile] = 1;
	return(-1);

prob:
	ptr[ifile] = 1;
	printf("read error\n");
	return(-1);
}

put(ofile,s,optr) char s[]; {
	register i;

	while(optr-- >= 0)
		 buf[ofile][(ptr[ofile] < 512)?ptr[ofile]++:flsh(ofile,1)] = *s++;
	return;
}

flsh(ofile,i){
	register error;
	error = write(file[ofile],buf[ofile],ptr[ofile]);
	if(error < 0)goto prob;

	ptr[ofile] = i;
	return(0);
prob:
	printf("write error on t.%d\n",file[ofile]);
	unl();
}

wdval(wfile) int wfile; {
	static let,wflg;
	register k;
beg:
	k = -1;
	if(wflg == 1){wflg = 0;
		goto st; }
	while((let = get(wfile)) != '\n'){
st:
		switch(let){
		case -1:	return(0);
		case '%':	if(k != -1)break;
					goto ret;
		case '-':
				if((let = get(wfile)) == '\n'){
					while((let = get(wfile)) == '\n')if(let == -1)return(0);
					goto st; }
				else {wflg = 1;
					goto ret; }
		case '\'':
				if(eflg != 1){
					if(k < 0)goto beg;
					if(((let=get(wfile)) >= 'A' && let <= 'Z')||
						(let >= 'a' && let <= 'z')){
						nwd[++k] = '\'';
							goto st;
						}
					else {
						wflg = 0;
						goto ret;
					}
				}
		case '.':
				if(eflg == 1){
					while((let = get(wfile)) != '\n')if(let == -1)return(0);
					goto beg; }
				else goto ret;
		default:
				eflg = 0;
				if(let < 'A')goto ret;
				if(let <= 'Z'){
					nwd[++k] = let + ' ';
					break; }
				if(let < 'a' || let > 'z')goto ret;
				nwd[++k] = let;
			}
		 eflg = 0;	}

	eflg = 1;
ret:
	if(k < 1)goto beg;
	nwd[++k] = '\n';
	ct = k;
	return(k);
}

