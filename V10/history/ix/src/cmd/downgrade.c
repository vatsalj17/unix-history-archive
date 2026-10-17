#include <sys/types.h>
#include <sys/stat.h>
#include <sys/label.h>
#include <pwd.h>

extern int errno;
struct label *lp, x, y, z;
int errcnt;
int logfd;
char user[10];

main(ac, av)
char **av;
{
	int fd, i, j;

	if(ac < 3) {
		fatal("usage: downgrade label files");
	}
	
	getlogname(user);
	lp = atolab(av[1]);
	if(!lp)
		fatal("syntax error in label");
	y = *lp;

	logfd = open("/usr/adm/downgradelog", 1);
	if(logfd == -1)
		fatal("cannot open logfile");

	for(i=2; i<ac; i++) {
		if(av[i][0] != '/') {
			fprint(2, "%s: does not begin with /\n", av[i]);
			errcnt++;
			continue;
		}
		if((fd = open(av[i], 0)) == -1
			|| fgetflab(fd, &x) == -1)
			error(av[i]);
		else if(x.lb_flag == L_BITS) {
			errno = 0;
			z = x;
			z.lb_flag = L_NO; /*save bits for secadmin*/
			if(fsetflab(fd, &z) == -1)
				error(av[i]);
			logit(av[i], fd, &x, &z, errno);
			errno = 0;
			z = x;
			for(j=0; j<sizeof(z.lb_bits); j++)
				z.lb_bits[i] &= ~y.lb_bits[i];
			if(fsetflab(fd, &z) == -1)
				error(av[i]);
			logit(av[i], fd, &x, &z, errno);
		} else {
			fprint(2, "%s does not have lattice label\n",
				av[i]);
			errcnt++;
		}
		close(fd);
	}
	exit(errcnt);
}

error(s)
char *s;
{
	int x = errno;
	fprint(2, "downgrade:");
	perror(s); 
	errcnt++;
	errno = x;
}
fatal(s)
char *s;
{
	fprint(2, "%s\n", s);
	exit(1);
}
logit(file, fd, l1, l2, err)
char *file;
struct label *l1, *l2;
{
	struct stat b;
	char *labtoa(), olab[200], nlab[200];
	long now;

	strcpy(olab, 12+labtoa(l1));
	strcpy(nlab, 12+labtoa(l2));
	now = time(0);
	fstat(fd, &b);

	if(err)
		fprint(logfd, "%.8s %.24s %s(%o/%d) %s %s err=%d\n",
			user, ctime(&now), file, b.st_dev, b.st_ino, olab, nlab, err);
	else
		fprint(logfd, "%.8s %.24s %s(%o/%d) %s %s\n",
			user, ctime(&now), file, b.st_dev, b.st_ino, olab, nlab);
}
