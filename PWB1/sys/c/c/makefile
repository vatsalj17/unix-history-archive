# You should be `root' to install things.

BIN = /bin
LIB = /lib
PREF =

STICKY = OLD

CC = cc
CFLAGS = -O
YACC = yacc
LEX = lex
LDFLAGS = -n -s

#------
# install: Create & install cc, c0, c1, c2, cpp, using $(PREF) as version ID.
#------
install: cc c0 c1 c2 cpp
	-mv $(BIN)/$(PREF)cc $(BIN)/$(STICKY)$(PREF)cc
	-mv $(LIB)/$(PREF)c0 $(LIB)/$(STICKY)$(PREF)c0
	-mv $(LIB)/$(PREF)c1 $(LIB)/$(STICKY)$(PREF)c1
	-mv $(LIB)/$(PREF)c2 $(LIB)/$(STICKY)$(PREF)c2
	-mv $(LIB)/$(PREF)cpp $(LIB)/$(STICKY)$(PREF)cpp
	mv cc $(BIN)/$(PREF)cc
	mv c0 $(LIB)/$(PREF)c0
	mv c1 $(LIB)/$(PREF)c1
	mv c2 $(LIB)/$(PREF)c2
	mv cpp $(LIB)/$(PREF)cpp
	-chown bin $(BIN)/$(PREF)cc $(LIB)/$(PREF)c0 $(LIB)/$(PREF)c1 \
			$(LIB)/$(PREF)c2 $(LIB)/$(PREF)cpp
	-chmod 1755 $(BIN)/$(PREF)cc $(LIB)/$(PREF)c0 $(LIB)/$(PREF)c1 \
			$(LIB)/$(PREF)c2 $(LIB)/$(PREF)cpp
	ls -l $(BIN)/$(PREF)cc $(LIB)/$(PREF)c0 $(LIB)/$(PREF)c1 \
			$(LIB)/$(PREF)c2 $(LIB)/$(PREF)cpp
	echo "Remove following STICKY files after next reboot:"
	-ls -l $(BIN)/$(STICKY)$(PREF)cc $(LIB)/$(STICKY)$(PREF)c0 \
			$(LIB)/$(STICKY)$(PREF)c1 \
			$(LIB)/$(STICKY)$(PREF)c2 $(LIB)/$(STICKY)$(PREF)cpp

cc: cc.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o cc cc.c -lS


c0: c00.o c01.o c02.o c03.o c04.o c05.o
	$(CC) $(LDFLAGS) -o c0 c00.o c01.o c02.o c03.o c04.o c05.o

c00.o c01.o c02.o c03.o c04.o c05.o: c0.h


c1: c10.o c11.o c12.o c13.o table.o
	$(CC) $(LDFLAGS) -o c1 c10.o c11.o c12.o c13.o table.o

c10.o c11.o c12.o c13.o: c1.h

table.o: table.s cvopt
	= p $$p: ; cvopt <table.s >table.i
	as -o table.o table.i
	rm table.i

cvopt:	cvopt.c
	$(CC) -n -s -O -o cvopt cvopt.c


c2: c20.o c21.o
	$(CC) $(LDFLAGS) -o c2 c20.o c21.o

c20.o c21.0: c2.h


cpp: cpp.o cpy.o
	$(CC) $(CFLAGS) $(LDFLAGS) cpp.o cpy.o -ly -ll -lS -o cpp

cpp.o: cpp.c

cpy.o: lex.yy.c cpy.c

lex.yy.c: cpl.l
	$(LEX) cpl.l


clean:
	-rm cpp.o lex.yy.c cpy.c cpy.o y.tab.c
	-rm c0?.o c1?.o c2?.o
