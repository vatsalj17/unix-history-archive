
/* In the kernel a label is represented by a pointer to a
   "joint" label.  jlabels are shared for permanently identical
   labels, as is required on the two ends of a pipe, or
   between a process and its core image in /proc.
   Further, to save space and comparison time, the bits of
   lattice values are always shared in slabels, on the belief
   that many labels in use at once will have identical values.
*/

struct slabel {		/* form of shared label bits */
	unsigned short	sl_refcnt;
	unsigned short	sl_hash;
	struct slabel	*sl_next;
	unsigned char	sl_bits[LABSIZ];
};
#ifndef vax
deliberate syntax error: machine dependent bitfield ordering allows 
overloading of jl_refcnt in otherwise unused priv.lb_junk
#endif

struct jlabel {		/* form of a joint label */
	union {
		struct slabel	*jl_uslabp;
		struct jlabel	*jl_ufree;
	} jl_u1;
	union {
		unsigned short	jl_ux[2];
		struct labpriv  jl_upriv;
	} jl_u2;
#	define		jl_slabp  jl_u1.jl_uslabp
#	define		jl_free   jl_u1.jl_ufree
#	define		jl_Priv		jl_u2.jl_upriv	/* the whole thing */
#	define		jl_priv		jl_u2.jl_ux[1]	/* meaningful part */
#	define		jl_refcnt	jl_u2.jl_ux[0]
#	define		jl_flag(p)   ((p)->jl_u2.jl_upriv.lp_flag)
#	define		jl_fix(p)   ((p)->jl_u2.jl_upriv.lp_fix)
#	define		jl_t(p)   ((p)->jl_u2.jl_upriv.lp_t)
#	define		jl_u(p)   ((p)->jl_u2.jl_upriv.lp_u)
};
#define jl_setflag(p,x) (p)->jl_u2.jl_upriv.lp_flag=(x)
#define jl_setfix(p,x) (p)->jl_u2.jl_upriv.lp_fix=(x)
#define jl_sett(p,x) (p)->jl_u2.jl_upriv.lp_t=(x)
#define jl_setu(p,x) (p)->jl_u2.jl_upriv.lp_u=(x)
extern struct slabel slabel[];		/* the shared label table */
extern int slabelcnt;			/* size */
extern struct jlabel jlabel[];		/* the joint label table */
extern int jlabelcnt;
extern struct slabel *slabix[];		/* hash index to slabel */
extern int slabixcnt;

extern struct jlabel *labelbot;		/* label constants */
extern struct jlabel *labeltop;
extern struct jlabel *labelyes;
extern struct jlabel *labelno;

extern struct jlabel *labCP();		/* copy flag & bits */
extern struct jlabel *labCPX();		/* copy flag & bits & privs, too */
extern struct jlabel *labMAX();
extern struct jlabel *labDUP();
extern struct jlabel *jalloc();
extern struct jlabel *jfree();

/*
 * Codes for check algorithms in sysent[].sy_check
 */
#define RCH	(1<<0)		/* R check for file descriptor arg */
#define	WCH	(1<<1)		/* W check, ditto */
#define RCHN	(1<<2)		/* R check for named file arg */
#define	WCHN	(1<<3)		/* W check for named file arg */
#define RCHD	(1<<4)		/* READ(d) check for fildes */
#define WCHD	(1<<5)		/* WRITE(d) check for fildes */
#define	TU	(1<<7)		/* in suser(), need T_UAREA */
