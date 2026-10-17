	.unixorder
	.llen 132
	.globl iscyrix
	.globl cx_r
	.globl cx_w
	.globl cx_invalidate_cache
	.globl cx_start_cache

/////////////////////////////////////////////////////////////////////
//	int iscyrix(void)
//	Determines if Cyrix CPU is present.  Cyrix CPU's do not
//	change flags where flags change in an undefined manner
//	on other CPU's.
//
//	This is Cyrix's code, and Hal made me put it in although
//	I think it is "less than desireable" - Louis
//
//	Returns: ax == 1 if Cyrix, ax == 0 if not
//
iscyrix:
	xor	%ax,%ax			/ Clear %ax
	sahf				/ Clear flags
	movw	$0x5,%ax		 
	movw	$0x2,%bx
	divb	%bl			/ Do an operation that does not
					/ change the flags.
	lahf				/ Get the flags.
	cmpb	$0x2,%ah		/ Check for a change.
	jne	not_cyrix		/ If changed, not cyrix.
	mov	$0x1,%eax
	jmp	iscyrix_done
not_cyrix:
	mov	$0x0,%eax
iscyrix_done:
	ret

////////////////////////////////////////////////////////////////////
// unsigned char cx_r(unsigned char regnum)
// Read a Cyrix control register
cx_r:
	pushl	%ebp
	movl	%esp,%ebp
	pushfl
	cli
	movb	8(%ebp),%al
	outb	$0x22			// Select register
	inb	$0x23			// Read byte
	andl	$0x000000FF,%eax	// Thank you, MWC cc.
	popfl
	movl	%ebp,%esp
	popl	%ebp
	ret

/////////////////////////////////////////////////////////////////////
// void cx_w(unsigned char index, unsigned char value)
// Write a Cyrix control register
cx_w:
	pushl	%ebp
	movl	%esp,%ebp
	pushfl
	cli
	movb	8(%ebp),%al		// Select register
	outb	$0x22
	movb	12(%ebp),%al		// Write value
	outb	$0x23
	popfl
	movl	%ebp,%esp
	popl	%ebp
	ret

cx_invalidate_cache: // Interrupts *must* be disabled!!!

	/-- Turn off caching
	movl	%cr0,%eax
	orl	$0x40000000,%eax	// disable bit
	movl	%eax,%cr0

	.byte 0x0f,0x08		/INVD -- invalidate cache

	jmp	.+2		// Flush prefetch queue

	ret

cx_start_cache: // Interrupts *must* be disabled!!!
	movl	%cr0,%eax
	andl	$0xBFFFFFFF,%eax	// enable bit
	movl	%eax,%cr0
	ret
