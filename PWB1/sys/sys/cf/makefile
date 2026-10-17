NAME = unix

.s.o:
	-as -o $*.o $<

# procedure to load a new unix
# libraries lib0, lib1, lib2 have a separate makefile

UNIX: low.o conf.o
	-/bin/ld -x -r -d low.o ../lib0 conf.o ../lib[1-9]
	/etc/sysfix a.out $(NAME) $(NAME)
	-rm -f a.out
	size $(NAME)
