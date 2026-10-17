PRGS=cc68 as68 c268 ccom68 ld68 lorder68 nm68 pr68 rl68 size68
LIBS=a68.hdr libc68.a
CFLAGS= -noshare -O 

install: newh cc68
	-mkdir ../bin
	-mv $(PRGS) ../bin
	-mv $(LIBS) ../lib

cc68:	cc68.o
	cc ${CFLAGS} -o cc68 cc68.o
	rm cc68.h cc68.o

cc68.o:	cc68.h
  
newh:
	sed 's;MACROOT;$(MACROOT);' cc68.hdr >cc68.h

# 'zap' can be used after installing with Eunice to zap some directories
# if you are tight on space.  Van Jacobson's Eunice instructions 
# (cceun/README) use directories bin, include and lib to hold files
# needed for user access of the compiler, loader, rmaker, etc.
# You may also want to remove directories cmd, etc, fix, info, and ws
# if you don't need them.

zap:
	rm -rf ../cc41 ../cc42 ../h ../man 
	rm -rf ../lib/io ../lib/*.s ../lib/*.c ../lib/*.b ../lib/libmact.a

