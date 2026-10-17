	.unixorder
	.llen	132
	.include	"as.inc"
	.include	"lib/struct.inc"
	.include	"lib/intr.inc"
	.include	"lib/ddi.inc"

IODELAY	.macro
/	jmp	.+2		/ DELAY
/	jmp	.+2		/ DELAY
	pushl	%eax
	inb	$0x61
	popl	%eax
	.endm

/
/ USTART and ESP_START map kernel stack and u area within top 4k page
/ of virtual space.
/ NDP context starts 0x100 bytes below u area.
/ See also U_OFFSET, NDP_OFFSET in uproc.h
/
	.set	USTART,0xFFFFFB00
	.set	ESP0_START,0xFFFFF300
	.set	ESP1_START,USTART

	.set	u,USTART
	.set	PSW_VAL,0x1200	/ set system IOPL to 1, enable IRQ
/	.set	PSW_VAL,0x3200	/ set system IOPL to 3, enable IRQ

/ (lgl-
/	The information contained herein is a trade secret of Mark Williams
/	Company, and  is confidential information.  It is provided  under a
/	license agreement,  and may be  copied or disclosed  only under the
/	terms of  that agreement.  Any  reproduction or disclosure  of this
/	material without the express written authorization of Mark Williams
/	Company or persuant to the license agreement is unlawful.

/	Copyright (c) 1982, 1992.
/	An unpublished work by Mark Williams Company, Chicago.
/	All rights reserved.

/	Intel 386 port and extensions
/	Copyright (c) Ciaran O'Donnell, Bievres (FRANCE), 1991
/ -lgl)
/ 
/ $Log:	k0.s,v $
/
/ Revision 1.1  93/10/29  00:56:47  nigel
/ R98 (aka 4.2 Beta) prior to removing System Global memory
/
/ Revision 2.4  93/09/02  18:10:06  nigel
/ Many edits for library and preparation for new interrupt system,
/ plus removal of much dead code and icodep cleanup.
/
/ Revision 2.3  93/07/26  13:55:44  nigel
/ Nigel's R80
/
/ Revision 1.17  92/12/08  16:43:10  root
/ ker 70
/ 
/ Revision 1.16  92/11/12  10:04:31  root
/ Ker #68
/ 
/ Revision 1.15  92/11/09  17:08:28  root
/ Just before adding vio segs.
/ 
/ Revision 1.13  92/10/06  23:47:48  root
/ Ker #64
/ 
/ Revision 1.12  92/10/06  20:45:40  root
/ Ker #63d
/ 
/ Revision 1.10  92/07/27  18:15:43  hal
/ Kernel #59
/ 
/ Revision 1.9  92/07/16  16:38:14  hal
/ Kernel #58
/ 
/ Revision 1.8  92/07/15  13:50:55  root
/ COH 4.0.0
/ 
/ Revision 1.6  92/04/03  11:05:28  hal
/ Fix missed IRQ bug.
/ Add read_t0(), read_psw(), getusd(), putusd().
/ 

///////
/ Machine language assist for
/ Intel 80386/80486 Coherent. This contains the parts
/ that are common to all machines as well as the machine-specific code
/ for the IBM PC-386

///////
/ System entry point.
/ When this code is entered, the boot program has done the following:
/	Relocate itself above where the kernel will be (e.g., 0x20600).
/	Load as.s binary; text at 0x02000, data at next paragraph (16-byte)
/		boundary at or beyond end of kernel text.
/	CS = 0x02000 <- start of kernel text ("physical" stext)
/	ES = 0x02xxx <- start of kernel data
/	SS,DS = 0x20xxx <- ....some address in boot data space....

/ Due to the way the kernel has been linked (see ld.master), symbol "stext"
/ has a value of 0xFFC0_0000, which is the start of the last 4 meg segment.
/ This value is the address in linear space once we have entered paging mode,
/ but until that time relocation arithmetic is necessary:
/
/ Before segmentation is turned on, symbols in kernel text or data space
/ must be relocated by -SBASE<<BPCSHIFT for memory reference instructions
/ to work.
///////

///
/ Start with the assembler in 16-bit mode, such as it is...
///
	.16
stext:					/ kernel code starts at stext+0x100
	.org .+0x100			/ reserve stack space
	cli				/ No interrupts, please.

/ Some systems come up with NT (Nested Task) flag in PSW undefined. 
/ Clear it NOW.

	pushfw
	popw	%ax
	andw	$0xBFFF,%ax
	pushw	%ax
	popfw

/ put up a debugging "!" on the screen.  We can still use the BIOS.
	pushw    %si                      / Save registers.
	pushw    %di

	movb	$'!', %al

	movw	$0x0007, %bx             / Page 0, white on black
	movb	$0x0E, %ah               / Write TTY.
	int	$VIDEO                   / Call video I/O in ROM.
	popw	%di
	popw	%si

	/ equipment status word to AX
	int	$0x11			/ Obtain int 11 value before printf().
	xorl	%ecx,%ecx		/ clear high 16 bits of ecx
	movw	%ax,%cx			/ esw -> cx

///
/ Although we are still running in 16-bit mode, as has trouble with the
/ addresses in the following "movl" and "lgdtl" instructions, so turn on
/ 32-bit assembly early and hand-code the prefixes.
///
	.32
	/ val11 is a long, initially zero, in the CS.
	/ copy (long)esw to val11
	.byte	PX_ADDR			/ 32-bit address
	.byte	PX_OPND			/ 32-bit operand
	movl	%ecx,%cs:[[-SBASE]<<BPCSHIFT]+val11

   					/ last use of boot block's stack
	.byte	PX_ADDR			/ 32-bit address
	.byte	PX_OPND			/ 32-bit operand
	lgdtl	%cs:[[-SBASE]<<BPCSHIFT]+gdtinit

///
/ Move between CR0 and %eax is always 32 bits, regardless of the current
/ operand/address modes.
///
	/ turn on lsbit of cr0 - Protection Enable
	mov	%cr0,%eax
	orb	$1,%al
	mov	%eax,%cr0

	/ intersegment jump (48-bit address)
	/ jumping flushes the cache...
	/ 
	.byte	PX_OPND
	ljmp	$SEG_386_II, $next

next:
	movw	$SEG_386_ID, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss
	movw	$SEG_386_UD|R_USR, %ax
	movw	%ax, %fs
	mov	$stext+0x100,%eax	/ 256 byte stack for initialization
	mov	%eax,%esp

/ Enable the A20 address line, which is normally disabled by the ROM BIOS.
/ This line is under the control of the 8042 keyboard interface controller.

	sub	%ecx, %ecx
loc0:	inb	$KBCTRL		/ Wait for 8042 input buffer to empty.
	testb	$2,%al
	loopne	loc0
	IODELAY

	movb	$0xD1, %al 	/ Request next output byte to be
	outb	$KBCTRL		/ sent to the 8042 output port.
	IODELAY

	sub	%ecx, %ecx
loc1:	inb	$KBCTRL		/ Wait for 8042 input buffer to empty.
	testb	$2, %al
	loopne	loc1
	IODELAY

	movb	$0xDF,%al	/ Enable A20 address line.
	outb	$KBDATA		/ See Page 1-44, IBM-AT Tech Ref.
	IODELAY

	sub	%ecx, %ecx
loc2:	inb	$KBCTRL		/ Wait for 8042 input buffer to empty.
	testb	$2,%al
	loopne	loc2
	IODELAY

/ A20 may not enabled for up to 400 msec.  The proper handshake is to
/ send another command - we use 0xAE, keyboard enable - to the keyboard
/ controller and wait for its input buffer to be empty.  Then A20 should
/ be enabled.

	movb	$0xAE,%al	/ Send a commnad to the keyboard controller.
	outb	$KBDATA
	IODELAY

	sub	%ecx, %ecx
loc2a:	inb	$KBCTRL		/ Wait for 8042 input buffer to empty.
	testb	$2,%al
	loopne	loc2a
	IODELAY

/ Reprogram the 8253 timer so that channel 0, 
/ which is used as the clock, interrupts at exactly
/ 100 HZ, instead of 18.2 HZ.

	movb	$0x36,%al	/ Timer 0, LSB, MSB, mode 3
	outb	$PIT+3
	IODELAY
	movb	$0x9C,%al	/ Lsb of 59659/5 = 11932
	outb	$PIT
	IODELAY
	movb	$0x2E,%al	/ Msb of 59659/5 = 11932
	outb	$PIT
	IODELAY

/ Reprogram the 1st programmable interrupt controller.
/ Its default vector table collides with iAPX 286 protection vectors.
	movb	$0x11,%al	/ ICW1 - edge, ICW4
	outb	$PIC
	IODELAY
	movb	$0x20,%al	/ ICW2 - Reserve 1st 32 vectors for 286
	outb	$PICM
	IODELAY
	movb	$0x04,%al	/ ICW3 - master level 2
	outb	$PICM
	IODELAY
	movb	$0x01,%al	/ ICW4 - 8086 mode, master.
	outb	$PICM
	IODELAY

/ NIGEL: The original code here (and related code in "i386/md.c") turned off
/ the chain bit in the first PIC by default (and at every subsequent
/ opportunity) even though all the mask bits in the slave PIC are set to off.
/ In order to support an enhanced interrupt architecture for the STREAMS and
/ DDI/DDK subsystems I want to remove the state knowledge from the code in
/ "i386/md.c" so that the chain bit is always left on.
/ In order to do this, I have modified the startup code below so that the
/ system by default allows the slave PIC to interrupt (of course, it still
/ won't interrupt unless it is enabled to; the masking I have removed was
/ totally redundant).

	movb	$0xFB,%al	/ Disable interrupts from master PIC.
	outb	$PICM		/ (except for slave PIC interrupt).

	movb	$0x11,%al	/ ICW1 - edge, ICW4
	outb	$SPIC
	IODELAY
	movb	$0x70,%al	/ ICW2 - slave starts at 0x70th interrupt
	outb	$SPICM
	IODELAY
	movb	$0x02,%al	/ ICW3 - master level 2
	outb	$SPICM
	IODELAY
	movb	$0x01,%al	/ ICW4 - 8086 mode.
	outb	$SPICM
	IODELAY
	movb	$0xFF,%al
	outb	$SPICM		/ Disable interrupts from slave PIC.
/DEBUG
	xor	%ebp, %ebp	/ Halt stack backtraces
	cli
	call	__cinit
	call	mchinit		/ C initialization
	mov	%cr0,%eax	/ Turn on paging
/ use 80000001 to allow FP
/	or	$0x80000001,%eax
/ use 80000005 to disallow FP
	or	$0x80000005,%eax
	mov	%eax,%cr0
	ljmp	$SEG_RNG0_TXT,$loc3	/ clear pipeline; jump far, direct

/
/ Ring 0 startup code
/
loc3:
	movw	$SEG_386_KD, %ax
	movw	%ax, %ds
	movw	%ax, %es		/ Need %es too, for memset () etc...
	movw	$SEG_RNG0_STK, %ax
	movw	%ax, %ss
	movl	$ESP0_START,%esp	/ Stack pointer for init
	clts				/ Clear task switched flag.

/ Call the machine setup code.
/ Call Coherent main.
/ On return, send control off to the user
/ at its entry point.

	sub	%eax, %eax	/ Load local descriptor table register.
	lldt	%ax

/	movw	$tss,%ax	/ Fix low 16 bits of tss base in gdt
/	movw	%ax,gdt+SEG_TSS+2

	/ Fix tss base in gdt
	movl	$tss,%eax
	movw	%ax,gdt+SEG_TSS+2	/ Fix bits  0..15
	rorl	$16,%eax		/ Get tss bits 16..31
	movb	%al,gdt+SEG_TSS+4	/ Fix bits 16..23
	movb	%ah,gdt+SEG_TSS+7	/ Fix bits 24..31

	movw	$SEG_TSS,%ax		/ Load task state segment register.
	ltr	%ax
	lidt	idtmap			/ Load interrupt descriptor table
	lgdt	gdtmap

/	movw	$ldt,%ax		/ Relocate ldt in gdt
/	movw	%ax,gdt+SEG_LDT+2

	/ Fix ldt base in gdt
	movl	$ldt,%eax
	movw	%ax,gdt+SEG_LDT+2	/ Fix bits  0..15
	rorl	$16,%eax		/ Get ldt bits 16..31
	movb	%al,gdt+SEG_LDT+4	/ Fix bits 16..23
	movb	%ah,gdt+SEG_LDT+7	/ Fix bits 24..31

	movw	$SEG_LDT,%ax 
	lldt	%ax

	call	i8086			/ i8086() does fixup of tss_sp0

/
/ Enter Ring 1 kernel from Ring 0
/
	pushl	$SEG_RNG1_STK		/ SS
	pushl	$ESP1_START		/ ESP
	pushl	$PSW_VAL		/ PSW
	pushl	$SEG_RNG1_TXT		/ CS
	pushl	$__xmain__		/ IP
	movw	$SEG_386_KD, %ax	/ DS, ES
	movw	%ax, %ds		/ Map data segment
	movw	%ax, %es		/ Map extra segment
		xorl	%eax, %eax
		movw	%ax, %fs
		iret			/ Go to ring 1

/
/ Start of Ring 1 kernel.
/ Need Ring 1 because interrupts are about to turn on, and all irpt gates
/ have DPL (descriptor privilege level) 1.
/
__xmain__:
	sti				/ Interrupts on, and
	call	main			/ call Coherent mainline.

/
/ Enter User mode from Ring 1 kernel
/
	pushl	$SEG_386_UD|R_USR	/ SS
	pushl	$NBPC			/ ESP
	pushfl				/ PSW
	pushl	$SEG_386_UI|R_USR	/ CS
	pushl	$0			/ IP

	cli				/ interrupts off
	movw	$SEG_386_UD|R_USR, %ax	/ DS, ES
	movw	%ax, %ds		/ Map data segment
	movw	%ax, %es		/ Map extra segment
	iret				/ Go to user state.

/
/ Here is another version of tsave, called only from the GP vector (RING 0)
/

BYPASS		.macro	addr
		cmpl	$addr, _eip(%esp)	/ trapped EIP
		jz	?tsaveok
		.endm
	
tsave0:
		pusha
		push	%ds			/ Save current state
		push	%es
		push	%fs
		push	%gs
	
		movw	$SEG_386_KD, %ax	/ Map ds
		movw	%ax, %ds
		movw	%ax, %es
		movw	$SEG_386_UD|R_USR, %ax	/ Map es
		movw	%ax, %fs

/ The following code is for helping to debug startup problems

.if	0
		BYPASS	read_cr0
		BYPASS	read_cr2
		BYPASS	read_cr3
tsave0q:
	 	mov	52(%esp),%eax		/ Print fault code.
	 	cmpb	$0x40,%al
	 	je	tsave0b			/ Skip over hardware interrupts.
	 	push	%eax
	 	call	print32
	 	pop	%ecx
	 
	 	push	$' '
	 	call	mchirp
	 	pop	%ecx
	 
	 	mov	56(%esp),%eax		/ Print eip.
	 	push	%eax
	 	call	print32
	 	pop	%ecx
	
	 	push	$' '
	 	call	mchirp
	 	pop	%ecx
	
	 	push	%esp			/ Print esp.
	 	call	print32
	 	pop	%ecx
 
?loop:		jmp	?loop

?tsaveok:

/ Note that the .endi /must/ be indented to stroke the Coherent 386
/ assembler. Another fine product from MWC like its sickly brethren.

	.endi

		icall	_trapno(%esp)		/ and call the caller

		pop	%gs			/ Restore
		pop	%fs
		pop	%es
		pop	%ds
		popa
		add	$8,%esp			/ forget err, trapno
		iret				/ Done.

///////
/ Save the environment of a process
/ envsave(p)
/ MENV *p;

/ Save the context of a process
/ consave(p)
/ MCON *p;
///////

envsave:
consave:
		mov	%edi, %ecx		/ Hide di.
		mov	4(%esp), %edi 		/ di at the MCON block.
	
		cld				/ Ensure increment.
		mov	%ecx, %eax		/ Save di
		stosl
		mov	%esi, %eax		/ Save si
		stosl
		mov	%ebx, %eax		/ Save bx
		stosl
		mov	%ebp, %eax		/ Save bp
		stosl
		mov	%esp, %eax		/ Save sp
		stosl
		mov	(%esp), %eax		/ Save ra as pc
		stosl
		movzxb	cpu+dc_ipl, %eax	/ current IPL
		stosl				/ save it
		xorl	%eax, %eax
		movw	%fs, %ax		/ save space pointer
		stosl
		mov	%ecx, %edi		/ Put di back,
		sub	%eax, %eax		/ indicate a state save and
		ret				/ return to caller.

///////
/ Restore the environment of a process.
/ envrest(p)
/ MENV *p;
///////

envrest:
		mov	4(%esp), %esi		/ Pointer to context
do_restore:
		cld
		lodsl				/ Restore di
		mov	%eax, %edi
		lodsl				/ Restore si
		mov	%eax, %ecx		/ Save for later
		lodsl				/ Restore bx
		mov	%eax, %ebx
		lodsl				/ Restore bp
		mov	%eax, %ebp
		lodsl				/ Restore sp
		mov	%eax, %esp
		lodsl				/ Restore pc
		movl	%eax, (%esp)

		lodsl				/ Restore IPL
		movb	%al, cpu+dc_ipl
		movl	_masktab(%eax,4), %eax	/ new mask

	/ send the mask out to the PICs after including the base mask info.

		orl	cpu+dc_base_mask, %eax
		outb	$PICM
		movb	%ah, %al
		outb	$SPICM

		lodsl
		movw	%ax, %fs		/ Restore space
		mov	%ecx, %esi		/ Restore si
		mov	$1, %eax		/ We are restoring
		ret				/ Return

///////
/ Restore the context of a process.
/ Called with interrupts disabled from dispatch.
/ conrest(u, o)
/ saddr_t u;
///////

conrest:
		mov	8(%esp), %esi		/ Fetch syscon offset

		cli				/ Interrupts on hold

	/ Map new u area into linear space and update paging hardware

		mov	4(%esp),%eax		/ Fetch new u area saddr_t
		orb	$SEG_SRW,%al
		mov	%eax,[PTABLE1_V<<BPCSHIFT]+UADDR

		lcall	$SEG_MMUUPD,$0		/ strobe CR3

	/ Restore context with interrupts disabled - unlike envrest (), in
	/ this case the destination %esp may be lower than the caller's.

		jmp	do_restore


/ Disable interrupts.  Previous value is returned.

sphi:
		pushf				/ Save flags
		pop	%eax			/ Return current value
		cli				/ Disable interrupts
		ret				/ And return

/ Enable interrupts.  Previous value is returned.

splo:
		pushf
		pop	%eax
		sti
		ret

/ Change interrupt flag, with no return value. Since we want to test bit 10
/ of the flags word, we test bit 2 of the second byte with a byte test.
/ (An earlier version of this routine used IRET, which is a bad idea beacuse
/ it is a) very slow, and b) dangerous).

spl:
		pushf				/ Transfer previous flags to
		pop	%eax			/ %eax by way of the stack
		testb	$0x2,5(%esp)		/ Test argument interrupt flag
		je	?cleari			/ Branch if flag was clear
		sti
		ret
?cleari:
		cli
		ret

///////
/ This dummy routine is put in vector
/ table slots that are unused. All it does is
/ return to the caller.
///////

vret:	ret

/ mmuupd() uses a call gate.
mmuupd:
		pushf
		cli
		lcall	$SEG_MMUUPD,$0	/ gates to mmuupdfR0
		popf
		ret

/ Ring 0 far mmu update.  Called via a gate.  Uses %eax.
/ Want interrupts off when we arrive since the interrupt gates
/ lead into Ring 1.
mmuupdfR0:
		mov	$PTABLE0_P<<BPCSHIFT, %eax
		mov	%eax, %cr3
		lret

/ Ring 0 near mmu update.  Called from ring 0 startup.  Uses %eax.
mmuupdnR0:
		mov	$PTABLE0_P<<BPCSHIFT, %eax
		mov	%eax, %cr3
		ret

///////
/ Get cs selector - return 0 if in kernel, CS if not in kernel.
/ This version is for resident drivers.
/ There is a different version (cs_self.s) for loadable drivers.
/ int	cs_sel();
///////

cs_sel:
	sub	%eax, %eax
	ret

/	load the 'alternate address space register' (fs)
/	with the segment reference passed as an argument. 
/	The value returned is the old value of the 'fs' register

setspace:
		xorl	%eax, %eax
		movw	%fs, %ax
		movw	4(%esp), %fs
		ret

/////////////////////////
/ From __xtrap_on__ to __xtrap_off__, GP fault and page fault will not
/ cause panic.  Normally, these two traps coming from kernel text result
/ in panic.
/////////////////////////
		.globl	__xtrap_on__
		.globl	__xtrap_break__
		.globl	__xtrap_off__
__xtrap_on__:

///////

start_copy	.macro
		movl	%esp, %edx		/ Frame pointer for copy code
		.endm

end_copy	.macro
		ret
		.endm

copy_frame	.define	%edx

///////
/ Fetch a byte from the user's data space.
/ getubd(u)
/ char *u;
///////

getubd:
		start_copy
		movl	%ss:4(copy_frame), %ecx
		movzxb	%fs:(%ecx), %eax
		end_copy

///////
/ Fetch a short from the user's data space.
/	Coherent 386 fetches a 16 bit short
/ getusd(u)
/ char *u;
///////

getusd:
		start_copy
		movl	%ss:4(copy_frame), %ecx
		movzxw	%fs:(%ecx), %eax
		end_copy

///////
/ Fetch a word from the user's data space.
/	Coherent 386 fetches a 32 bit word
/ getuwd(u)
/ char *u;
///////

getuwd:
getupd:
		start_copy
		movl	%ss:4(copy_frame), %ecx
		movl	%fs:(%ecx), %eax
		end_copy

///////
/ Store a byte into the user's data space.
/ putubd(u, w)
/ char *u;
/ int w;
///////

putubd:	
		start_copy
		movl	%ss:4(copy_frame), %ecx
		movb	%ss:8(copy_frame), %al
		movb	%al, %fs:(%ecx)
		end_copy

///////
/ Store a short into the user's data space.
/	Coherent 386 stores a 16 bit short
/ putusd(u, w)
/ char *u;
/ int w;
///////

putusd:
		start_copy
		movl	%ss:4(copy_frame), %ecx
		movw	%ss:8(copy_frame), %ax
		movw	%ax, %fs:(%ecx)
		end_copy

///////
/ Store a word into the user's data space.
/	Coherent 386 stores a 32 bit word
/ putuwd(u, w)
/ char *u;
/ int w;
///////

putuwi:
putuwd:
		start_copy
		movl	%ss:4(copy_frame), %ecx
		movl	%ss:8(copy_frame), %eax
		movl	%eax, %fs:(%ecx)
		end_copy

///////
/ Perform a block-clear of user-space memory
/ size_t umemclear (caddr_t * dest, size_t size)
//////
		.globl	umemclear
umemclear:
		start_copy

		push	%ds			/ Preserve %ds
		push	%es			/ Preserve %es (?)
		pushl	%esi			/ Preserve %esi
		pushl	%edi			/ Preserve %esi

		movw	%fs, %ax
		movw	%ax, %es		/ Dest segment

		movl	%ss:4(copy_frame), %edi	/ Dest

		movl	%ss:8(copy_frame), %ecx	/ Length
		sarl	$2, %ecx		/ in longwords.

		xorl	%eax, %eax		/ Zero-fill target
		rep stosl			/ If %ecx > 0, clear longwords

		movl	%ss:8(copy_frame), %ecx	/ Length
		andl	$3, %ecx		/ residual byte count
		rep stosb			/ If %ecx > 0, clear bytes

		movl	%ss:8(copy_frame), %eax	/ Return value

		popl	%edi			/ Restore registers
		popl	%esi
		pop	%es
		pop	%ds
		end_copy

////////
/ Block transfer "n" bytes from location
/ "k" in the system map to location "u" in the
/ user's data space. Return the number of bytes
/ transferred.
/ kucopy(k, u, n)
/ char *k;
/ char *u;
/ int n;
///////

kucopy:
		start_copy

		push	%ds			/ Preserve %ds
		push	%es			/ Preserve %es (?)
		pushl	%esi			/ Preserve %esi
		pushl	%edi			/ Preserve %esi

		movw	%fs, %ax
		movw	%ax, %es		/ Dest segment

		movl	%ss:4(copy_frame), %esi	/ Source
		movl	%ss:8(copy_frame), %edi	/ Dest

		movl	%ss:12(copy_frame), %eax / Return value
		movl	%eax, %ecx		/ Length
		sarl	$2, %ecx		/ in longwords.
		rep movsl			/ If %ecx > 0, move longwords

		movl	%eax, %ecx		/ Length
		andl	$3, %ecx		/ residual byte count
		rep movsb			/ If %ecx > 0, move bytes

		popl	%edi			/ Restore registers
		popl	%esi
		pop	%es
		pop	%ds
		end_copy


///////
/ Block copy "n" bytes from location "u" in
/ the user data space to location "k" in the system
/ data space. Return the actual number of bytes
/ moved.
/ ukcopy(u, k, n)
/ char *u;
/ char *k;
/ int n;
///////

ukcopy:
		start_copy

		push	%ds			/ Preserve %ds
		push	%es			/ Preserve %es (assume == %ds)
		pushl	%esi			/ Preserve %si
		pushl	%edi			/ Preserve %di

		movw	%fs, %ax
		movw	%ax, %ds		/ Source segment

		mov	%ss:4(copy_frame), %esi	/ Source
		mov	%ss:8(copy_frame), %edi	/ Dest

		movl	%ss:12(copy_frame), %eax / Return value
		movl	%eax, %ecx		/ Length
		sarl	$2, %ecx		/ in longwords
		rep movsl			/ If %ecx > 0, move longwords

		movl	%eax, %ecx		/ Length
		andl	$3, %ecx		/ residual byte count
		rep movsb			/ if %ecx > 0, move bytes

		popl	%edi			/ Restore registers
		popl	%esi
		pop	%es
		pop	%ds
		end_copy			/ Return



///////
/ Block copy max of "n" bytes from location "u" in
/ the user data space to location "k" in the system
/ data space. Return the actual number of bytes
/ moved or -1 on a fault.
/ ukstrncopy(u, k, n, eos)
/ char *u;
/ char *k;
/ int n;
///////
.globl ukstrncopy
ukstrncopy:
		start_copy

		push	%ds			/ Preserve %ds
		push	%es			/ Preserve %es (assume == %ds)
		pushl	%esi			/ Preserve %si
		pushl	%edi			/ Preserve %di
		pushl	%ebx			/ Preserve %ebx

		movw	%fs, %ax
		movw	%ax, %ds		/ Source segment

		mov	%ss:4(copy_frame), %esi	/ Source
		mov	%ss:8(copy_frame), %edi	/ Dest

		movl	%ss:12(copy_frame), %ecx / Length

		movl	%ss:16(copy_frame), %ebx / End of segment

		cld

?ukstrcpyloop:
		cmpl	%ebx, %esi		/ Make sure in seg still
		jae     ?uk_badaddr		/ Out of segment ... error
		lodsb
		stosb
		testb	%al,%al			/ Found a null?
		loopne	?ukstrcpyloop

		movl	%ss:12(copy_frame), %ebx
		subl	%ecx, %ebx		/ How many non-zero moved?

		rep	stosb			/ Null-pad destination

		movl	%ebx, %eax		/ Return non-zero moved
		jmp	?uk_out

?uk_badaddr:
		movl	$0xFFFFFFFF, %eax	/ Return -1
?uk_out:

		popl	%ebx
		popl	%edi			/ Restore registers
		popl	%esi
		pop	%es
		pop	%ds
		end_copy			/ Return

////////
/ Block copy "n" bytes from far location "src" in
/ an arbitrary (but valid) to location "dst" in 
/ data space. Return the actual number of bytes
/ moved.
/
/ ffcopy(src, dst, n)
/ char far *src;
/ char far *dst;
/ int n;
////////
		.globl	ffcopy
ffcopy:
		start_copy

		push	%ds			/ Preserve %ds
		push	%es			/ Preserve %es
		pushl	%esi			/ Preserve %esi
		pushl	%edi			/ Preserve %edi

		les	%ss:12(copy_frame), %edi / Dest segment, length
		lds	%ss:4(copy_frame), %esi	/ Source segment, length

		movl	%ss:20(copy_frame), %eax / Return value
		movl	%eax, %ecx		/ Length
		sarl	$2, %ecx		/ in longwords
		rep movsl			/ if %ecx > 0, move longwords

		movl	%eax, %ecx		/ Length
		andl	$3, %ecx		/ residual byte count
		rep movsb			/ if %ecx > 0, move bytes

		popl	%edi
		popl	%esi
		pop	%es
		pop	%ds
		end_copy			/ Return

////////
/ Read a byte from a selector and offset.
/
/ ffbyte(off, sel)
/ unsigned long sel;
/ unsigned long off;
////////
		.globl	ffbyte
ffbyte:
		start_copy
		lgs	%ss:4(copy_frame), %ecx	/ Source seg:offset
		movzxb	%gs:(%ecx), %eax	/ Move with zero-fill
		end_copy

////////
/ Read a (short) word from a selector and offset.
/
/ ffword(off, sel)
/ unsigned long sel;
/ unsigned long off;
////////
		.globl	ffword
ffword:
		start_copy
		lgs	%ss:4(copy_frame), %ecx	/ Source seg:offset
		movzxw	%gs:(%ecx), %eax	/ Move with zero-fill
		end_copy

////////
/ write a byte using a selector and offset.
/
/ sfbyte(off, sel, byte)
/ unsigned long sel;
/ unsigned long off;
/ int byte;
////////
		.globl	sfbyte
sfbyte:
		start_copy
		lgs	%ss:4(copy_frame), %ecx	/ Dest seg:offset
		movb	%ss:12(copy_frame), %al
		movb	%al, %gs:(%ecx)
		end_copy

////////
/ write a (short) word using a selector and offset.
/
/ sfword(off, sel, word)
/ unsigned long sel;
/ unsigned long off;
/ int word;
////////
		.globl	sfword
sfword:
		start_copy
		lgs	%ss:4(copy_frame), %ecx	/ Dest seg:offset
		movw	%ss:12(copy_frame), %ax
		movw	%ax, %gs:(%ecx)
		end_copy

///////
/ The n-element copy routines jump here with the stack untouched if they
/ detect a bounds error or page fault on a user address. The only routines
/ above that use the stack at all do so with a standard format, so we detect
/ what is on the stack and restore appropriately.
/ [ For simplicity, just assume that the pushed values will either all be
/   there or not be there at all, which is a very good assumption. ]
///////

__xtrap_break__:
		subl	%eax, %eax		/ Return 0 to indicate error
						/ condition.
		cmpl	%esp, %edx		/ Anything on stack?
		je	?no_stack

		popl	%edi			/ Restore registers
		popl	%esi
		pop	%es
		pop	%ds
?no_stack:
		end_copy			/ Return

__xtrap_off__:				/ See __xtrap_on__ above.

///////
/ Profile scaling - special multiply routine is used for speed.
/
/ pscale(a,b) is product a*b shifted right 16 bits
///////

		.globl	pscale
pscale:
		mov	4(%esp),%eax	/ fetch first argument
		mull	8(%esp)		/ unsigned multiply by second argument
		shrd	$16,%edx,%eax	/ shift 64-bit product right 16 bits
		ret

__gpfault:
/		add	$4 ,%esp			/ discard fault code
/		push	$0x0D			/ General protection
		call	tsave0
		jmp	gpfault

		.globl	__debug_ker__
trap1_ker:
		push	$0x01			/ Single step.
		call	tsave0
		jmp	__debug_ker__

__stackfault:
		call	tsave0
		jmp	stackfault

///////
/ Read the equipment description. Use
/ the "int 11" interface, so that the IBM
/ ROM will do all the details.
///////

int11:		mov	%cs:val11,%eax		/ Ask the ROM
		ret				/ to put stuff in AX

///////
/ Bootstrap.
/ Called by the keyboard driver on control-alt-del.
/ Requests the 8042 controller to initiate a processor reset.
/
/	Reference: IBM-AT Technical Reference Manual,
/			Real-time Clock/CMOS RAM [Page 1-45]
/			Keyboard controller [Page 1-40]
/			Test 3, Page 5-68.
/
/ Modified 03-14-94, Udo
/
/ Extended 94/04/30, Hal
/
/ The following should be cleaned up sometime:
/
/ A halt instruction is only valid from Ring 0.
/ It is only issued from COHERENT when we want to reboot.
/ Some motherboard configurations have glue chips that specifically
/ watch for a write of 0xFE to port 0x64 (KBCTRL) followed by a "hlt"
/ as a trigger for cold boot, so we want to do this whenever rebooting.
/
/ The right way to do this will be to make "boot:" a call gate into
/ a routine which does the i/o's, then loops between hlt and writing
/ FE to port 64 inside the gate.  Since there is not time to do this
/ now, we count on the gp trap on "hlt" to get us into Ring 0, to the
/ "halt:" entry point.
/
/ Also, we should insert code here to call a keyboard routine so as
/ to restore the keyboard to scan code set 1 if it is currently in
/ scan code set 3.
/
///////
boot:
		/ make sure that the shutdown status byte in
		/ CMOS is 0

		movb	$0x0f,%al		/ offset of shutdown status in CMOS
		outb	$0x70			/ CMOS address port
		IODELAY

		movb	$0,%al			/ set the shutdown status
		outb	$0x71			/ to 0 in CMOS
		IODELAY

halt:
		cli				/ Disable interrupts.

loc13:
		movb	$0xFE,%al		/ Issue a shutdown command
		outb	$KBCTRL			/ to the 8042 control port.
		IODELAY

		hlt				/ Halt until processor reset
		jmp	loc13


/ long _canl(l) long l;
/ This is called by the routines that
/ transform longs to and from the
/ canonical formats.

_canl:
		mov	4(%esp),%eax
		rol	$16,%eax
		ret

val11:		.long	0		/ Value obtained from int11 [in code].

aicode:
		push	$envp - aicode		/ Empty environment
		push	$argl - aicode		/ Argument list for init
		push	$fn - aicode		/ File name to exec ()
		push	%eax			/ Dummy word for exec
		movl	$59, %eax
		lcall	$0x7,$0
		jmp	.			/ exec failed, page fault

		.alignoff
		.align	2
argl:		.long	fn - aicode		/ argv[0] = "/etc/init";
		.long	a1 - aicode		/ argv[1] = "";
envp:		.long	0			/ argv[2] = NULL;

fn:		.byte	"/etc/init",0
a1:		.byte	0
aicode_end:

///////
/ Task State Segment - Coherent runs as a single protected mode 386 task.
///////

	.alignon
	.align	4
	.globl	tss_sp0		/ Use run-time fixup for tss_sp0
	.globl	tssIoMap
	.globl	tssIoEnd
tss:				/ Task State Segment.
tss_lnk:.long	0		/  0: Back link selector to TSS.
tss_sp0:.long	ESP0_START	/  4: SP for CPL 0.
tss_ss0:.long	SEG_RNG0_STK	/  8: SS for CPL 0.
tss_sp1:.long	ESP1_START	/  C: SP for CPL 1.
tss_ss1:.long	SEG_RNG1_STK	/ 10: SS for CPL 1.
tss_sp2:.long	u+NBPC		/ 14: SP for CPL 2.
tss_ss2:.long	SEG_386_KD	/ 18: SS for CPL 2.
tss_cr3:.long	PTABLE0_P<<BPCSHIFT / 1C: CR3 (PDBR)
tss_ip:	.long	0		/ 20: EIP (Entry point).
tss_psw:.long	0		/ 24: Flag word.
tss_ax:	.long	0		/ 28: Register AX.
tss_cx:	.long	0		/ 2C: Register CX.
tss_dx:	.long	0		/ 30: Register DX.
tss_bx:	.long	0		/ 34: Register BX.
tss_bp:	.long	0		/ 38: Register BP.
tss_sp:	.long	0		/ 3C: Register SP.
tss_si:	.long	0		/ 40: Register SI.
tss_di:	.long	0		/ 44: Register DI.
tss_es:	.long	0		/ 48: Register ES.
tss_cs:	.long	0		/ 4C: Register CS.
tss_ss:	.long	0		/ 50: Register SS.
tss_ds:	.long	0		/ 54: Register DS.
tss_fs:	.long	0		/ 58: Register FS.
tss_gs:	.long	0		/ 5C: Register GS.
tss_ldt:.long	SEG_LDT		/ 60: Task LDT Selector.
	.word	0		/ 64: T bit.
	.word	TSS_IOMAP_OFF	/ 66: I/O map base.
/ I/O map is part of tss.
/ Bitmap up to port address TSS_IOMAP_LEN.
/ Initialize to all 1's, meaning no I/O allowed.
/ tss + 0x68 = tssIoMap
tssIoMap:
	.long	[[TSS_IOMAP_LEN + 31] .div 32] # -1
tssIoEnd:
	.long	[[0x1000 - TSS_IOMAP_LEN] .div 32] # -1
	.long	-1
///////

/ Data.

///////
	.data
sdata:

vecs:	.long	16 # vret	/ Interrupt vector table

	.text
///////
/       Read a byte from the CMOS.  Takes one argument--the
/       CMOS address to read from as an int; returns the
/       value read as a char.
/
/	int read_cmos(int addr);

read_cmos:
	push	%esi
	push	%edi
        movb    12(%esp), %al	/ Fetch address from stack.
        outb    $CMOSA		/ Send address to CMOS.
	IODELAY
	sub	%eax, %eax	/ Zero out everything we don't want.
        inb     $CMOSD		/ Get Value from CMOS into al.
	pop	%edi
	pop	%esi
        ret                     / Return from read_cmos().

/       Write a byte to the CMOS.
/
/	void write_cmos(int addr, int data)

write_cmos:
	push	%esi
	push	%edi
        movb    12(%esp), %al	/ Fetch address from stack.
        outb    $CMOSA		/ Send address to CMOS.
	IODELAY
        movb    16(%esp), %al	/ Fetch address from stack.
        outb     $CMOSD		/ Get Value from CMOS into al.
	IODELAY
	pop	%edi
	pop	%esi
        ret                     / Return from read_cmos().

/ Read timer channel 0 into int value.  
/ Clock counts down from 11932 to 0 with each clock tick.
	.globl	read_t0
read_t0:
	pushfl
	cli
	xorl	%eax,%eax	/ Counter latch timer 0 and clear return val
	outb	$PIT+3
	IODELAY
	inb	$PIT		/ low byte of counter latch
	IODELAY
	movb	%al,%ah
	inb	$PIT		/ high byte of counter latch
	IODELAY
	xchgb	%al,%ah
	popfl
	ret

/ return current contents of cr0
	.globl	read_cr0
read_cr0:
	movl	%cr0,%eax
	ret

/ return current contents of cr2
	.globl	read_cr2
read_cr2:
	movl	%cr2,%eax
	ret

/ return current contents of cr3
	.globl	read_cr3
read_cr3:
	movl	%cr3,%eax
	ret

/ write to cr0
	.globl write_cr0
write_cr0:
	movl	4(%esp),%eax
	movl	%eax,%cr0
	ret

/////////
/
/ Debugging support.
/
/////////
	.globl	write_dr0
	.globl	write_dr1
	.globl	write_dr2
	.globl	write_dr3
	.globl	write_dr6
	.globl	write_dr7

	.globl	read_dr0
	.globl	read_dr1
	.globl	read_dr2
	.globl	read_dr3
	.globl	read_dr6
	.globl	read_dr7

/ write arg to dr0
write_dr0:
	movl	4(%esp),%eax
	movl	%eax,%dr0
	ret

/ write arg to dr1
write_dr1:
	movl	4(%esp),%eax
	movl	%eax,%dr1
	ret

/ write arg to dr2
write_dr2:
	movl	4(%esp),%eax
	movl	%eax,%dr2
	ret

/ write arg to dr3
write_dr3:
	movl	4(%esp),%eax
	movl	%eax,%dr3
	ret

/ write arg to dr6
write_dr6:
	movl	4(%esp),%eax
	movl	%eax,%dr6
	ret

/ write arg to dr7
write_dr7:
	movl	4(%esp),%eax
	movl	%eax,%dr7
	ret

read_dr0:
	movl	%dr0,%eax
	ret

read_dr1:
	movl	%dr1,%eax
	ret

read_dr2:
	movl	%dr2,%eax
	ret

read_dr3:
	movl	%dr3,%eax
	ret

read_dr6:
	movl	%dr6,%eax
	ret

read_dr7:
	movl	%dr7,%eax
	ret

/ write to the EM bit of CR0
/ this routine is a stub for the ring 0 code
/ argument is 0 or 1
/
/	void setEm(int bit)
	.globl	setEm
setEm:
	movl	4(%esp),%eax	/ fetch argument
	pushf
	cli
	pushl	%eax
	lcall	$SEG_SET_EM,$0	/ gate to setEmfR0
	/ setEmfR0 will delete 4 bytes worth of args
	popf
	ret

/ Ring 0 write to CR0 EM bit.  Called via a gate.
/ Want interrupts off when we arrive since the interrupt gates
/ lead into Ring 1.
setEmfR0:
	movb	8(%esp),%cl	/ fetch argument

	cmpb	$0,%cl
	movl	%cr0,%eax
	jz	se00
	orb	$4,%al		/ set EM bit
	andb	$0xDF,%al	/ clear NE bit
	jmp	se01
se00:
	andb	$0xFB,%al	/ clear EM bit
	orb	$0x20,%al	/ set NE bit
se01:
	mov	%eax,%cr0
	/ make 4-byte arg list disappear
	lret	$4

/ return nonzero if paging is turned on
	.globl	paging
paging:
	movl	(%esp),%eax		/ fetch return address
	cmpl	$[SBASE<<BPCSHIFT],%eax	/ is it >= unsigned FFC0_0000?
	jae	pagingMaybe
	xorl	%eax,%eax		/ if not, no paging
	ret
pagingMaybe:
	movw	%cs,%ax			/ if return addr high, cs is a selector
	cmpw	$0x58,%ax		/ selectors 58-6F are nonpaging
	jb	pagingYes
	cmpw	$0x6F,%ax		/ selectors 58-6F are nonpaging
	ja	pagingYes
	xorl	%eax,%eax		/ no paging
	ret
pagingYes:
	movl	$1,%eax
	ret

///////////////////////
/ Data tables for initial GDT, LDT, IDT, TSS.
/
/	the init gdt is in the text segment since the data segment
/	is relocated after we switch into 386 mode (when a
/	valid gdt is required)

	.alignoff
	.align	4

gdtinit:				/ used before turning on paging
	.value	gdtend-gdt-1		/ limit of gdt
					/ gdt physical addr is 0x0040_2000+gdt
	.long	[[[-SBASE]+PBASE]<<BPCSHIFT]+gdt
	.value	0
gdtmap:					/ used after paging is enabled
	.value	gdtend-gdt-1		/ limit of gdt
	.long	gdt
	.value	0

idtmap:					/ used after paging is enabled
	.value	2047
	.long	idt
	.value	0

////
/
/ Macro MEM_SEG specifies a memory segment descriptor.
/ base is 32 bits
/ limit is 20 bits
/ type is 4 bits
/ dpl is 2 bits
/ flags is the 4 high bits of byte at descriptor + 6
/   8's bit of flags is limit granularity (0:byte 1:click)
/   4's bit of flags is default op size (0:16 bit 1:32 bit)
/
////

MEM_SEG	.macro	base,limit,type,dpl,flags
	.value	limit
	.value	base
	.byte	[base] >> 16
	.byte	0x90 | [[[dpl] & 3] << 5] | [[type] & 0xF]
	.byte	[[[flags] << 4] & 0xF0] | [[[limit] >> 16] & 0xF]
	.byte	[base] >> 24
	.endm

////
/
/ Macro TSS_SEG specifies a tss segment descriptor.
/ base is 32 bits
/ limit is 20 bits
/ [type is 0x9]
/ [dpl is 0]
/ gran is limit granularity (0:byte 1:click)
/
////

TSS_SEG	.macro	base,limit,gran
	.value	limit
	.value	base
	.byte	[base] >> 16
	.byte	0x89
	.byte	[[[gran] << 7] & 0x80] | [[[limit] >> 16] & 0xF]
	.byte	[base] >> 24
	.endm

////
/
/ Macro LDT_SEG specifies a tss segment descriptor.
/ base is 32 bits
/ limit is 20 bits
/ [type is 0x2]
/ [dpl is 0]
/ gran is limit granularity (0:byte 1:click)
/
////

LDT_SEG	.macro	base,limit,gran
	.value	limit
	.value	base
	.byte	[base] >> 16
	.byte	0x82
	.byte	[[[gran] << 7] & 0x80] | [[[limit] >> 16] & 0xF]
	.byte	[base] >> 24
	.endm

////
/
/ Macro CALL_GATE specifies a call gate descriptor.
/ selector is 16 bits
/ offset is 32 bits
/ dwdcount is 5 bits
/ dpl is 2 bits
/
/ Would like the following, but can't shift offset since it's a symbol.
/	.value	offset
/	.value	selector
/	.value	0x8C00 | [[dwdcount] & 0x1F] | [[[dpl] & 3] << 13]
/	.value	[offset] >> 16
/
/ IMPORTANT!!!
/ This macro does not create a proper call gate.  
/ Count on idtinit() to swap 16-bit words at macro+2, macro+6.
/
////

CALL_GATE	.macro	selector,offset,dwdcount,dpl

	.long	offset
	.value	0x8C00 | [[dwdcount] & 0x1F] | [[[dpl] & 3] << 13]
	.value	selector
	.endm

////
/
/ "segment xxxx" below gives the value in a segment register corresponding
/ to the given descriptor.  The low 3 bits in a segment register are not
/ used in indexing into the descriptor table.
/
////

	.set	DPL_0,0
	.set	DPL_1,1
	.set	DPL_2,2
	.set	DPL_3,3

gdt:
	/ segment 0000
	.long	0,0			/ null entry

	/ segment 0008 - SEG_386_UI
	MEM_SEG	0,0x7FFFF,0xB,DPL_3,0xC

	/ segment 0010 - SEG_386_UD
	MEM_SEG	0,0x8FFFF,0x3,DPL_3,0xC

	/ segment 0018 - SEG_RNG0_TXT/SEG_386_KI
	MEM_SEG	0,0xFFFFF,0xB,DPL_0,0xC

	/ segment 0020 - SEG_386_KD
	MEM_SEG	0,0xFFFFF,0x3,DPL_1,0xC

	/ segment 0028 - SEG_286_UI
	MEM_SEG	0,0xF,0xB,DPL_3,0x8

	/ segment 0030 - SEG_286_UD
	MEM_SEG	0,0xF,0x3,DPL_3,0x8

	/ segment 0038 - SEG_TSS
	/ limit is 0x68 + (number of bytes in iomap)
	/ This allows the required filler byte beyond desired addressing.
	TSS_SEG	0xFFC00000,TSS_IOMAP_OFF+[TSS_IOMAP_LEN .div 8]+3,0

	/ segment 0040 - SEG_ROM
	MEM_SEG	0xFFFC0000,0xF,0x3,DPL_0,0x8

	/ segment 0048 - SEG_VIDEOa
	MEM_SEG	0xFFFB0000,0xF,0x3,DPL_1,0x8

	/ segment 0050 - SEG_VIDEOb
	MEM_SEG	0xFFFA0000,0xF,0x3,DPL_1,0x8

	/ segment 0058 - SEG_386_II	/ init code (text)
	MEM_SEG	0x400000+[PBASE<<BPCSHIFT],0xFFFFF,0xB,DPL_0,0xC

	/ segment 0060 - SEG_386_ID	/ init code (data)
	MEM_SEG	0x400000+[PBASE<<BPCSHIFT],0xFFFFF,0x3,DPL_0,0xC

	/ segment 0068 - SEG_286_UII
	MEM_SEG	0x400000,0xF,0xB,DPL_3,0x8

	/ segment 0070 - SEG_LDT
	LDT_SEG	0xFFC00000,0xF,0

	/ segment 0078 - SEG_RNG0_STK
	MEM_SEG	0,0xFFBFF,0x7,DPL_0,0xC

	/ segment 0080 - SEG_RNG1_TXT
	MEM_SEG	0,0xFFFFF,0xB,DPL_1,0xC

	/ segment 0088 - SEG_RNG1_STK
	MEM_SEG	0,0xFFFFE,0x7,DPL_1,0xC

	/ Call gates need idtinit() to fix them before they can be used.
	.globl	gdtFixBegin
	.globl	gdtFixEnd
gdtFixBegin:
	/ segment 0090 - SEG_MMUUPD - Call gate for mmu update
	CALL_GATE	SEG_RNG0_TXT,mmuupdfR0,0,DPL_1

	/ segment 0098 - SEG_SET_EM - Call gate for writing CR0 EM bit
	CALL_GATE	SEG_RNG0_TXT,setEmfR0,1,DPL_1
gdtFixEnd:
gdtend:

/	The two entries in the ldt are call gates whose format is somewhat
/	different from the other segment descriptors
/
/	BCS compatibility requires an LDT

ldt:
	/ ldt + 0000
	CALL_GATE	SEG_RNG1_TXT,syscall_386,1,DPL_3
/	.long	syc32				/ call gate for system call
/	.long	0xFFC0EC01

	/ ldt + 0008
/	.long	sig32				/ call gate for signal return
/	.long	0xFFC0EC01
	CALL_GATE	SEG_RNG1_TXT,signal_386,1,DPL_3
ldtend:

////
/
/ Macro IRPT_GATE specifies a call gate descriptor.
/ selector is 16 bits
/ offset is 32 bits
/ dwdcount is 5 bits
/ dpl is 2 bits
/
/ Would like the following, but can't shift offset since it's a symbol.
/	.value	offset
/	.value	selector
/	.value	0x8E00 | [[dwdcount] & 0x1F] | [[[dpl] & 3] << 13]
/	.value	[offset] >> 16
/
/ IMPORTANT!!!
/ This macro does not create a proper interrupt gate.  
/ Count on idtinit() to swap 16-bit words at macro+2, macro+6.
/
////

IRPT_GATE	.macro	selector,offset,dwdcount,dpl

	.long	offset
	.value	0x8E00 | [[dwdcount] & 0x1F] | [[[dpl] & 3] << 13]
	.value	selector
	.endm

idt:
	/ A Fault is an exception reported *before* executing the instruction
	/ in question;  a Trap is an exception reported *after* executing the
	/ instruction;  an Abort is a non-recoverable exception.

	/ Divide Error Fault.
	IRPT_GATE	SEG_RNG1_TXT,trap0,0,DPL_3

	/ Debug Exception Fault/Trap
/	IRPT_GATE	SEG_RNG0_TXT,trap1_ker,0,DPL_3	/ Ring 0
	IRPT_GATE	SEG_RNG1_TXT,trap1_usr,0,DPL_3	/ Ring 1

	/ NMI trap.
	IRPT_GATE	SEG_RNG1_TXT,trap2,0,DPL_3

	/ INT 3 Trap.
	IRPT_GATE	SEG_RNG1_TXT,trap3,0,DPL_3

	/ Overflow Trap.
	IRPT_GATE	SEG_RNG1_TXT,trap4,0,DPL_3

	/ Bounds Check Fault.
	IRPT_GATE	SEG_RNG1_TXT,trap5,0,DPL_3

	/ Invalid Opcode Fault.
	IRPT_GATE	SEG_RNG1_TXT,trap6,0,DPL_3

	/ Coprocessor Device Not Available Fault.
	IRPT_GATE	SEG_RNG1_TXT,trap7,0,DPL_3

	/ Double Fault Abort.
	IRPT_GATE	SEG_RNG1_TXT,trap8,0,DPL_3

	/ Coprocessor Segment Overrun Abort.
	IRPT_GATE	SEG_RNG1_TXT,trap9,0,DPL_3

	/ Invalid TSS Fault.
	IRPT_GATE	SEG_RNG1_TXT,trap10,0,DPL_3

	/ Segment Not Present Fault.
	IRPT_GATE	SEG_RNG1_TXT,trap11,0,DPL_3

	/ Stack Segment Fault.
	IRPT_GATE	SEG_RNG0_TXT,__stackfault,0,DPL_3

	/ General Protection Fault.
	IRPT_GATE	SEG_RNG0_TXT,__gpfault,0,DPL_3	/ Ring 0!

	/ Page Fault.
	IRPT_GATE	SEG_RNG1_TXT,trap14,0,DPL_3

	/  I don't think there is an exception number 15 - hws.
	.long	0,0

	/ Coprocessor Error Fault.
	IRPT_GATE	SEG_RNG1_TXT,trap16,0,DPL_3
	.org	.+0x78

	/ Hardware interrupt vectors IRQ0-7.
	IRPT_GATE	SEG_RNG1_TXT,dev0,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,dev1,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,dev9,0,DPL_3 / Hardware sends 9 to 2.
	IRPT_GATE	SEG_RNG1_TXT,dev3,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,dev4,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,dev5,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,dev6,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,dev7,0,DPL_3

	/ Hardware interrupt vectors IRQ8-15.
	.org	.+0x240
	IRPT_GATE	SEG_RNG1_TXT,dev8,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,dev9,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,dev10,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,dev11,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,dev12,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,dev13,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,dev14,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,dev15,0,DPL_3

	/ 286 COHERENT System call entry points.
	.org	.+0x40
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	IRPT_GATE	SEG_RNG1_TXT,syscall_286,0,DPL_3
	.long	0
idtend:
	.align	4
	.long	0,0,0,0
