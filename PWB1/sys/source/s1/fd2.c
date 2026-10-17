static char Sccsid[] "@(#)fd2.c	1.4";
/*
	Command to open file descriptor 2 to a file and exec
	a command.  Four forms:

		fd2 comd ...		[write on "msg.out"]
		fd2 -file comd ...	[write on "file"]
		fd2 --file comd ...	[append to "file"]
		fd2 -- comd ...		[append to "msg.out"]

	Additional usage:

		fd2 + comd ...

	causes file descriptor 2 to be made same as 1.
*/

main(argc,argv)
int argc;
char *argv[];
{
	char *file, app, path[100];
	int fd, start;

	if(argv[1][0] == '+') {
		start = 2;
		close(2);
		dup(1);
	}
	else {
		if(argv[1][0] != '-') {
			file = "\0";
			app = 0;
			start = 1;
		}
		else if(argv[1][1] != '-') {
			file = &argv[1][1];
			app = 0;
			start = 2;
		}
		else {
			file = &argv[1][2];
			app = 1;
			start = 2;
		}
		if(file[0] == '\0') file = "msg.out";
	
		if(app)
			if((fd=open(file,1)) < 0) app = 0;
			else seek(fd,0,2);
		if(!app)
			fd = creat(file,0666);
		if (fd > 2) {
			close(2);
			if (dup(fd) != 2) {
				write(open("/dev/tty",1),"bad file descriptor?!?\n",23);
				exit(1);
			}
			close(fd);
		}
	}

	argv[argc] = 0;
	pexec(argv[start],&argv[start]);
	exit(1);
}
