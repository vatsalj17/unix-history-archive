CFLAGS	= -O
OBS	=\
	code_gen.o constant.o decl.o decl_item.o decl_shared.o\
	expr.o f_equiv.o f_expr.o f_id.o f_life.o\
	f_main.o f_sub.o f_trace.o factor.o find_ident.o\
	find_str.o find_type.o function.o init.o\
	initi.o io.o jump.o \
	lex.o macdep.o main.o massage.o \
	memory.o parse.o readonly.o sort.o\
	stats.o suopt.o sutab.o xnodes.o
COBS	=\
	code_gen.O constant.O decl.O decl_item.O decl_shared.O\
	expr.O f_equiv.O f_expr.O f_id.O f_life.O\
	f_main.O f_sub.O f_trace.O factor.O find_ident.O\
	find_str.O find_type.O function.O init.O\
	initi.O io.O jump.O \
	lex.O macdep.O main.O massage.O \
	memory.O parse.O readonly.O sort.O\
	stats.O suopt.O sutab.O xnodes.O

%.o	: %.c
	$(CC) $(CFLAGS) -c $%.c
	cyntax -c $%.c

ccom	: $(OBS) bits.o
	cyntax $(COBS) bits.O
	$(CC) $(OBS) bits.o -o ccom
	echo done
clean	:
	rm -f $(COBS) $(OBS) ccom

bits.c: tables.c
	$(CC) -E tables.c | ./sets > bits.c


install: ccom
	cp ccom /usr/lib/cyntax/ccom
	
ctags:
	ctags *.c

$(OBS):	defs.h

bits.c code_gen.o constant.o decl.o f_equiv.o f_expr.o: cnodes.h
f_id.o f_life.o f_main.o f_sub.o f_trace.o factor.o: cnodes.h
function.o initi.o io.o jump.o macdep.o massage.o: cnodes.h
memory.o parse.o stats.o : cnodes.h

stats.o massage.o initi.o init.o factor.o expr.o: xtypes.h
decl_shared.o decl_item.o decl.o constant.o: xtypes.h

constant.o decl.o decl_item.o decl_shared.o expr.o f_expr.o: types.h
f_id.o f_life.o f_sub.o factor.o find_ident.o init.o: types.h
initi.o io.o macdep.o massage.o parse.o stats.o: types.h

bits.c constant.o decl.o decl_item.o decl_shared.o expr.o : tokens.h
f_life.o f_sub.o factor.o function.o initi.o io.o : tokens.h
jump.o lex.o memory.o parse.o stats.o : tokens.h

constant.o expr.o factor.o : void.h

decl.o f_life.o f_main.o f_sub.o function.o stats.o: func.h

f_equiv.o f_expr.o f_id.o f_life.o f_main.o f_sub.o f_trace.o: flow.h

decl.o decl_shared.o factor.o find_type.o initi.o: io.h
io.o lex.o main.o memory.o: io.h

decl.o factor.o find_ident.o function.o stats.o: parse.h

bits.c decl.o decl_shared.o factor.o find_type.o: stdobj.h
initi.o io.o : stdobj.h


bits.c decl.o f_life.o function.o initi.o main.o : options.h
bits.c initi.o :objects.h

decl_shared.o factor.o initi.o lex.o macdep.o massage.o parse.o: macdep.h
sort.o	suopt.o:	sort.h
