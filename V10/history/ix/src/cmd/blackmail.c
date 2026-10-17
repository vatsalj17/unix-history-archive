#include "libc.h"
#include "sys/label.h"
#include "sys/types.h"
#include "sys/stat.h"
#include <ndir.h>
#include <errno.h>
extern int errno;

/*
 * Copy standard input into a new file in user's mail directory,
 * with sanitized names. 
 * Update FLAG to show new mail is here.
 * Remove zero length files in mail directory.
 *
 * Must be self-licencing T_NOCHK
 */
struct stat sbuf;
main(ac, av) 
char **av;
{
	struct label p, c;
	char *s, *mkunique();
	int i, n, fd;

	if(ac != 2) {
		fprint(2, "usage: %s user-mail-dir\n", av[0]);
		exit(1);
	}

	if(chdir(av[1]) == -1) {
		perror(av[1]);
		exit(1);
	}
	unlink("FLAG");
	tidyup();
	fd = creat("FLAG", 0644);
	n = time(0);
	s = ctime(&n);
	write(fd, s, strlen(s));
	close(fd);

	s = mkunique("m", 0644, &fd);
	if(s == 0){
		perror(av[1]);
		exit(1);
	}

	getplab(&p, &c);
	p.lb_u = p.lb_t = 0;
	p.lb_fix = F_FROZEN;
	c = p;
	setplab(&p, &c);

	dup2(fd, 1);
	
	execl("/usr/bin/sed", "sed", 
		"-e", "1s/ remote from .*$//", 
		"-e", "2,$s/^From/>From/", 
		/* "-e", "$a\\\n", */
		0);
	execl("/bin/sed", "sed", 
		"-e", "1s/ remote from .*$//", 
		"-e", "2,$s/^From/>From/", 
		/* "-e", "$a\\\n", */
		0);
	exit(1);
}

/*
 * Perform a housekeeping function which logically belongs to redmail.
 * But redmail is not trusted, so it must not remove letters, lest it
 * contaminate the mail directory.  So redmail just truncates them, and
 * lets blackmail remove them next time a letter is delivered.
 * In the boundary case of no input, blackmail stops here: this can
 * be used by redmail to remove FLAG.
 */
tidyup() {
	struct stat sbuf;
	DIR *mailDIR;
	struct direct *dirent;
	
	if(mailDIR = opendir("."))
		while(dirent = readdir(mailDIR)) {
			if(stat(dirent->d_name, &sbuf) == 0 
			    && sbuf.st_size == 0)
				unlink(dirent->d_name);
		}
	closedir(mailDIR);

	if(fstat(0, &sbuf) == -1 && errno == EBADF)
		exit(0);
}
