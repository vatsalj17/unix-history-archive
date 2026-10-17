CFLAGS	= -O
BIN	= /usr/bin
LIB	= /usr/lib/cyntax

OBS	= diag.o equiv.o fix.o main.o misc.o read.o string.o symbol.o\
	  table.o time.o type.o var.o
COBS	= diag.O equiv.O fix.O main.O misc.O read.O string.O symbol.O\
	  table.O time.O type.O var.O

all	: cem cyntax

cem	: $(OBS) $(COBS)
	cyntax $(COBS)
	$(CC) $(LFLAGS) $(OBS) -o cem

%.O: %.o
	cyntax -c $%.c

diag.o	: cem.h stdobj.h types.h symbol.h type.h
equiv.o	: cem.h stdobj.h types.h symbol.h type.h var.h
fix.o	: cem.h stdobj.h 	 symbol.h type.h
main.o	: cem.h
misc.o	: cem.h
read.o	: cem.h stdobj.h
string.o: cem.h
symbol.o: cem.h 		 symbol.h
table.o	: cem.h stdobj.h 	 symbol.h type.h
time.o	: cem.h
type.o	: cem.h stdobj.h 	 symbol.h type.h
var.o	: cem.h stdobj.h types.h symbol.h type.h var.h

clean	:
	-rm -f $(OBS) cem cyntax

install	: $(BIN)/cyntax $(LIB)/cem

$(BIN)/cyntax: cyntax
	cp cyntax /usr/bin/cyntax

$(LIB)/cem: cem
	cp cem /usr/lib/cyntax/cem

