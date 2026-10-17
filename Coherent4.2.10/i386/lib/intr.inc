/ $Header: $

/ Here we define some structures where information about interrupt disposition
/ is stored. Assembly-language code deals with dispatching the interrupts
/ based on this data, while C-language code writes into this structure. This
/ structure and the C version in <kernel/intr.h> had better stay in synch.
/ $Log: $
/

/ struct interrupt_control {
		.struct	intr_info
		.member		_int_mask, ulong
		.member		_int_func, ulong
		.member		_int_arg, int
		.member		_int_count, int
		.member		_int_stats, ulong
		.member		__reserved1, ulong
		.member		__reserved2, ulong
		.member		__reserved3, ulong
		.ends	intr_info
/ };
		.typedef	intr_t, intr_info

/ struct stack_frame {
		.struct	stack_info
		.member		_gs, ushort
		.member		__pad1, ushort
		.member		_fs, ushort
		.member		__pad2, ushort
		.member		_es, ushort
		.member		__pad3, ushort
		.member		_ds, ushort
		.member		__pad4, ushort
		.member		_edi, ulong
		.member		_esi, ulong
		.member		_ebp, ulong
		.member		_esp, ulong
		.member		_ebx, ulong
		.member		_edx, ulong
		.member		_ecx, ulong
		.member		_eax, ulong
		.member		_trapno, ulong
		.member		_err, ulong
		.member		_eip, ulong
		.member		_cs, ushort
		.member		__pad5, ushort
		.member		_eflags, ulong
		.member		_uesp, ulong
		.member		_ss, ushort
		.member		__pad6, ushort
		.ends	stack_info
/ };
		.typedef	stack_t, stack_info

ints		.define	__interrupts

		.extern	ints, intr_t
