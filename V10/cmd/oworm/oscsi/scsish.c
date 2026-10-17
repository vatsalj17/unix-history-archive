#include	"scsi.h"

main()
{
	if(scsiinit(1))
		exit(1);
	scsi_verbose = 1;
	scsiid(2);
	Ftie(0, 1);
	Fprint(1, "running\n");
	Fflush(1);
	while(yyparse())
		fprint(2, "error!!\n");
	exit(0);
}

getc()
{
	register c;

	if((c = Fgetc(0)) < 0)
		return(0);
	else
		return(c);
}

help()
{
	extern char *helpstrs[];
	int i;

	Fprint(1, "\n>> grammar\n");
	for(i = 0; helpstrs[i]; i++)
		Fprint(1, "%s\n", helpstrs[i]);
	Fprint(1, "\n");
}
