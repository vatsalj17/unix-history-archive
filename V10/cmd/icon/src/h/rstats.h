/*
 * Variables for performance analysis
 */
/*
 * Number of lines and opcodes executed
 */
int	ex_n_lines;
int	ex_n_opcodes;
/*
 * Number of invokes for operators, built-ins,
 *  and Icon procedures.
 */
int	ex_n_invoke;
int	ex_n_ipinvoke;
int	ex_n_bpinvoke;
int	ex_n_argadjust;
int	ex_n_opinvoke;
int	ex_n_mdge;
int	ex_n_stinvoke;
int	ex_n_keywd;
int	ex_n_locref;
int	ex_n_globref;
int	ex_n_statref;
int	ex_n_globpatch;
/*
 * Number of suspensions and space copied
 */
int	gde_n_esusp;
int	gde_bc_esusp;
int	gde_n_psusp;
int 	gde_bc_psusp;
int	gde_n_susp;
int	gde_bc_susp;
/*
 * Failures and returns
 */
int	gde_n_efail;
int	gde_n_pfail;
int	gde_n_fail;
int	gde_n_resume;
int	gde_n_eret;
int	gde_n_pret;
int	gde_n_bret;
/*
 * Gc junk
 */
int	gc_n_total;
int	gc_n_string;
int	gc_n_blk;
int	gc_n_coexpr;
int	gc_n_sm;
long	gc_t_last;
long	gc_t_total;
long    gc_t_start;
/*
 * Misc
 */
int	ev_n_deref;
int	ev_n_redunderef;
int	ev_n_ttderef;
int	ev_n_tsderef;
int	ev_n_tpderef;
/*
 * Conversions
 */
int	cv_n_int;
int	cv_n_rint;
int	cv_n_real;
int	cv_n_rreal;
int	cv_n_num;
int	cv_n_rnum;
int	cv_n_str;
int	cv_n_rstr;
int	cv_n_cset;
int	cv_n_rcset;
/*
 * Allocations
 */
int	al_n_total;
int	al_bc_btotal;
int	al_bc_stotal;
int	al_bc_smtotal;
int	al_n_str;
int	al_n_sm;
int	al_n_subs;
int	al_n_cset;
int	al_n_real;
int	al_n_list;
int	al_n_lstb;
int	al_n_table;
int	al_n_telem;
int	al_n_tvtbl;
int	al_n_file;
int	al_n_recd;
int	al_n_eblk;
int	al_n_estk;

#define Inc(x)	x++
#define IncSum(x,s)	(x+=s)
