#include	<whoami.h>
#include	<sys/param.h>
#include	<a.out.h>
#include	<signal.h>
#include	<sys/file.h>
#include	<sysexits.h>

char	*mktemp();
int	errs;
struct	exec	exec, nexec;
#ifdef	MENLO_OVLY
struct	ovlhdr	ovlhdr, novlhdr;
#endif
#ifdef	MENLO_OVLY
#define	ISOVERLAID(x)	(((x).a_magic == A_MAGIC5) || ((x).a_magic == A_MAGIC6))
#endif
#define	ISSTRIPPED(x)	(((x).a_syms == 0) && (((x).a_flag & 1) != 0))

main(ac, av)
char **av;
{
	int nflag = 0;
	char *tname;
	register i;
	register in, out;
	off_t filesize, ovlsizes, rellen, reloffset;

	if (--ac > 1 && !strcmp(av[1], "-n")) {
		nflag++;
		av++;
	}

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	tname = mktemp("/tmp/sXXXXXX");
	while (*++av)	{
		rellen = (off_t) 0;
		reloffset = (off_t) 0;
		ovlsizes = (off_t) 0;
		filesize = (off_t) 0;
		if ((out = creat(tname, 0600)) < 0) {
			printf("Cannot create temporary file for %s\n", *av);
			exit(EX_TEMPFAIL);
		}
		if ((in = open(*av, FATT_RDONLY)) < 0) {
			perror(*av);
			errs++;
			goto err;
		}
		if (read(in, (char *) &exec, sizeof (struct exec)) != sizeof (struct exec)) {
			printf("%s:  not in a.out format\n", *av);
			errs++;
			goto err;
		}
		nexec = exec;
		if (N_BADMAG(exec)) {
			printf("%s:  not in a.out format\n", *av);
			errs++;
			goto err;
		}
		if(ISSTRIPPED(exec)) {
			printf("%s:  already stripped\n", *av);
			errs++;
			goto err;
		}
		if (nflag) {
			nexec.a_text = 0;
			nexec.a_data = 0;
			nexec.a_bss = 0;
		}
		else {
			nexec.a_syms = 0;
			nexec.a_flag |= 1;
		}
		if (copyout(*av, out, (char *) &nexec, sizeof (struct exec)) < 0)
			goto err;
		else
			filesize += (off_t) sizeof (struct exec);
#ifdef	MENLO_OVLY
		if (ISOVERLAID(nexec)) {
			if (read(in, (char *) &ovlhdr, sizeof (ovlhdr)) != sizeof (ovlhdr)) {
				perror(*av);
				errs++;
				goto err;
			}
			if (nflag) {
				novlhdr.max_ovl = 0;
				for (i = 0; i < NOVL; i++)
					novlhdr.ov_siz[i] = 0;
			}
			else
				novlhdr = ovlhdr;
			if (copyout(*av, out, (char *) &novlhdr, sizeof (struct ovlhdr)) < 0)
				goto err;
			else
				filesize += (off_t) sizeof (struct ovlhdr);
		}
#endif
		if (!nflag) {
			if (copy(*av, in, out, (off_t) exec.a_text) < 0)
				goto err;
			else
				filesize += (off_t) exec.a_text;
#ifdef	MENLO_OVLY
			if (ISOVERLAID(nexec))
				for (i = 0; i < NOVL; i++)
					ovlsizes += ovlhdr.ov_siz[i];
				if (copy(*av, in, out, ovlsizes) < 0)
					goto err;
				else
					filesize += ovlsizes;
#endif
			if (copy(*av, in, out, (off_t) exec.a_data) < 0)
				goto err;
			else
				filesize += (off_t) exec.a_data;
		}
		else
			{
			reloffset = N_TXTOFF(exec) + exec.a_text + exec.a_data;
#ifdef	MENLO_JCL
			if (ISOVERLAID(nexec))
				for (i = 0; i < NOVL; i++)
					reloffset += (off_t) ovlhdr.ov_siz[i];
#endif
			if (exec.a_flag & 1)
				rellen = (off_t) 0;
			else
				rellen = (off_t) exec.a_text + (off_t) exec.a_data;
			lseek(in, reloffset + rellen, FSEEK_ABSOLUTE);
			if (copy(*av, in, out, (off_t) exec.a_syms) < 0)
				goto err;
			else
				filesize += (off_t) exec.a_syms;
		}
		(void) close(in);
		(void) close(out);
		if ((out = creat(*av, 0666)) < 0) {
			printf("%s:  cannot recreate\n", *av);
			errs++;
			goto err;
		}
		if ((in = open(tname, FATT_RDONLY)) < 0) {
			perror(tname);
			errs++;
			goto err;
		}
		else
			if (copy(*av, in, out, filesize) < 0) {
				printf("I/o error in rewriting %s\n", *av);
				errs++;
				goto err;
			}
err:
		(void) close(in);
		(void) close(out);
		(void) unlink(tname);
	}
	exit(errs);
}

copy(name, fromfd, tofd, size)
char *name;
off_t size;
{
	register s, n;
	char buf[BSIZE];

	while (size) {
		s = sizeof (buf);
		if (size < (off_t) sizeof (buf))
			s = (int) size;
		n = read(fromfd, buf, s);
		if (n != s) {
			printf("%s:  unexpected eof\n", name);
			errs++;
			return (-1);
		}
		n = write(tofd, buf, (u_short) s);
		if (n != s) {
			perror(name);
			errs++;
			return (-1);
		}
		size -= (off_t) s;
	}
	return(0);
}

copyout (name, tofd, buf, nbytes)
char *name, *buf;
u_short nbytes;
{
	if (write(tofd, buf, nbytes) != nbytes) {
		perror(name);
		errs++;
		return (-1);
	}
	else
		return (0);
}
