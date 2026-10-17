
D = /bin/cp

a.out:	cp.c
	cc -Od2 cp.c

install:	a.out
	strip a.out
	cat a.out >$D
	chmod +x $D

clean:
	rm -f *.o a.out
