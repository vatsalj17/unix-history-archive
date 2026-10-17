INGRES=	../..
CC=	cc
CFLAGS=	-O
AR=	ar
ARFLAGS=rvu
LIB=	$(INGRES)/lib

OBJS= exit.o strout.o _cleanup.o getufield.o append.o atof.o atol.o \
	bequal.o bitpos.o concat.o iocv.o itoa.o length.o scompare.o \
	sequal.o ftoa.o pad.o pmove.o prargs.o ztack.o fullwait.o \
	need.o freebuf.o syserr.o oatoi.o longconst.o setflag.o \
	cat.o getpw.o fgetline.o fread.o getw.o getchar.o getc.o \
	ungetc.o rewind.o fopen.o set_so_buf.o set_si_buf.o \
	trace.o atoi.o smove.o setbuf.o fwrite.o xputchar.o putchar.o putc.o \
	fclose.o fseek.o fflush.o _filbuf.o _flsbuf.o _getbuf.o bmove.o \
	locv.o wait.o  bitcnt.o  tTf.o  _iob.o  malloc.o 

gutil:	$(OBJS)
	-$(AR) $(ARFLAGS) gutil $(OBJS)	

.s.o:
	as -o $@ /usr/include/sys.s $<

install: gutil
	chmod	644	gutil
	cp gutil $(LIB)/gutil

clean:
	rm *.o
