#include "ctype.h"
#include "sys/label.h"
#include "fio.h"
#include <libc.h>
#include <ftw.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

struct stat root;
struct label *Atolab();
char *labtoa();
char *ids();
int checkbad();
int nbad;

struct file {
	char *name;
	int uid;		/* masked to 0xffff. -1 = wildcard */
	int gid;
	int mode;
	int visited;
	struct label lab;
	struct file *next;
} *new, *filsys;
struct label *fp;

int	fflag = 0;		/* wisdom ??? */
int	Fflag = 0;		/* valor ??? */

main(ac, av)
char **av;
{
	struct stat s;
	int i;

	for(i=1; i<ac; i++) {
		/*
		if(strcmp(av[i], "-f") == 0) {
			fflag = 1;
			continue;
		}
		else if(strcmp(av[i], "-F") == 0) {
			Fflag = 1;
			continue;
		}
		*/
		if(stat(av[i], &root) == -1) {
			perror(av[i]);
			continue;
		}
		init();
		if((root.st_mode & S_IFMT) == S_IFDIR) {
			char *v[6];

			v[0] = av[i];
			v[1] = "bin,bin";
			v[2] = "0644";
			v[3] = "-----";
			v[4] = "-----";
			v[5] = "0000...";

			tote(v);
			ftw(av[i], checkbad, 100);
		}
		else if(parse(av[i])) {
			if(stat(filsys->name, &root) == -1) {
				perror(av[i]);
				continue;
			}
			checkgood();

			ftw(filsys->name, checkbad, 100);
		}
		fflush(stdout);
	}
	return(nbad?1:0);
}

checkgood() {
	struct label lab;
	struct stat buf;
	struct file *p;

	for(p=filsys->next; p; p=p->next) {
		if(stat(p->name, &buf) == -1)
			perror(p->name);
		else if(getflab(p->name, &lab) == -1)
			perror(p->name);
		else {
			cfile(p->name, p, &buf, &lab);
		}
	}
}

cfile(name, p, sb, lp)
register char *name;
register struct file *p;
register struct stat *sb;
register struct label *lp;
{
	if(p == 0)
		goto bad;
	if(p->visited++) return;
#	define mm(x) (x&07777)
	if(mm(p->mode) != mm(sb->st_mode)) goto bad;
	if(p->uid != -1 && p->uid != sb->st_uid) goto bad;
	if(p->gid != -1 && p->gid != sb->st_gid) goto bad;
	if(!labEQ(&p->lab, lp)) goto bad;
	if(p->lab.lb_t != lp->lb_t) goto bad;
	if(p->lab.lb_u != lp->lb_u) goto bad;

	if((p->lab.lb_flag == L_BITS) && !labLE(&p->lab, fp)) goto bad;

	p->visited = 1;
	return;
bad:
	nbad++;
	printf("%-20s %15s %o %s\n", 
		name, 
		ids(sb->st_uid, sb->st_gid), 
		sb->st_mode, 
		labtoa(lp));
	/*
	if(fflag && (p != 0))
		fixit(name, sb, lp, p->uid, p->gid, p->mode, &p->lab);
	else if(Fflag && (p == 0)) {
		fixit(name, sb, lp, p->uid, p->gid, p->mode, &p->lab);
	}
	*/
}

/*
 * dubious
 */
fixit(name, sb, lp, newu, newg, newmode, newlab)
char *name;
struct stat *sb;
struct label *lp;
struct label *newlab;
{
	struct label templab;
	int x, u, g;

	if(lp->lb_t || lp->lb_u) {
		templab = *lp;
		templab.lb_fix = F_LOOSE;
		templab.lb_t = 0;
		templab.lb_u = 0;
		setflab(name, &templab);
	}
	x = 0;
	u = sb->st_uid;
	g = sb->st_gid;
	if(newu != -1 && newu != u) { x++; u = newu; }
	if(newg != -1 && newg != g) { x++; g = newg; }
	if(x != 0)
		if(chmod(name, 0) == -1 || chown(name, u, g) == -1)
			perror(name);
	if(chmod(name, newmode) == -1)
		perror(name);
	x = 0;
	if(0 != newlab->lb_t) x++;
	if(0 != newlab->lb_u) x++;
	if( !labEQ(newlab, lp)) x++;
	if(x != 0)
		if(setflab(name, newlab) == -1)
			perror(name);
}

struct label undef;

int fn();
checkbad(name, sb, code, s)
register char *name;
register struct stat *sb;
struct FTW *s;
{
	int suspect = 0;
	struct label lab;
	register struct file *p;
	int mode;

	if(code == FTW_D && sb->st_dev != root.st_dev) {
		s->quit = FTW_SKD;
		return 0;
	} else if(code == FTW_DP)
		return 0;
	if(getflab(name, &lab) == -1) {
		perror(name);
		lab = undef;
		suspect++;
	}

	mode = sb->st_mode & S_IFMT;

	if(lab.lb_flag == L_YES) suspect++;
	else if(lab.lb_flag == L_UNDEF) suspect++;
	else if(lab.lb_flag == L_BITS && labLE(&lab, &filsys->lab) == 0)
		suspect++;
	else if(lab.lb_t & ~fp->lb_t) suspect++;
	else if(lab.lb_u & ~fp->lb_u) suspect++;
	else if(mode == S_IFBLK || mode == S_IFCHR) {
		if(lab.lb_flag != L_NO) suspect++;
	} else {
		if(lab.lb_flag == L_NO) suspect++;
	}
	if(suspect) {
		for(p=filsys->next; p; p=p->next)
			if(strcmp(p->name, name) == 0)
				break;
		cfile(name, p, sb, &lab);
	}
	return 0;
}

init() {
	struct file *p, *q;

	for(p=filsys; p; p = q) {
		q = p->next;
		if(p->name)free(p->name);
		free((char *)p);
	}
	new = filsys = 0;
}
parse(file)
char *file;
{
	char *fields[7];
	char *s, *t;
	int fd;

	fd = open(file, 0);
	if(fd == -1) {
		perror(file);
		return 0;
	}
	Finit(fd, (char *)0);
	while(s = Frdline(fd)) {
		if((t = strchr(s, '#')) != 0) *t = 0;
		while(*s == ' ' || *s == '\t') s++;
		if(getmfields(s, fields, 6) >= 6)
			tote(fields);
	}
	close(fd);
	return filsys != 0;
}

int uid, gid;

tote(av)
char *av[];
{
	struct label *xxx;
	struct file *p;

	p = (struct file *) calloc(sizeof(*p), 1);
	p->name = strdup(av[0]);
	uidgid(av[1]);
	p->uid = uid;
	p->gid = gid;
	p->mode = otoi(av[2]);
	xxx = Atolab(av[5]);
	if(xxx) p->lab = *xxx;
	if(atopriv(av[3])<0) fprintf(stderr, "%s bad priv\n", av[3]), exit(1);
	if(atopriv(av[4])<0) fprintf(stderr, "%s bad priv\n", av[4]), exit(1);
	p->lab.lb_t = atopriv(av[3]);
	p->lab.lb_u = atopriv(av[4]);

	if(new == 0) {
		filsys = p;
		fp = &p->lab;
	}
	else
		new->next = p;
	new = p;
}
int otoi(s)
char *s;
{
	int n = 0;

	while(*s) {
		if(*s < '0' || *s > '7')
			fprintf(stderr, "%c bad octal const\n", *s), exit(1);
		n = 8*n + *s++ - '0';
	}
	return n;
}


int nouid, nogid;

uidgid(ug)
char *ug;
{
	char *g;
	struct passwd *pwd;
	struct group *grp;
	char *strchr();
	struct passwd *getpwnam();
	struct group *getgrnam();

	if ((g = strchr(ug, ',')) != NULL)
		*g++ = '\0';
	if (*ug == '\0')
		nouid++;
	else if (isnumber(ug))
		uid = atoi(ug);
	else if ((pwd = getpwnam(ug)) != NULL)
		uid = pwd->pw_uid;
	else {
		fprintf(stderr, "unknown user id: %s\n", ug);
		exit(4);
	}
	if (g == NULL || *g == '\0')
		nogid++;
	else if (isnumber(g))
		gid = atoi(g);
	else if ((grp = getgrnam(g)) != NULL)
		gid = grp->gr_gid;
	else {
		fprintf(stderr, "unknown group id: %s\n", g);
		exit(4);
	}
	gid &= 0xffff;
	uid &= 0xffff;

	if(nogid) gid = -1;
	if(nouid) uid = -1;
}

isnumber(s)
char *s;
{
	register c;

	if(*s == '-') s++;
	while(c = *s++)
		if(!isdigit(c))
			return(0);
	return(1);
}


struct label *
Atolab(s)
char *s;
{
	int c, x;
	struct label *atolab();
	struct label *yp;


	x = L_BITS;
	while(c = *s++) {
		if(isspace(c)) continue;
		else switch(c) {
		case 'F':
		case 'R':
		case 'C': continue;
		case 'N': x = L_NO; continue;
		case 'Y': x = L_YES; continue;
		case 'U': x = L_UNDEF; continue;
		}
		s--;
		break;
	}
	yp = atolab(s);
	if(yp)yp->lb_flag = x;
	return yp;
}
char *
ids(uid, gid)
{
	static char buf[300];
	char num[20];
	struct passwd *p;
	struct group *g;

	p = getpwuid(uid);
	if(p == 0) sprintf(buf, "%d,", uid);
	else sprintf(buf, "%s,", p->pw_name);

	g = getgrgid(gid);
	if(g == 0) sprintf(num, "%d", gid);
	else strncpy(num, g->gr_name, sizeof(num)-1);

	strcat(buf, num);
	return buf;
}
