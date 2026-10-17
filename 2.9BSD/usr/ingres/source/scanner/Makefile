INGRES=	../..
LIB=	$(INGRES)/lib
CC=	cc
CFLAGS=	-O
AR=	-ar
ARFLAGS=rvu


OBJS=	parser.o \
	yylex.o \
	inout.o \
	name.o \
	number.o \
	operator.o \
	string.o \
	symtab.o \
	comment.o \
	cmap.o

scanner: $(OBJS)
	-$(AR) $(ARFLAGS) scanner $(OBJS)

install: scanner
	chmod 644 scanner
	cp scanner $(LIB)/scanner

clean:
	rm *.o
