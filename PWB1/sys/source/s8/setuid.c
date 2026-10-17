 /* setuid -- Set real and effective user-id to the argument.

	e.g.:
		setuid hasp [login_directory]
		command ....
	The "command" will be executed (via "sh -t") as if user
	"hasp" had executed it.

	Only the super-user can use "setuid".
 */

struct ibuf {
	int	idev;
	int	inum;
	int	iflags;
	char	inl;
	char	iuid;
	char	igid;
	char	isize0;
	char	*isize;
	int	iaddr[8];
	int	iatime[2];
	int	imtime[2];
} statb;

struct {
	char	logname[8],
		logdir[22],
		logtty[2];
} u;

char pin[518];

main(argc, argv) char **argv; {
int i;

	if(argc < 2) {
		printf("Usage: setuid user_name [ log_dir ]\n");
		printf("command to be executed...\n");
		exit(9);
	}
	if((i=getunum(argv[1]))== -1) {
		printf("invalid user name\n");
		exit(9);
	}

	fstat(0, &statb);
	if(statb.iflags & 020000) printf("%% ");
	strcpy(u.logname, argv[1]);
	if(argc == 3) {
		strcpy(u.logdir, argv[2]);
		chdir(u.logdir);
	} else
		strcpy(u.logdir, logdir());
	strcpy(u.logtty, logtty());
	logpost(&u);
	setuid(i<<8 | i);
	execl("/bin/sh", "sh", "-t", 0);
	printf("setuid failed!\n");
	exit(9);
}

getunum(s) char *s; { /* find username in /etc/passwd & return num. */
int i;
char str[20], *sp, c;
	i = -1;
	fopen("/etc/passwd",&pin);
	c = '\n'; /* prime with a CR */
	do {
		if(c=='\n') {
			sp = str;
			while((*sp = getc(pin)) != ':')
				if(*sp++ == -1) goto RET;
			*sp = '\0';
			if(strcmp(str,s)==0) {
				while((c=getc(pin)) != ':')
					if(c == -1) goto RET;
				sp = str;
				while((*sp = getc(pin)) != ':') sp++;
				*sp = '\0';
				i = atoi(str);
				goto RET;
			}
		}
	} while(c = getc(pin));
 RET:
	close(pin[0]);
	return(i);
}
