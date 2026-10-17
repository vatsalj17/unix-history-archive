#include	"stdio.h"
#include	"stat.h"
#include	"time.h"
#include	"dir.h"

struct statb sbuf;
struct dir dirb;
int low;
long comp;

main(argc,argv)
	int argc;
	char **argv;
{

	char newlist[64], newdate[64], command[65], *sptr;
	char base[25];
	int line, list, n, dfd;
	long atol();

	/* setup pathname for .newlist and .newdate */

	sptr = logdir();
	strcpy(base,sptr);
	strcpy(newlist,sptr);
	strcpy(newdate,newlist);
	strcat(newlist,"/.newlist");
	strcat(newdate,"/.newdate");

	list = fopen(newlist,"r");
	stat(newdate,&sbuf);
	comp = sbuf.i_mtime;

	argv++;
	if(--argc) {
		if(**argv == '-') {
			comp = atol(++*argv);
			if(comp < 700000L) {
				printf("bad date\n");
				exit(1);
			}
			low++;
			argc--;
			argv++;
		}
		if(argc) list = fopen(*argv,"r");
	}
	if(list == NULL) {
		printf("cannot read list\n");
		exit(1);
	}
	if(!comp) {
		printf("cannot obtain comparison date\n");
		exit(1);
	}

	line = 0;
	chdir(base);
	while(fgets(command,64,list)) {
		line++;
		n = strlen(command);
		if(command[--n] != '\n') {
			printf("bad input line: %d\n",line);
			while(getc(stdin) != '\n');
			continue;
		}
		command[n] = '\0';
		if(stat(command,&sbuf) < 0) {
			printf("%s: cannot access file status\n",command);
			continue;
		}

		if((sbuf.i_mode & IFMT) == IFDIR) {
			if((dfd = open(command,0)) < 0) {
				printf("\n%s: cannot read directory\n",command);
				chdir (base);
				continue;
			}
			if(chdir(command) < 0) {
				printf("%s: cannot chdir\n",command);
				chdir(base);
				continue;
			}
			printf("%s:\n",command);
			seek(dfd,32,0);
			while(read(dfd,&dirb,16) == 16)
				if(dirb.d_ino) {
					if(stat(dirb.d_name,&sbuf) < 0)
						printf("%s: cannot access file status\n",dirb.d_name);
					else check(dirb.d_name);
				}
			printf("\n");
			chdir (base);
			close(dfd);
		}
		else check(command);
	}
	if(!low) creat(newdate,0600);
}

long
atol(s)
	register char *s;
{

	register int n;
	long value;

	for(n=0,value=0; '0' <= *s && *s <= '9'; n++)
		value = value * 10 + *s++ - '0';
	if( n != 6) return (0);
	return(value);
}

check(string)
	register char *string;
{
	register struct time *t;
	long lcomp;

	if(low) {
		t = localtime(&sbuf.i_mtime);
		lcomp = t->t_year * 10000L 
			+ (t->t_month + 1) * 100L
			+ t->t_day_month;
		if(lcomp > comp) 
			printf("%s: modified %s", string, ctime(&sbuf.i_mtime));
		return;
	}
	if(sbuf.i_mtime > comp)
		printf("%s: modified %s", string, ctime(&sbuf.i_mtime));
}
