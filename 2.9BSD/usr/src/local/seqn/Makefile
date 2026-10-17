#FP = -f
CFLAGS = $(FP) -O -s -DNEQN 
LFLAGS=
YFLAGS=-d

all:	seqn

cp:	seqn
	cp seqn /bin/seqn
	rm *.o e.c e.def seqn

cmp:	seqn
	cmp seqn /bin/seqn
	rm *.o e.c e.def seqn

SOURCE = e.y e.h diacrit.c eqnbox.c font.c fromto.c funny.c glob.c integral.c \
 io.c lex.c lookup.c mark.c matrix.c move.c over.c paren.c \
 pile.c shift.c size.c sqrt.c text.c

FILES = e.o diacrit.o eqnbox.o font.o fromto.o funny.o glob.o integral.o \
 io.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o \
 pile.o shift.o size.o sqrt.o text.o

seqn:	$(FILES)
	cc -n $(CFLAGS) $(FILES) -o seqn $(LFLAGS)

e.c:	e.def

e.def:	e.y
	yacc -d e.y
	mv y.tab.c e.c
	mv y.tab.h e.def

$(FILES):	e.h e.def

list:
	pr TODO $(SOURCE) makefile

gcos:	y.tab.c
	fsend e.h e.y *.c

src:	makefile $(SOURCE)
	cp $? /usr/src/cmd/neqn
	touch src

lint:	y.tab.c
	lint -spb *.c
