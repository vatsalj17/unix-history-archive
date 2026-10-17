| Initial run-time module for C programs on a stand-alone
| SUN.  Commented by Bill Nowicki March 1982
| Note that because of Vaughan's ddtinit hack, you must have
| a dummy routine called ddtinit for this to work.
| Modified the trap #14 to be a call to exit so file cleanup is done
| if the fio lib is used - Mike Nielsen, 23 July 82

	.data
	.text
	.globl	_start
	.globl	__end
_start:	jra	__st1
__end:	.long	_end+4
__st1:	jsr	ddtinit
	jsr	main
    	movl	#0,sp@-
	jsr 	exit
    	addql	#4,sp
	jra	__st1
