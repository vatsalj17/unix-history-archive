/ $Header: $
		.unixorder

/ Cheap interrupt-disable function for internal use in STREAMS, DDI/DKI.
/	void __CHEAP_DISABLE_INTS (void)
/ Prototyped in <sys/inline.h>
/ $Log: $
/
		.text
		.globl	__CHEAP_DISABLE_INTS

__CHEAP_DISABLE_INTS:
		cli
		ret
