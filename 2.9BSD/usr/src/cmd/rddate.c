#include <stdio.h>
#include <sys/param.h>
#include <sys/ino.h>
#include <dumprestor.h>
struct idates dat;

main() {
	int f;
	char *ctime();
	long tim;

	if ((f = open("/etc/ddate",0)) < 0) {
		printf("Can't open /etc/ddate.\n");
		exit(1);
		}
	while (read(f,(char *)&dat,sizeof(dat)) > 0 ) {
		tim = dat.id_ddate;
		printf("%5s, Level %c, %s",dat.id_name ,dat.id_incno,
			ctime(&tim) );
		}
}
