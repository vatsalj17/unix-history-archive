#include <fio.h>
#include <libc.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/pex.h>
#include <sys/label.h>


extern char *atalla();
extern char *labtoa();
extern struct label *atolab();
extern ipcopen();

int cleanup();
char *getstring();
char *atallashow();

char pwfile[] = "/etc/pwfile";
char npwfile[] = "/etc/npwfile";
char opwfile[] = "/etc/opwfile";
char tpwfile[] = "/etc/pwXXXXXX";

int fd = -1;
int fd2 = -1;
int flgflg;
int cflg;
int dflg;
int qflg;
int pexed;
char user[30];
char *fields[10];
struct label clab, plab, olab, tlab;

#define NAME	0
#define	UPW	1
#define	APW	2
#define	PRIV	3	/* vestigal */
#define	LAB	4

/*
 * regular uses:  pwx [user]	edit UPW field (defaults to logname)
 *		  pwx -q user	check identity
 * special uses:
 * 		  pwx -c user	edit (create if needed) user's fields
 *		  pwx -d user	delete
 */

main(ac, av)
char **av;
{
	int x;

	nochk(1,0);
	nochk(2,0);

	srand(getpid());
	srand(rand()^getuid());
	srand(rand()^(int)time((long*)0));
	srand(rand()^getppid());

	getflab(pwfile, &olab);
	atallainit();
	if(ac==3 && strcmp(av[1], "-q")==0) {
		qflg++;
		strncpy(user, av[2], sizeof(user));
	}
	else if(ac==3 && strcmp(av[1], "-c")==0) {
		cflg++;
		strncpy(user, av[2], sizeof(user));
	}
	else if(ac==3 && strcmp(av[1], "-d")==0) {
		dflg++;
		strncpy(user, av[2], sizeof(user));
	}
	else if(ac==2) {
		flgflg++;
		strncpy(user, av[1], sizeof(user));
	}
	else if(ac<=1) {
		flgflg++;
		getlogname(user);
	}
	else
		sorry("usage:\tpwx -[cdq] user\nor\t\tpwx [user]\n");

	getplab(&plab, &clab);
	if(T_NOCHK & ~plab.lb_t)
		sorry("Need nocheck to read pwfile\n");

	if(qflg) {
		if(perm(user))
			exit(0);
		else
			sorry("Sorry.\n");
	}

	if((T_SETPRIV & ~plab.lb_t) && !qflg)
		sorry("Need setpriv to change pwfile\n");

	if(!perm(flgflg?user:"pwedit"))
		sorry("Sorry.\n");


	fd = open(pwfile, 0);
	Finit(fd, (char*)0);

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	mktemp(tpwfile);
	onexit(cleanup);
	fd2 = creat(tpwfile, 0644);
	if(fd2 == -1)
		sorry("Cannot create new pw file\n");
	if(ioctl(fd2, FIOPX, 0)!=0)
		perror("tmp pex");
	close(creat(tpwfile, 0644));

	x = 0;
	setfields(":");
	while(fetch()) {
		if(strcmp(user, fields[NAME])==0) {
			if(dflg) continue;
			else if(cflg||flgflg) {
				if(x++) {
					print( "skip dup entry for %s\n", user);
					continue;
				} else {
					edit1();
					if(cflg) edit2();
				}
			}
		}
		stash();
	}
	if(cflg&&!x) {
		fields[NAME] = user;
		fields[UPW] = "sorry";
		fields[APW] = "";
		fields[LAB] = "0000...";
		edit1();
		edit2();
		stash();
	}
	Fflush(fd2);
	enforce();
	exit(0);
}

pexline() {
	if(pex(0, -1, (struct pexclude*)0)!=0) {
		pexed = 0;
	} else
		pexed = 1;
}

sorry(s)
char *s;
{
	print(s);
	exit(1);
}

cleanup() {
	unlink(tpwfile);
	unlink(npwfile);
	setflab(pwfile, &olab);
	fsetflab(fd2, &olab);
	unpex(0, 10);
}

edit1() {
	int i, c;
	long salt;
	char *a, *b, saltc[2];

	if(cflg) {
		print("Encrypted password: %s\n", fields[UPW]);
		if(!yesno("Change it?"))
			return;
	}
	for(;;) {
		a = strdup(getpass("New password:"));
		if(cflg!=0 && (*a==0||strcmp(a, "sorry")==0)
		    && yesno("You want an unusable password, right? ")) {
			fields[UPW] = "sorry";
			return;
		}
		if(!goodpw(a)) {
			print("Password too simple, try again\n");
			continue;
		}
		b = getpass("Again:");
		if(strcmp(a,b)!=0) {
			print("Whoops!\n");
			continue;
		}
		break;
	}

	time(&salt);
	salt += getpid();
	saltc[0] = salt & 077;
	saltc[1] = (salt>>6) & 077;
	for(i=0; i<2; i++) {
		c = saltc[i] + '.';
		if(c>'9') c += 7;
		if(c>'Z') c += 6;
		saltc[i] = c;
	}
	fields[UPW] = strdup(crypt(a, saltc));
}

yesno(s)
char *s;
{
	char *t;

	for(;;) {
		t = getstring(s);
		if(*t == 'y') return 1;
		else if(*t == 'n') return 0;
		else print("yes or no, please\n");
	}
}

char *hex[256], *oct[256];

edit2() {
	int x, i;
	struct label *lp;
	char akey[8], hkey[20], *s, *h;
top:
	print("Atalla key:");
	fields[APW] = atallashow(fields[APW]);

	print("Ceiling: %s\n", fields[LAB]);

	if(yesno("OK?"))
		return;


	if(*fields[APW] && yesno("delete existing Atalla key?"))
		fields[APW] = "";
	else if(yesno( *fields[APW] ? 
	    "change existing Atalla key?" :"assign an Atalla key?")) {
		for(;;) {
			h = hkey;
			if(yesno("should I generate one for you?")) {
				for(i=0; i<8; i++) {
					akey[i] = x = rand()&0xff;
					s = hex[x];
					*h++ = *s++;
					*h++ = *s;
				}
			} else {
				print("OK, enter 8 groups of 3 digits\n");
				for(i=0; i<8; i++) {
					do {
						print("group %d", i+1);
						s = getstring(":");
						x = lookup(s,strlen(s)+1,oct);
					} while (x<0);
					akey[i] = x;
					s = hex[x];
					*h++ = *s++;
					*h++ = *s;
				}
			}
			hkey[16] = 0;
			print("I have ");
			atallashow(hkey);
			if(yesno("OK?")) break;
		}
		fields[APW] = strdup(hkey);
	}
out:

	if(yesno("change ceiling?")) {
		for(;;) {
			s = getstring("new ceiling (clearance):");
			lp = atolab(s);
			if(lp == 0)
				print("bad label format, try again\n");
			else if(lp->lb_t || lp->lb_u)
				print("no privs here, please\n");
			else break;
		}
		s = labtoa(lp)+16;
		fields[LAB] = s;
	}
	goto top;
}


char *
atallashow(s)
char *s;
{
	int i, x;
	if(s==0 || *s == 0 || strlen(s) != 16) {
bad:
		print(" none\n");
		return "";
	}
	for(i=0; i<16; i += 2) {
		x = lookup(s+i, 2, hex);
		if(x==-1)
			goto bad;
	}
	for(i=0; i<16; i += 2) {
		x = lookup(s+i, 2, hex);
		print(" %s", oct[x]);
	}
	print("\n");
	return s;
}

atallainit() {
	char buf[30];
	int i;
	for(i=0; i<256; i++) {
		sprint(buf, "%.3o", i); oct[i] = strdup(buf);
		sprint(buf, "%.2x", i); hex[i] = strdup(buf);
	}
}

lookup(s,n,v)
char *s, **v;
{
	int i;

	for(i=0; i<256; i++)
		if(strncmp(s, v[i], n)==0)return i;
	return -1;
}


goodpw(pw)
char *pw;
{
	static char minlen[] =
	{ 
		8, 8, 6, 5, 6, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4	};
	register char *p;
	register c, flags;
	int ok, pwlen;
	char *rev(), *multi();

	ok = 0;
	pwlen = 0;
	flags = 0;
	p = pw;
	while (c = *p++) {
		pwlen++;
		if (islower(c))
			flags |= 02;
		else if (isupper(c))
			flags |= 04;
		else if (isdigit(c))
			flags |= 01;
		else
			flags |= 010;
	}
	if (pwlen == 0)
		return(0);
	if (pwlen>=minlen[flags])
		ok++;

	if((strcmp(pw, user) == 0)
	    || (strcmp(pw, rev(user)) == 0)
	    || (strcmp(pw, multi(user)) == 0)){

		ok = 0;
	}
	return ok;
}

char *rev(s)
char *s;
{
	static char t[200];
	char *p, *q;

	p = s;
	q = &t[199];
	*q-- = '\0';

	while(*q-- = *p++);

	return q+2;
}

char *multi(s)
char *s;
{

	int i, x;
	char *p=s;
	static char w[9];

	while(*p++); 
	x = p-s-1;

	if(x > 4) return "password";

	*w = '\0';

	for(i=0;i<2*x*x-(x*x*x+47*x)/6+12;i++)

		strcat(w,s);

	return w;
}

/*
 *	move temp back to pwfile file
 */
enforce() {
	/*
	 * start with  T P
	 */
	while (link(tpwfile, npwfile) < 0) {
		if (errno == EEXIST) {
 if (*getstring("Pwfile file busy... shall I wait?") == 'y')
				sleep(5);
			else
				sorry("");
		} else
			sorry("Cannot link to temp\n");
	}
	/*
	 * now  T=N P
	 */
	if(ioctl(fd, FIOPX, 0)!=0)
		sorry("Cannot pex old pwfile");

	tlab = olab;
	tlab.lb_t = tlab.lb_u = 0;
	if(fsetflab(fd, &tlab)==-1)
		sorry("Cannot unbutton old pwfile.\n");

	if(link(pwfile, opwfile) < 0)
		sorry("Cannot link to opwfile\n");
	/*
	 * now  T=N P=O
	 */
	if (unlink(pwfile) < 0)
		sorry("Cannot unlink old pwfile.\n");
	/*
	 * now  T=N O
	 */
	if (link(npwfile, pwfile) < 0)
		sorry("Cannot link new pwfile.\n");
	/*
	 * now  T=N=P O
	 */
	unlink(opwfile);
	unlink(tpwfile);
	unlink(npwfile);
	/*
	 * now  just P
	 */

	if (fsetflab(fd2, &olab)==-1)
		sorry("Cannot rebutton new pwfile.\n");
}

fetch() {
	register char *s;

	for(;;) {
		s = Frdline(fd);
		if(s == 0) return 0;
		if(getfields(s, fields, 5) == 5)
			return 1;
	}
}

stash() {
	Fprint(fd2, "%s:%s:%s::%s\n",
	    fields[NAME],
	    fields[UPW],
	    fields[APW],
	    fields[LAB]);
}

perm(name)
char *name;
{
	int x;
	struct label paranoid;

	x = pwquery(0, name, "pex");
	if(x!=-1)
		return x;
	pexline();
	if(pexed==0 && qflg==0)
		sorry("Unsafe line, sorry\n");
	fd = open(pwfile, 0);
	if(fd == -1)
		goto garble;

	if(fgetflab(fd, &paranoid) == -1
	    || (paranoid.lb_t == 0 && paranoid.lb_u == 0)) {
		goto garble;
	}

	setfields(":");
	while(fetch()) {
		if(strcmp(name, fields[NAME]) == 0) {
			if(checkpw(name, fields[UPW], fields[APW]) == 0)
				goto garble;
			close(fd);
			fd = -1;
			return 1;
		}
	}
garble:
	close(fd);
	fd = -1;
	return 0;
}

/*
 * pw in /etc/pwfile is of form
 * 	C4u3dJWbg7RCk:0123456789abcdef
 *	regular unix pw (13 chars) : atalla key 16 hex chars
 */
checkpw(id, upw, apw)
char *id, *upw, *apw;
{
	char *r;
	char *resp, challenge[65];
	char akey[8], prompt[65];

	resp = atalla(apw, challenge);
	sprint(prompt, "Password(%s:%s):", pexed?id:"UNSAFE LINE", challenge);
	if(r = getpass( prompt)) {
		if(pexed!=0&&strcmp(upw, crypt(r, upw))==0)
			return 1;
		else if(resp != 0)
			if(strncmp(resp, r, 5)==0) 
				return 1;
	}
	return 0;
}

char *
getstring(prompt)
char *prompt;
{
	char *s;
	print("%s", prompt);
	s = Frdline(0);
	return s?strdup(s):0;
}
