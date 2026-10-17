char *sccsid = "@(#)size.c	2.5";
#include	<stdio.h>
#include 	<a.out.h>
#include	<whoami.h>

/*
 *	size -- determine object size
 */

main(argc, argv)
register char **argv;
{
	struct exec buf;
	long sum;
#ifdef MENLO_OVLY
	long coresize;
	struct ovlhdr ovlbuf;
#endif MENLO_OVLY
	register gorp,i;
	FILE *f;

	if (argc == 1) {
		*argv = "a.out";
		argc++;
		--argv;
	}
	gorp = argc;
	printf("text\tdata\tbss\tdec\toct\n");
	while (--argc) {
		++argv;
		if ((f = fopen(*argv, "r")) == NULL) {
			perror(*argv);
			continue;
		}
		if (fread((char *)&buf, sizeof(buf), 1, f) != 1) {
			printf("%s:  not in object file format\n", *argv);
			fclose(f);
			continue;
		}
		if (N_BADMAG(buf)) {
			printf("%s:  not in object file format\n", *argv);
			fclose(f);
			continue;
		}
		printf("%u +\t%u +\t%u =\t", buf.a_text,buf.a_data,buf.a_bss);
		sum = (long) buf.a_text + (long) buf.a_data + (long) buf.a_bss;
		printf("%ld =\t%lo", sum, sum);
		printf("\t%s\n", *argv);
#ifdef MENLO_OVLY
		if (buf.a_magic == A_MAGIC5 || buf.a_magic == A_MAGIC6) {
			fread(&ovlbuf, sizeof ovlbuf, 1, f);
			coresize = buf.a_text;
			for (i = 0; i < NOVL; i++)
				if (ovlbuf.ov_siz[i])
					coresize += ovlbuf.ov_siz[i];
			printf("%ld total text, overlays: (", coresize);
			for (i = 0; i < NOVL; i++)
				if (ovlbuf.ov_siz[i]) {
					if (i > 0)
						printf(",");
					printf("%u", ovlbuf.ov_siz[i]);
				}
			printf(")\n");
		}
#endif MENLO_OVLY
		fclose(f);
	}
	exit(0);
}
