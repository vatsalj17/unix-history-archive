/ $Header: $
/ Here we define some selector values that are very important to the kernel
/ library code. The most important is the value for the kernel data segment
/ selector, which the trap- and interrupt-dispatching code needs to set up
/ the kernel %ds for the C-language code.
/ $Log: $
/
		.set	SEL_386_KD, 0x21	/ kernel data selector
		.set	SEL_386_UD, 0x13	/ user data selector
