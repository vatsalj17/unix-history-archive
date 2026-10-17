# include	<stdio.h>
# include	<sys/types.h>
# include	<dir.h>
# include	<stat.h>
# include	<ctype.h>

#define	NROFF	"nroff -man"
#define	SSP	"cat -s"

# define	reg	register
# define	bool	char

# define	SYSTEM(str)	(pflag ? printf("%s\n", str) : system(str))

char		buf[BUFSIZ],
		pflag = 0,
		nflag = 0,
		wflag = 0;
char	*rindex();

main(ac, av)
int	ac;
char	*av[]; {

	reg char	*tsp, *msp, *csp, *man, *cat, *sp;
	reg FILE	*mdir, *inf;
	reg long	time;
	reg char	*sections;
	reg int		exstat = 0;
	reg bool	changed = 0;
	static struct direct	dir;
	static struct stat	sbuf;

	while (ac > 1) {
		av++;
		if (strcmp(*av, "-p") == 0)
			pflag++;
		else if (strcmp(*av, "-n") == 0)
			nflag++;
		else if (strcmp(*av, "-w") == 0)
			wflag++;
		else if (*av[0] == '-')
			goto usage;
		else
			break;
		ac--;
	}
	if (ac == 2)
		sections = *av;
	else if (ac < 2)
		sections = "12345678lnpu";
	else {
usage:
		printf("usage: catman [ -p ] [ -n ] [ -w ] [ sections ]\n");
		exit(-1);
	}
	if (wflag)
		goto whatis;
	chdir("/usr/man");
	man = "manx/xxxxxxxxxxxxxx";
	cat = "catx/xxxxxxxxxxxxxx";
	msp = &man[5];
	csp = &cat[5];
	umask(0);
	for (sp = sections; *sp; sp++) {
		man[3] = cat[3] = *sp;
		*msp = *csp = '\0';
		if ((mdir = fopen(man, "r")) == NULL) {
			fprintf(stderr, "fopen:");
			perror(man);
			exstat = 1;
			continue;
		}
		if (stat(cat, &sbuf) < 0) {
			sprintf(buf, "mkdir %s", cat);
			SYSTEM(buf);
			stat(cat, &sbuf);
		}
		if ((sbuf.st_mode & 0777) != 0777)
			chmod(cat, 0777);
		while (fread((char *) &dir, sizeof dir, 1, mdir) > 0) {
			if (dir.d_ino == 0 || dir.d_name[0] == '.')
				continue;
			/*
			 * make sure this is a man file, i.e., that it
			 * ends in .[0-9] or .[0-9][a-z]
			 */
			tsp = rindex(dir.d_name, '.');
			if (tsp == NULL)
				continue;
			switch (*++tsp)	{
				case '1':	case '2':	case '3':
				case '4':	case '5':	case '6':
				case '7':	case '8':	case 'l':
				case 'n':	case 'p':	case 'u':
					if (isalpha(*++tsp) || *tsp == '\0')
						break;
				default:
					continue;
			}

			strncpy(msp, dir.d_name, DIRSIZ);
			if ((inf = fopen(man, "r")) == NULL) {
				perror(man);
				exstat = 1;
				continue;
			}
			if (getc(inf) == '.' && getc(inf) == 's'
			    && getc(inf) == 'o') {
				fclose(inf);
				continue;
			}
			fclose(inf);
			strncpy(csp, dir.d_name, DIRSIZ);
			if (stat(cat, &sbuf) >= 0) {
				time = sbuf.st_mtime;
				stat(man, &sbuf);
				if (time >= sbuf.st_mtime)
					continue;
				unlink(cat);
			}
			sprintf(buf, "%s %s | %s > %s", NROFF, man, SSP, cat);
			SYSTEM(buf);
			changed = 1;
		}
		fclose(mdir);
	}
	if (changed && !nflag) {
whatis:
		if (pflag)
			printf("/bin/csh /usr/lib/makewhatis\n");
		else {
			execl("/bin/csh", "/bin/csh", "/usr/lib/makewhatis", 0);
			perror("/bin/csh /usr/lib/makewhatis");
			exstat = 1;
		}
	}
	exit(exstat);
}
