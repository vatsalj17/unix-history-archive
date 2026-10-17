#
/*
**  BATCH.H -- batch file declarations.
**
**	Version:
**		@(#)batch.h	8.1	12/31/84
*/



# define	BATCHSIZE	506	/* available buffer space */
# define	IDSIZE		6	/* size of file id */

struct batchbuf
{
	char	file_id[IDSIZE];	/* unique file name identifier */
	char	bbuf[BATCHSIZE];	/* buffer for batch storage */
};


struct si_doms
{
	short	rel_off;	/* offset in primary tuple */
	short	tupo_off;	/* offset in saved tuple-old */
	short	dom_size;	/* width of the domain */
				/* if zero then domain not used */
};
struct batchhd
{
	char	db_name[15];	/* data base name */
	char	rel_name[13];	/* relation name */
	char	userid[2];	/* ingres user code */
	long	num_updts;	/* actual number of tuples to be updated */
	short	mode_up;	/* type of update */
	short	tido_size;	/* width of old_tuple_id field */
	short	tupo_size;	/* width of old tuple */
	short	tupn_size;	/* width of new tuple */
	short	tidn_size;	/* width of new_tuple_id field */
	short	si_dcount;	/* number of sec. index domains affected */
	struct si_doms	si[MAXDOM+1];	/* entry for each domain with sec. index */
};



short	Batch_fp;	/* file descriptor for batch file */
short	Batch_cnt;	/* number of bytes taken from the current buffer */
short	Batch_dirty;	/* used during update to mark a dirty page */
short	Batch_lread;	/* number of bytes last read in readbatch() */
short	Batch_recovery;	/* TRUE is this is recovery, else FALSE */

extern char	*Fileset;	/* unique id of batch maker */
struct batchbuf	Batchbuf;
struct batchhd	Batchhd;

# include	<stdio.h>

FILE	*Repl_infp;
FILE	*Repl_outfp;

int	Del_cnt;
FILE	*Del_infp;
FILE	*Del_outfp;

# define	MODBATCH	"_SYSmod"
# define	MODTEMP		"_SYSnewr"
# define	ISAM_SORTED	"_SYSsort"
# define	ISAM_DESC	"_SYSdesc"
# define	ISAM_SPOOL	"_SYSspol"
# define	MOD_PREBATCH	"_SYSpreb"
# define	BTREESEC	"_SYSbsec"
# define	STEMP		"_SYSstemp"
# define	REPL_IN		"_SYSr_in"
# define	REPL_DESC	"_SYSr_desc"
# define	REPL_OUT	"_SYSr_out"
# define	DEL_IN		"_SYSd_in"
# define	DEL_DESC	"_SYSd_desc"
# define	DEL_OUT		"_SYSd_out"
