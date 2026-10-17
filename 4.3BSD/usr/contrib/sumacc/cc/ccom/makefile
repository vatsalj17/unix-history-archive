# Master makefile for the 68000 compiler

t:
		@make -f makefile_t
		./ccom68 test.c >test.s

o:
		@make -f makefile_o

b:
		@make -f makefile_b

clean:
		rm -f *.s *.o *.b *BAK *CKP ccom ccom68

install:
	@make -f makefile_t install
