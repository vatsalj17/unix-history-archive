/* Copyright (c) 1979 Regents of the University of California */
#include <sys/types.h>
#include <stdio.h>
#include <a.out.h>
#include <ctype.h>

long	ftell();

/*
 * Strings - extract strings from an object file for whatever
 *
 * Bill Joy UCB 
 * April 22, 1978
 *
 * The algorithm is to look for sequences of "non-junk" characters
 * The variable "minlen" is the minimum length string printed.
 * This helps get rid of garbage.
 * Default minimum string length is 4 characters.
 */

struct	exec header;
struct ovlhdr	ovlbuf;

char	*infile = "Standard input";
int	oflg;
int	asdata;
long	offset;
int	minlength = 4;

main(argc, argv)
	int argc;
	char *argv[];
{
	register i;
	off_t off;

	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		register int i;
		if (argv[0][1] == 0)
			asdata++;
		else for (i = 1; argv[0][i] != 0; i++) switch (argv[0][i]) {

		case 'o':
			oflg++;
			break;

		case 'a':
			asdata++;
			break;

		default:
			if (!isdigit(argv[0][i])) {
				fprintf(stderr, "Usage: strings [ - ] [ -o ] [ -# ] [ file ... ]\n");
				exit(1);
			}
			minlength = argv[0][i] - '0';
			for (i++; isdigit(argv[0][i]); i++)
				minlength = minlength * 10 + argv[0][i] - '0';
			i--;
			break;
		}
		argc--, argv++;
	}
	do {
		if (argc > 0) {
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			infile = argv[0];
			argc--, argv++;
		}
		fseek(stdin, (long) 0, 0);
		if (asdata ||
		    fread((char *)&header, sizeof header, 1, stdin) != 1 || 
		    N_BADMAG(header)) {
			fseek(stdin, (long) 0, 0);
			find((long) 100000000L);
			continue;
		}
		off	= (long) N_TXTOFF(header) + (long) header.a_text;
		if (header.a_magic == A_MAGIC5 || header.a_magic == A_MAGIC6) {
			fread ((char *) &ovlbuf, sizeof ovlbuf, 1, stdin);
			for (i = 0; i < NOVL; i++)
				off	+= ovlbuf.ov_siz[i];
		}
		fseek(stdin, off, 0);
		find((long) header.a_data);
	} while (argc > 0);
}

find(cnt)
	long cnt;
{
	static char buf[BUFSIZ];
	register char *cp;
	register int c, cc;

	cp = buf, cc = 0;
	for (; cnt != 0; cnt--) {
		c = getc(stdin);
		if (c == '\n' || dirt(c) || cnt == 0) {
			if (cp > buf && cp[-1] == '\n')
				--cp;
			*cp++ = 0;
			if (cp > &buf[minlength]) {
				if (oflg)
					printf("%7D ", ftell(stdin) - cc - 1);
				printf("%s\n", buf);
			}
			cp = buf, cc = 0;
		} else {
			if (cp < &buf[sizeof buf - 2])
				*cp++ = c;
			cc++;
		}
		if (ferror(stdin) || feof(stdin))
			break;
	}
}

dirt(c)
	int c;
{

	switch (c) {

	case '\n':
	case '\f':
		return (0);

	case 0177:
		return (1);

	default:
		return (c > 0200 || c < ' ');
	}
}
