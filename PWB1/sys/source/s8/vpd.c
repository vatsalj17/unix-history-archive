#define ENXIO 6
# define SIGPIPE 13
# define SIGCLK 14
# define ENDFORK 33

int	mode[3]	{ 0120, 0, 0};
int	errno;

main()
{
	char *f, *name;
	int dfd,status,i,w,flag;
	extern error();

	if (fork()) exit(0);
	setpgrp();
	setuid(180); /* UID 180 (games) is reserved for VP & RJE */
	signal(1,1);
	signal(2,1);
	signal(3,1);
	signal(SIGPIPE,error);
	signal(SIGCLK,error);
	close(0);
	close(1);
	close(2);
	open("/dev/null",2);
	dup(0);
	dup(0);
	chdir("/usr/vpd");
	flag = 0;
	dfd =open(".", 0);
	while (f=next(dfd,1)) {
		close(1);
		vopen();
		unlink(".error");
		i = invoke("/bin/sh", f);
		close(1);
		dup(0);
		while ((w=wait(&status))!=i) ;
		if ((status&077) == SIGPIPE)
			error();
		unlink(f);
		flag = 1;
	}
	if (flag==0)
		exit(0);
	close(1);
	vopen();
}


invoke(p,arg)
{
	register int i,f;

	for (i=1; (f=fork())<0 && i<ENDFORK; i =* 2) sleep(i);
	if (f<0) exit();
	if (f) return(f);
	close(2);
	dup(1);
	execl(p,p,arg,0);
	exit(1);
}


next(fd,repeat)
{
	static struct {
		int ino;
		char entry[15];
	} dir;

	while (read(fd, &dir, 16)==16) {
		if (dir.ino==0) continue;
		if (dir.entry[0]=='q') return(dir.entry);
		if (dir.entry[0] != '.' && dir.entry[0] != 't')
			unlink(dir.entry);
	}
	seek(fd,0,0);
	if (repeat)
		return(next(fd,0));
	return(0);
}


error()
{
	alarm(0);
	creat(".error",0);
	write(open("/dev/tty8",1),"\nPrinter error!\n",16);
	exit(1);
}


vopen()
{
	alarm(30);
	if (open("/dev/vp0",1)!= 1) {
		if (errno==ENXIO) exit(0);
		else error();
	}
	alarm(0);
}
