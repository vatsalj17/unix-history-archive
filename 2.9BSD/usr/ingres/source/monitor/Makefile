INGRES=	../..
CC=	cc
CFLAGS=	-O
AR=	ar
ARFLAGS=rvu
LIB=	$(INGRES)/lib
BIN=	$(INGRES)/bin

LIBS= $(LIB)/iutil $(LIB)/gutil

OBJS= branch.o buf.o clear.o clrline.o edit.o eval.o expr.o getch.o \
	getfilename.o go.o include.o mac.o main.o mcall.o monitor.o newdirec.o \
	print.o prompt.o quit.o shell.o trapquery.o writeout.o xwait.o

monitor: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o monitor

monlib:	$(OBJS)
	-$(AR) $(ARFLAGS) $(OBJS)

install: monitor
	cp monitor $(BIN)/monitor
