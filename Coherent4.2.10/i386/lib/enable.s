/ $Header: $
		.unixorder

/ Cheap interrupt-enable function for use in STREAMS, DDI/DKI.
/	void __CHEAP_ENABLE_INTS (void)
/ Protyped in <sys/inline.h>
/ $Log: $
		.text
		.globl	__CHEAP_ENABLE_INTS

__CHEAP_ENABLE_INTS:
		sti
		ret

