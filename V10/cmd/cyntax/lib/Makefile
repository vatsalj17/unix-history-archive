LIB		= /usr/lib/cyntax
CCOM		= $(LIB)/ccom
CEM		= $(LIB)/cem

all		: libc libj

libj		: llib-lj $(CCOM) $(CEM)
		/lib/cpp -M -I/usr/jerq/include llib-lj | $(CCOM) -llibj - out && $(CEM) -p out && mv out libj

libg		: llib-lg $(CCOM) $(CEM)
		/lib/cpp -M -I/n/coma/usr/2include -Uvax -Dmc68020=1 llib-lg | $(CCOM) -Vprint:1 -Vfprint:2 -Vsprint:2 -VFprint:2 -llibg - out && $(CEM) -p out && mv out libg

libk		: llib-lk $(CCOM) $(CEM)
		/lib/cpp -M -I/usr/ninclude -Uvax -Dmc68020=1 llib-lk | $(CCOM) -Vprint:1 -Vfprint:2 -Vsprint:2 -llibk - out && $(CEM) -p out && mv out libk

lib%		: llib-l% var% $(CCOM) $(CEM)
		/lib/cpp -M llib-l$% | $(CCOM) `cat var$%` -llib$% - out && $(CEM) -p out && mv out lib$%

install		: $(LIB)/libc $(LIB)/libj

$(LIB)/lib%	: lib%
		cp lib$% $(LIB)/lib$%

clean		:
		-rm -f libc libj out
