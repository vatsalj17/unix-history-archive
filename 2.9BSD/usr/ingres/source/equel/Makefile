INGRES=../..
BIN=$(INGRES)/bin
LIB=$(INGRES)/lib
LIBS=$(LIB)/equelib $(LIB)/iutil $(LIB)/gutil -lm
YACC=$(BIN)/yyyacc
YFLAGS=
CC=cc
CFLAGS=-O
LDFLAGS=
AR=ar
OBJS=	main.o\
	parser.o\
	yylex.o\
	cvar.o\
	include.o\
	name.o\
	getkey.o\
	number.o\
	operator.o\
	prtout.o\
	string.o\
	comment.o\
	retrieve.o\
	display.o\
	getch.o\
	cmap.o\
	yyerror.o\
	nalloc.o

equel: equelib y.tab.o
	$(CC) $(LDFLAGS) y.tab.o $(LIBS) -o equel

y.tab.c: grammar.y
	$(YACC) $(YFLAGS) grammar.y

equelib: $(OBJS)
	-$(AR) crvu equelib $(OBJS)
	cp equelib $(LIB)/equelib
	chmod	0644 $(LIB)/equelib

install: equel
	cp equel $(BIN)/equel
	chmod 711 $(BIN)/equel

clean:
	rm *.o
