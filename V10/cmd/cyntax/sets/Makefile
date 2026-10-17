DEFS	=
CFLAGS	=
LDFLAGS	=  -ll
INCS	=
INSDIR	= ../cyn
OBS	= \
	sets.o sets.lex.o

sets		: $(OBS)
		$(CC) -o sets $(OBS) $(LDFLAGS)

sets.lex.c	: sets.lex.l
		lex sets.lex.l
		mv lex.yy.c sets.lex.c

install		: $(INSDIR)/sets

$(INSDIR)/sets	: sets
		cp sets $(INSDIR)/sets

$(OBS)		: $(INCS)

clean		:
		-rm -f $(OBS) sets.lex.c 
