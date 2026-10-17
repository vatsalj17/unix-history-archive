CC=cc
LIB=../../lib
BIN=../../bin
LIBS=$(LIB)/scanner $(LIB)/access $(LIB)/gutil $(LIB)/iutil -lm
CFLAGS=-O
LDFLAGS=-i
YACC=$(BIN)/yyyacc
YFLAGS=-s
MODE=4700
LIBMOD=644
AR= ar
OBJS= 	attfcn.o\
	xdot.o\
	main.o\
	rngfcn.o\
	tree.o\
	norml.o\
	call_p.o\
	call_tree.o\
	control.o\
	format.o\
	p_util.o\
	getscr.o

parser: y.tab.o parlib
	$(CC) $(LDFLAGS) y.tab.o parlib $(LIBS) -o parser

parlib: $(OBJS)
	-$(AR) rvuc parlib $(OBJS)
	cp parlib $(LIB)/parlib
	chmod $(LIBMOD) $(LIB)/parlib

y.tab.c: grammar.y yyerror.y tables.y
	$(YACC) $(YFLAGS) grammar.y

grammar.y: master.grammar
	grep -v "DDD" master.grammar > grammar.y 

install: parser
	cp parser $(BIN)/parser
	chmod $(MODE) $(BIN)/parser

clean:
	rm *.o
