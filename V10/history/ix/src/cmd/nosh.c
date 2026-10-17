/*
 * Dumb-dumb shell
 *
 * missing features: $ | < & , environment, syntax
 *
 * few features: >
 *
 * few builtins:
 *	cd dir
 *	exit [code]
 *	set -e			[exit on error (toggle)]
 *	lmask xyz command...	[run command with given lmask]
 * command syntax:
 *	 (B* ( ( > B* N* ) | N* )  )*
 * state [1]     [2]  [3]  [4]
 *   where B means blank, > means >, N means nonblank
 */
enum states { ANTEARG=1, ANTEREDIR, INREDIR, INARG };
#define QUOTING

#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/label.h>
#define ALLPRIV (T_LOG|T_UAREA|T_EXTERN|T_NOCHK|T_SETLIC|T_SETPRIV)
#define LINELEN	1024

char *prompt = (char *)0;
char line[LINELEN];
char *xv[LINELEN/2];

jmp_buf env;
int trap();
int sig;
int input = 0;
int eflag = 0;
int xflag = 0;

char *strcat();
void exit();

main(ac, av)
char **av;
{
	auth(ac, av);

	setjmp(env);
	signal(SIGQUIT, trap);
	signal(SIGINT, trap);

	for(;;) {
		enum states state;
		char *redir[3];		/* redir[0] not used */
		char *s;
		int fd;
		int xc;
		int r;

		sig = 0;
		chitchat(prompt);
		r = readline(line);
		if(r == 0) break;
		if(r == -1) syntax("can't read line\n");

		redir[1] = redir[2] = 0;
		xc = 0;
		state = ANTEARG;
		for(s=line; *s; s++) {
			switch(*s) {
			case '#':
				s[0] = 0;
				s[1] = 0;
				break;
			case ' ':
			case '\t':
				*s = 0;
				if(state != ANTEREDIR)
					state = ANTEARG;
				break;
			case '>':
				*s = 0;
				if(state==ANTEREDIR || state==INREDIR)
					syntax("> syntax error\n");
				fd = 1;
				if(!xc) 
					;
				else if(strcmp(xv[xc-1],"1")==0) 
					xc--;
				else if(strcmp(xv[xc-1],"2")==0) {
					fd = 2;
					xc--;
				}
				if(redir[fd] != 0)
					syntax("multiple >\n");
				state = ANTEREDIR;
				break;
			default:
#ifdef QUOTING
				if(*s == '\\') {
					char *t;
					for(t=s; *t; t++)
						t[0] = t[1];
				}
#endif QUOTING

				if(state == ANTEARG) {
					state = INARG;
					xv[xc++] = s;
				} else if(state == ANTEREDIR) {
					if(redir[fd] != 0)
						syntax("multiple >\n");
					redir[fd] = s;
					state = INREDIR;
				}
				break;
			}
		}
		if(state == ANTEREDIR)
			syntax("empty redirection\n");
		xv[xc] = 0;
		if(xflag)echo(xc, xv, redir);
		builtin(xc, xv, redir) || execute(xv[0], xv, redir, 0);
	}
	exit(0);
}
licspec(s)
char *s;
{
	int x;
	if(s == 0)
		return 0;
	x = 0;
	for(;*s;s++) switch(*s) {
	case 'g': x|= T_LOG; break;
	case 'u': x|= T_UAREA; break;
	case 'x': x|= T_EXTERN; break;
	case 'n': x|= T_NOCHK; break;
	case 'l': x|= T_SETLIC; break;
	case 'p': x|= T_SETPRIV; break;
	case '-':		 break;
	default:
		syntax("bad licence specifier\n");
	}
	return x;
}

char mess[] = "\nsig 00\n";
trap(n) {
	signal(n, trap);
	sig++;

	mess[5] = n>9? '0' + n/10: ' ';
	mess[6] = '0' + n%10;
	diag(mess);
}

echo(ac, av, redir)
char **av, **redir;
{
	int i;

	for(i=0; i<ac; i++) {
		if(i>0)chitchat(" ");
		chitchat(av[i]);
	}
	if(redir[1]) {
		ac++;
		chitchat(" >");
		chitchat(redir[1]);
	}
	if(redir[2]) {
		ac++;
		chitchat(" 2>");
		chitchat(redir[2]);
	}
	if(ac>0)
		chitchat("\n");
}

char *tooct();

builtin(ac, av, redir)
char **av, **redir;
{
	if(ac == 0) return 0;
	if(strcmp(av[0], "lmask") == 0) {
		if(ac > 2)
			execute(av[2], &av[2], redir, licspec(av[1]));
	} else
	if(strcmp(av[0], "cd") == 0) {
		if(ac == 2) {
			if(chdir(av[1]) == -1) {
				perror(av[1]);
				cexit();
			}
		} else 
			diag("usage: cd newdir\n");
	} else
	if(strcmp(av[0], "exit") == 0) {
		if(ac <= 1)
			exit(0);
		else if(ac == 2)
			exit(atoi(av[1]));
		else
			diag("usage: exit [code]\n");
	} else
	if(strcmp(av[0], "set") == 0) {
		if(ac==2 && strcmp(av[1],"-e") == 0)
			eflag = 1;
		else
		if(ac==2 && strcmp(av[1],"+e") == 0)
			eflag = 0;
		else
		if(ac==2 && strcmp(av[1],"-x") == 0)
			xflag = 1;
		else
		if(ac==2 && strcmp(av[1],"+x") == 0)
			xflag = 0;
		else
			diag("usage: set +-e (or +-x)\n");
	} else
		return 0;
	return 1;
}
execute(file, av, redir, newlic)
char *file, **av;
char **redir;
{
	struct label plab;
	int status = 0;
	int pid;

	if(file && *file!='/' && *file!='.')
		syntax("first letter not / or .\n");
	if(prompt) {
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
	}
	pid = fork();
	if(pid == 0) {
		int i;
		for(i=1; i<=2; i++) {
			if(redir[i] != 0) {
				int fd;

				if((fd = creat(redir[i], 0644)) == -1) {
					perror(redir[i]);
					exit(1);
				}
				dup2(fd, i);
				if(fd != i)
					close(fd);
			}
		}
		if(!prompt)
			close(input);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		if(file == 0)
			exit(0);
		getplab(&plab, (struct label*)0);
		plab.lb_u &= newlic;
		if(setplab(&plab, (struct label*)0) == -1) {
			perror("setplab");
			exit(1);
		}
		execv(file, av);
		perror(file);
		exit(1);
	} else if(pid == -1) {
		perror("fork");
		cexit();
	} else if(pid > 0)
		wait(&status);
	signal(SIGINT, trap);
	signal(SIGQUIT, trap);
	if(pid > 0 && status != 0) {
		char m[60];
		int t = status;

		m[0] = 0;
		if(t & 0377) {
			strcat(m,"  t=");
			strcat(m, tooct(t));
		}
		t >>= 8;

		if(t & 0377) {
			strcat(m,"  e=");
			strcat(m, tooct(t));
		}
		strcat(m, "\n");
		chitchat(m);

		if(t && eflag)
			exit(t);
		if(status & 0377)
			cexit();
	}
}

char *
tooct(n)
{
	static char buf[4];

	buf[3] = 0;
	buf[2] = (n&7) + '0'; n >>= 3;
	buf[1] = (n&7) + '0'; n >>= 3;
	buf[0] = (n&3) + '0';
	return buf;
}

/*
 * the REAL reason is to make ps output look tidy...
 */
extern char **environ;
char *e[] = {"=", (char*)0};

auth(ac, av)
char **av;
{
	int i;

	for(i=4; i<128; i++) 
		close(i);
	environ = e;
	if(ac>2)
		fatal("at most one argument allowed\n");
	else if(ac == 2) {
		if(av[1][0] != '/')
			fatal("command script not absolute path name\n");
		input = open(av[1], 0);
		if(input == -1)
			fatal("cannot open script\n");
	}
	if(ac>=1 && av[0][0] != '/')
		fatal("not invoked by absolute path name\n");

	if(signal(SIGINT, SIG_DFL) != SIG_DFL)
		fatal("may not run in background\n");
	if(ac <= 1)
		setprompt();
}

char *licnam(p, mask)
char *p;
{
	struct label x;

	getplab(&x, (struct label*)0);
	mask &= x.lb_u;
	if(mask & T_LOG) *p++ = 'g';
	if(mask & T_UAREA) *p++ = 'u';
	if(mask & T_EXTERN) *p++ = 'x';
	if(mask & T_NOCHK) *p++ = 'n';
	if(mask & T_SETLIC) *p++ = 'l';
	if(mask & T_SETPRIV) *p++ = 'p';
	return p;
}
char promptb[20];
setprompt() {
	char *p;

	prompt = promptb;
	p = licnam(prompt, ALLPRIV);
	if(getuid()) {
		*p++ = '$';
		*p++ = '$';
	} else {
		*p++ = '#';
		*p++ = '#';
	}
	*p++ = ' ';
	*p = 0;
}
syntax(s)
char *s;
{
	diag(s);
	longjmp(env, 1);
}
diag(s)
char *s;
{
	chitchat(s);
	cexit();
}
chitchat(s)
char *s;
{
	if(s && *s)
		write(2, s, strlen(s));
}
fatal(s)
char *s;
{
	chitchat(s);
	exit(1);
}
cexit()
{
	if(!prompt)
		exit(1);
}
readline(s)
char *s;
{
	int i, n;

	for(i=0; i<LINELEN-1;s++,i++) {
		n = read(input, s, 1);
		if(sig) {
			cexit();
			longjmp(env, 1);
		}
		if(n < 1)
			return i==0? 0: -1;
		else if(*s == '\n') {
			*s = 0;
			return 1;
		}
	}
	return -1;
}
