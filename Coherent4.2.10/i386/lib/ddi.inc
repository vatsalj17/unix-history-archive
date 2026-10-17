/ $Header: $
/ Because the DDI/DKI processor-priority manipulation system I have chosen
/ to implement uses interrupt masks, we need access to a reasonable amount
/ of per-CPU global information...
/
/	The current abstract IPL.
/	The current mask for the 'plbase' abstract IPL.
/	The table of additional masks for abstract IPLs.
/
/ This implementation is for Coherent, uniprocessor only. To make life easier
/ for this code we have put the relevant parts of the "ddi_cpu_data" structure
/ at the front. Coherent 'as' has no structure facility, so we use some
/ nasty macros I hacked together. Wouldn't this be easier if there was a
/ .struct facility that knew about the COFF/ABI structure alignment rules?
/ $Log: $
/

/ struct defcpu {
		.struct	defer
		.member		df_tab, ptr
		.member		df_read, uchar
		.member		df_max, uchar
		.member		df_write, uchar
		.member		df_rlock, uchar
		.member		df_wlock, ptr
		.ends	defer
/ };
		.typedef	defer_t,defer

/ struct dcdata {
		.struct	dcdata
		.member		dc_cpu_id, uint
		.member		dc_base_mask, ulong
		.member		dc_int_level, uchar
		.member		dc_user_level, uchar
		.member		dc_ipl, uchar
		.member		dc_run_timeouts, uchar
		.member		dc_defint, defer_t
		.member		dc_defproc, defer_t
		.member		dc_res1, ulong
		.member		dc_res2, ulong
		.member		dc_res3, ulong
		.member		dc_res4, ulong
		.ends	dcdata
/ };
		.typedef	dcdata_t, dcdata
/ struct dgdata {
					/ member offsets
		.struct	dgdata
		.member		dg_defint, defer_t
		.member		dg_defproc, defer_t
		.member		dg_run_bufcalls, uchar
		.member		dg_run_strched, uchar
		.member		dg_run_time, uchar
		.member		dg_init_flag, uchar
		.member		dg_res1, ulong
		.member		dg_res2, ulong
		.member		dg_res3, ulong
		.member		dg_res4, ulong
		.ends	dgdata
/ };
		.typedef	dgdata_t,dgdata

/ A shorthand for the base addresses of the above global structures.

cpu		.define	__ddi_cpu_data
glob		.define	__ddi_global_data

		.extern	cpu, dcdata_t
		.extern	glob, dgdata_t

		.globl	_masktab

/ Handy information about programmable interrupt controllers

		.set	PICC, 0x20	/ PIC A command register
		.set	PICM, 0x21	/ PIC A mask register address
		.set	SPICC, 0xA0	/ PIC B command register
		.set	SPICM, 0xA1	/ PIC B mask register address

		.set	EOI_COMMAND, 0x20
