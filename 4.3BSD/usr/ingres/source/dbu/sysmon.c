# include	<ingres.h>
# include	<version.h>
# include	<opsys.h>
# include	<pv.h>
# include	<func.h>
# include	<signal.h>
# include	<pipes.h>
# include	<setjmp.h>
# include	<sccs.h>
# include 	<symbol.h>

/*
**	SYSTTYMON - 
**		sets up the pipe block for the sysmod function,
**		and passed the -R flags in the parmvector.
**
**		note: this is kludged in the same way that the
**			ingres monitor is-- there is no systm_mon.
**			all work is done in systm_init.
**
**	SCCSID @(#)sysmon.c	8.1	12/31/84
*/




extern		systm_mon();
extern		systm_init();
short		tTsysttymon[100];
PARM		pv[PV_MAXPC+ 1];
int		pc;
extern jmp_buf  CmReset;

struct fn_def	SysTtyMonFn =
{
	"SYSMONITOR",
	systm_mon,
	systm_init,
	NULL,
	NULL,
	0,
	tTsysttymon,
	100,
	'S',
	0
};

systm_init(argc, argv)
int	argc;
char	*argv[];
{
	pb_t		pb;
	char		**p;
	char		*pp;

	/*
	**	THIS CODE IS A CLUDGE!!!
	**
	**	all work is done in the init function 
	** 	so that the sysmonitor will have control
	**	before the sysmod function.
	*/

	setjmp(CmReset);

	/* arrange to call the sysfunc */
	for ( p = &argv[6]; (pp = *p) != NULL; p++)
	{
		pp = *p;
		if (pp[1] == 'S')
			continue;
		syssetp(PV_STR, pp);
	}
	call_setup(&pb, mdSYSFUNC, NULL);
	pb_prime(&pb, PB_REG);
	pb.pb_proc = 1;		/**** SYSFUNC MUST BE IN PROC ONE ****/
	send_off(&pb, pc, pv);
	pb_tput(PV_EOF, "", 0, &pb);
	pb_flush(&pb);

	/* wait for the response */
	readinput(&pb);
	resetp();
	exit();
}
/*
**  SYSSETP -- set parameter
**
**	Sets a parameter, to be later sent by 'call' to whomever.
**	This looks suspiciously like ctlmod/setp.c, except
**	it sets a local pc and pv, and doesnt need to know anything
**	about the Ctx struct.
**
**	Parameters:
**		type -- parameter type.
**			PV_STRING -- a string, 'len' is ignored.
**			PV_INT -- an integer, 'len' is ignored.
**		val -- the value (real value if PV_INT, pointer
**			otherwise).
**		len -- the length of the tuple.
**
**	Returns:
**		none
**
**		Adjusts pc & pv.
**
*/

syssetp(type, val, len)
register int	type;
char		*val;
register int	len;
{
	PARM	*pp;
	char	*newp;
	extern char	*need();
	char	*buf;

	/*
	**  Check the magic bounds.
	*/

	pp = &pv[pc++];

	/*
	**  Figure out the length from the type.
	*/

	switch (type)
	{
	  case PV_STR:
		len = length(val) + 1;
		newp = need(Qbuf, len);
		bmove(val, newp, len);
		buf = newp;
		pp->pv_val.pv_str = newp;
		break;
	
	  case PV_INT:
		len = sizeof (short);
		pp->pv_val.pv_int = (int) val;
		break;

	
	  default:
		syserr("syssetp: type %d", type);
	}

	/*
	**  Set up the parameter.
	*/

	pp->pv_type = type;
	pp->pv_len = len;

# ifdef xSTR1
	if tTf(0,0)
	{
		lprintf("syssetp: ");
		pr_parm(pp);
	}
# endif
}
/*
**  SYSTM_MON -- "function to implement this module"
**
**	Since we have cludged up this module to work, and hence
**	the init routine should never return, this routine just
**	syserr's.
*/

systm_mon()
{
	syserr("systm_mon");
}
/*
**  ACC_INIT, PAGEFLUSH -- dummy access method routines
**
**	Since the CM wants to do some basic access method functions,
**	we will let it.
*/

acc_init()
{
}

pageflush(x)
char	*x;
{
	return (0);
}
/*
**  CLOSECATALOG -- dummy catalog close routine.
**
**	To keep from loading access methods.
*/

closecatalog()
{
}


/*
**  GET FLAG
**
*/

getflag(argv, dest)   	/* need some data structure to hold the flags */
char	**argv;
char 	**dest;
{
	int		destctr;
	int		i;

	destctr = 0;
	for (i = 0; i <= 6; i++)
	{
		if (argv[i][0] != NULL)
			strcpy( dest[destctr++], argv[i]);
		else
			return(0);
	}
	return(0);
}
