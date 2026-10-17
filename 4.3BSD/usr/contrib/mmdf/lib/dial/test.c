# include	<stdio.h>
# include	"ll_log.h"

char scfile[128];
struct ll_struct log = {
    "Log",
    "mktran log",
    '\0177',
    100,
    0,
    0,
    0,
};

main(argc, argv)
  int argc;
  char **argv;
    {
    register int result, word;
    extern int d_debug;
    extern int d_master, d_snseq, d_rcvseq;

    mmdf_init();
    siginit();

    if (argc > 1)
	log.ll_level = (char)(atoi (argv[1]));

    if ((result = d_opnlog(&log)) < 0)
    {
	printf("opnlog returns %d\n", result);
	exit(-1);
    }

    d_init();

    if ((result = d_tsopen("Tranfile")) < 0)
    {
	printf("d_tsopen returns %d\n", result);
	exit(-1);
    }

    d_master = 1;
    d_snseq = 3;
    d_rcvseq = 3;

    if ((result = d_scopen("Script", 0, (char **) 0)) < 0)
    {
	printf("d_scopen returns %d\n", result);
	exit(-1);
    }

    result = d_script();
    printf("d_script returns %d\n", result);


    /*  drop the connection  */
    d_drop ();
    exit(0);
}
