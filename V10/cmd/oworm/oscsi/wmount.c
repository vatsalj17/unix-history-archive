#include	"scsi.h"
#include	<getflags.h>
#include	<string.h>
#include	<errno.h>
#include	<fio.h>

#define		JDIR		"/usr/worm/jukedir"
#define		JMODE		03666
#define		NSHELF		50

#define		NDRIVES		2		/* how many to use */

char *shelf[NSHELF];
int wr = 0;

main(argc, argv)
	char **argv;
{
	if((argc = getflags(argc, argv, "pcre:1[volid]lw:1[secs]", 0)) < 0)
		usage("[ vol_id ]");
	if((argc <= 1) && !flag['r'] && !flag['e'] && !flag['l'] && !flag['w'])
		flag['p'] = flagset;
	if(flag['r'] || flag['e'] || flag['l'] || flag['p']){
		if(scsiinit(1))
			exit(1);
	} else {	/* mounting */
		if(scsiinit(0)){
			Fprint(1, "0\n");
			exit(0);
		}
	}
	scsiid(2);
	jread();
	if(flag['r'])
		jreload();
	if(flag['e'])
		jeject(flag['e'][0]);
	if(flag['l'])
		jload();
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
	register unsigned char *d;
	struct scsi_o o;

	if(s_istatus(0, &o) == 0){
		for(i = 0, d = &o.data[18]; i < NDRIVES; i++, d += 4)
			if(*d&0x80)
				Fprint(1, "drive %d: %s%c, %sline\n", i,
					shelf[(*d&0x7F)/2], (*d&1)? 'b':'a',
					(d[-2]&1)?"on":"off");
	}
	for(i = 0; i < NSHELF; i++)
		if(shelf[i]) Fprint(1, "%2d: %s\n", i, shelf[i]);
	Fflush(1);
}

jeject(s)
	char *s;
{
	int i, me;
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
	for(i = 0; i < NSHELF; i++)
		if(strcmp(shelf[i], s) == 0){
			if(s_diskset(i, A, 1, 1)){
				fprint(2, "worm mount: couldn't load %s into drive 1\n", s);
				return;
			}
			if(s_diskeject(1)){
				fprint(2, "worm mount: couldn't eject %s from drive 1\n", s);
				s_diskrelease(-1, A, 1, 1);
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
	int drive = 1, flip;
	char s[256], *side;
	struct scsi_o output;
	int ndone = 0;

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
	if(s_read(drive, 1, 1, &output)){
		fprint(2, "worm mount: read id failed\n", drive);
		return;
	}
	strcpy(s, (char *)&output.data[42]);
	side = &s[strlen(s)-1];
	if((*s == 0) || ((*side != 'a')&&(*side != 'b'))){
		fprint(2, "worm mount: bad name '%s' in drive %d\n", s, drive);
		return;
	}
	flip = (*side =='a')? A:B;
	*side = 0;
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

	flag['l'] = flagset;
	for(i = 0; i < NSHELF; i++)
		shelf[i] = 0;
	for(i = 0; i < NDRIVES; i++)
		s_diskrelease(-1, A, i, 0);	/* release disks loaded */
	for(i = 0, try = 0; i < 50; i++, try++){
		if(s_diskset(i, A, drive, 0))
			continue;
		Fprint(1, "doing shelf %d: ", i); Fflush(1);
		if(s_start(drive)){
			fprint(2, "worm mount: drive %d won't start\n", drive);
			exit(1);
		}
		if(s_read(drive, 1, 1, &output)){
			fprint(2, "worm mount: read id failed\n", drive);
			exit(1);
		}
		strcpy(s, (char *)&output.data[42]);
		side = &s[strlen(s)-1];
		if((*s == 0) || ((*side != 'a')&&(*side != 'b'))){
			fprint(2, "worm mount: bad volid '%s', %s\n", s, try?"ignoring":"retrying");
			if(try == 0){
				s_diskrelease(i, B, drive, 1);
				i--;
			}
			continue;
		}
		if(flag['c']){
			for(dest = 0; dest < i; dest++)
				if(shelf[dest] == 0) break;
		} else
			dest = i;
		flip = (*side == 'a')? A:B;
		if(s_diskrelease(dest, flip, drive, 1)){
			fprint(2, "worm mount: %s to shelf %d failed\n", s, dest);				exit(1);
		}
		*side = 0;
		Fprint(1, "%s\n", s); Fflush(1);
		shelf[dest] = strdup(s);
		wr = 1;
		try = -1;
	}
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
