#include	"scsi.h"
#include	<getflags.h>
#include	<string.h>
#include	<errno.h>
#include	<fio.h>

#define		JDIR		"/usr/worm/jukedir"
#define		JMODE		03666
#define		NSHELF		50

int NDRIVES;		/* how many to use */

char *shelf[NSHELF];
int wr = 0;

main(argc, argv)
	char **argv;
{
	long t;

	(void)time(&t);
	srand((int)t);
	if((argc = getflags(argc, argv, "clmprsuUe:1[volid]w:1[secs]", 0)) < 0)
		usage("[ vol_id ]");
	if((argc <= 1) && !flag['u'] && !flag['r'] && !flag['e'] && !flag['l'] && !flag['w'] && !flag['p'])
		flag['m'] = flagset;
	if(flag['r'] || flag['e'] || flag['l'] || flag['p'] || flag['m'] || flag['u']){
		if(scsiinit(1))
			exit(1);
	} else {	/* mounting */
		if(scsiinit(0)){
			Fprint(1, "0\n");
			exit(0);
		}
	}
	scsiid(2);
	NDRIVES = s_ndrives();
	if(NDRIVES < 1){
		fprint(2, "cannot determine the number of drives\n");
		exit(1);
	}
/*print("using %d drives\n", NDRIVES);/**/
	jread();
	if(flag['u'])
		unload(0);
	if(flag['U'])
		unload(1);
	if(flag['r'])
		if(jreload()){
			fprint(2, "reload failed\n");
			exit(1);
		}
	if(flag['e'])
		jeject(flag['e'][0]);
	if(flag['l'])
		jload();
	if(flag['m'])
		jmpr();
	if(flag['p'])
		jpr();
	if(wr)
		jwrite();
	if(argc > 1)
		jmount(argv[1]);
	else {
		if(flag['w']){
			fprint(2, "worm mount: -w given with no vol_id\n");
			exit(1);
		}
	}
	exit(0);
}

jpr()
{
	int i;

	for(i = 0; i < NSHELF; i++)
		if(shelf[i]) Fprint(1, "%2d: %s\n", i, shelf[i]);
	Fflush(1);
}

jmpr()
{
	int i;
	register unsigned char *d;
	struct scsi_o o;

	if(s_istatus(0, &o) == 0){
		for(i = 0, d = &o.data[18]; i < NDRIVES; i++, d += 4)
			if(*d&0x80)
				Fprint(1, "drive %d: %s%c, %sline\n", i,
					shelf[(*d&0x7F)/2], (*d&1)? 'b':'a',
					(d[-2]&1)?"on":"off");
	}
	Fflush(1);
}

jeject(s)
	char *s;
{
	int i, me, drive;
	register unsigned char *d;
	struct scsi_o o;

	if(s_istatus(0, &o) == 0){
		for(i = 0, d = &o.data[18]; i < NDRIVES; i++, d += 4)
			if((*d&0x80) && (strcmp(s, shelf[me = (*d&0x7F)/2]) == 0)){
				if(s_diskeject(i))
					fprint(2, "worm mount: couldn't eject drive %d\n", i);
				shelf[me] = 0;
				wr = 1;
				return;
			}
	}
	drive = getdrive();
	for(i = 0; i < NSHELF; i++)
		if(strcmp(shelf[i], s) == 0){
			if(s_diskset(i, A, drive, 1)){
				fprint(2, "worm mount: couldn't load %s into drive 1\n", s);
				return;
			}
			if(s_diskeject(1)){
				fprint(2, "worm mount: couldn't eject %s from drive 1\n", s);
				s_diskrelease(-1, A, drive, 1);
				return;
			}
			shelf[i] = 0;
			wr = 1;
			return;
		}
	fprint(2, "worm mount: can't find %s\n", s);
	exit(1);
}

jload()
{
	int i;
	int drive, flip;
	char s[256], *side;
	struct scsi_o output;
	int ndone = 0;

	drive = getdrive();
	Fprint(1, "loading new disks\n");
loop:
	if(s_diskset(127, A, drive, 0)){
		Fprint(1, "%d new disk%s loaded\n", ndone, ndone==1? "":"s");
		return;
	}
	Fprint(1, "loading... "); Fflush(1);
	if(s_start(drive)){
		fprint(2, "worm mount: drive %d won't start\n", drive);
		return;
	}
	s_readid(drive, s);
	side = &s[strlen(s)-1];
	if((*s == 0) || ((*side != 'a')&&(*side != 'b'))){
		fprint(2, "worm mount: bad name '%s' in drive %d\n", s, drive);
		return;
	}
	flip = (*side =='a')? A:B;
	*side = 0;
	if(strcmp(s, UNLABELLED))
	for(i = 0; i < NSHELF; i++)
		if(strcmp(shelf[i], s) == 0){
			fprint(2, "worm mount: %s was listed shelf %d\n", s, i);
			shelf[i] = 0;
			break;
		}
	for(i = 0; i < NSHELF; i++)
		if(shelf[i] == 0){
			if(s_diskrelease(i, flip, drive, 1)){
				fprint(2, "worm mount: %s to shelf %d failed\n", s, i);
				return;
			}
			shelf[i] = strdup(s);
			print("%s loaded to shelf %d\n", s, i);
			wr = 1;
			ndone++;
			goto loop;
		}
	fprint(2, "worm mount: no space left for %s\n", s);
}

jreload()
{
	int i, dest, flip, try;
	int drive = 1;
	char s[256], *side;
	struct scsi_o output;
	int todo[NSHELF];

	flag['l'] = flagset;
	for(i = 0; i < NSHELF; i++)
		shelf[i] = 0, todo[i] = 0;
	drive = getdrive();
	if(s_istatus(drive, &output))
		return(1);
	for(i = 0; i < NSHELF; i++)
		if((output.data[48+i]&0x80) && (output.data[48+i]&0x60)){
			shelf[i] = "";
			todo[i] = 1;
		}
	for(i = 0, try = 0; i < NSHELF; i++, try++){
		if(!todo[i]) continue;
		if(s_diskset(i, A, drive, 0))
			continue;
		Fprint(1, "doing shelf %d: ", i); Fflush(1);
		if(s_start(drive)){
			fprint(2, "worm mount: drive %d won't start\n", drive);
			exit(1);
		}
		s_readid(drive, s);
		if(strcmp(s, UNLABELLED) == 0)
			strcat(s, "a");
		side = &s[strlen(s)-1];
		if((*s == 0) || ((*side != 'a')&&(*side != 'b'))){
			fprint(2, "worm mount: bad volid '%s', %s\n", s, try?"ignoring":"retrying");
			if(try == 0){
				s_diskrelease(i, B, drive, 1);
				i--;
			}
			continue;
		}
	sleep(1);
		if(flag['c']){
			for(dest = 0; dest < i; dest++)
				if(shelf[dest] == 0) break;
		} else if(flag['s']){
			while(shelf[dest = nrand(NSHELF)] && (i != dest))
				;
		} else
			dest = i;
		flip = (*side == 'a')? A:B;
		if(s_diskrelease(dest, flip, drive, 1)){
			fprint(2, "worm mount: %s to shelf %d failed\n", s, dest);				exit(1);
		}
		*side = 0;
		Fprint(1, "%s", s);
		if(dest != i)
			Fprint(1, " -> shelf %d", dest);
		Fprint(1, "\n"); Fflush(1);
sleep(1);
		shelf[i] = 0;
		shelf[dest] = strdup(s);
		wr = 1;
		try = -1;
	}
	return(0);
}

#define	STEP	5

jmount(vol)
	char *vol;
{
	int limit, i, sh, flip;
	unsigned char *d;
	char *ss;
	struct scsi_o o;

	ss = &vol[strlen(vol)-1];
	if((*ss != 'a') && (*ss != 'b')){
		fprint(2, "worm mount: mount name (%s) must end in [ab]\n", vol);
		exit(1);
	}
	flip = (*ss == 'a')? A:B;
	*ss = 0;
	for(sh = 0; sh < NSHELF; sh++)
		if(shelf[sh] && (strcmp(vol, shelf[sh]) == 0))
			break;
	if(sh == NSHELF){
		fprint(2, "worm mount: no disk with vol_id '%s'\n", vol);
		exit(1);
	}
	limit = flag['w']? atoi(flag['w'][0]) : 9999999;
	for(; limit > -STEP; limit -= STEP, sleep(STEP)){
		if(s_istatus(0, &o))
			continue;
		for(i = 0, d = &o.data[18]; i < NDRIVES; i++, d += 4)
			if((*d&0x80) && (strcmp(shelf[(*d&0x7F)/2], vol) == 0)){
				if(((*d&1) == flip) && (d[-1] != 0x80)){
					Fprint(1, "%d\n", i);
					return;
				}
				s_diskrelease(-1, A, i, 0);
			}
		for(i = 0, d = &o.data[18]; i < NDRIVES; i++, d += 4)
			if((d[-2]&0x40) == 0){
				if(s_diskset(sh, flip, i, 1))
					exit(1);
/*----->*/			prompt(i);
				Fprint(1, "%d\n", i);
				return;
			}
		for(i = 0, d = &o.data[18]; i < NDRIVES; i++, d += 4)
			if((d[-2]&1) == 0){
				s_diskrelease(-1, A, i, 0);
			}
	}
}

jread()
{
	int fd, errs = 0;
	int ntries;
	register char *s, *ss;

	for(ntries = 0; ntries < 10; ntries++){
		if((fd = open(JDIR, 0)) >= 0)
			break;
		if(errno != ECONC)
			break;
		sleep(1);
	}
	if(fd < 0){
		perror(JDIR);
		exit(1);
	}
	Finit(fd, (char *)0);
	while(s = Frdline(fd)){
		if(ss = strchr(s, ' ')){
			*ss++ = 0;
			shelf[atoi(s)] = strdup(ss);
		} else {
			Fprint(2, "bad line: '%s'\n", s);
			errs++;
		}
	}
	if(errs){
		fprint(2, "%d errors\n", errs);
		exit(1);
	}
	close(fd);
}

jwrite()
{
	int ntries, fd, i;

	for(ntries = 0; ntries < 10; ntries++){
		if((fd = creat(JDIR, JMODE)) >= 0)
			break;
		if(errno != ECONC)
			break;
		sleep(1);
	}
	if(fd < 0){
		perror(JDIR);
		exit(1);
	}
	Finit(fd, (char *)0);
	for(i = 0; i < NSHELF; i++)
		if(shelf[i])
			Fprint(fd, "%d %s\n", i, shelf[i]);
	Fflush(fd);
	close(fd);
}

unload(force)
{
	int i, ret = -1;
	unsigned char *d;
	struct scsi_o output;

	if(s_istatus(0, &output))
		return(1);
	for(i = 0; i < NDRIVES; i++){
		d = &output.data[16+4*i];
		if((*d&0x40) || force){
			if(((*d&1) == 0) || force)
				s_diskrelease(-1, A, i, 0);
		} else
			ret = i;
	}
	return(ret);
}

getdrive()
{
	int i;

	if((i = unload(0)) < 0)
		i = NDRIVES;
	return(i);
}

#include	<sys/types.h>
#include	<sys/udaioc.h>

prompt(i)
{
	char buf[1024];
	int fd;

	sprint(buf, "/dev/worm%d", i);
	fd = open(buf, 0);
	read(fd, buf, sizeof buf);
	ioctl(fd, UIOSPDW);
	close(fd);
	sleep(1);
}
