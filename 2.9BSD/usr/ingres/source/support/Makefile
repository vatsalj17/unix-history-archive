BIN=	../../bin
LIB=	../../lib
LIBS=	$(LIB)/dbulib $(LIB)/access $(LIB)/iutil $(LIB)/iutil $(LIB)/gutil
CFLAGS= -O
LDFLAGS=-i
OBJS=	error.o
MODE=	4711
ALL=	creatdb destroydb purge helpr ingres printr restore sysmod \
	usersetup drtst timefix

all: $(ALL)

creatdb: creatdb.o $(OBJS)
	$(CC) $(LDFLAGS) creatdb.o $(OBJS) $(LIBS) -o creatdb

destroydb: destroydb.o
	$(CC) $(LDFLAGS) destroydb.o $(LIBS) -o destroydb

purge: purge.o subs.o $(OBJS)
	$(CC) $(LDFLAGS) purge.o subs.o $(OBJS) $(LIBS) -o purge
		
helpr: helpr.o $(OBJS)
	$(CC) $(LDFLAGS) helpr.o $(OBJS) $(LIBS) -o helpr

ingres: ingres.o
	$(CC) $(LDFLAGS) ingres.o $(LIBS) -o ingres

printr: printr.o
	$(CC) $(LDFLAGS) printr.o $(LIBS) -o printr

restore: restore.o $(OBJS) subs.o
	$(CC) $(LDFLAGS) restore.o $(OBJS) subs.o $(LIBS) -o restore

sysmod: sysmod.o $(OBJS)
	$(CC) $(LDFLAGS) sysmod.o $(OBJS) $(LIBS) -o sysmod

usersetup: usersetup.o
	$(CC) $(LDFLAGS) usersetup.o $(LIBS) -o usersetup

drtst: drtst.o
	$(CC) $(LDFLAGS) drtst.o $(LIBS) -o drtst

timefix: timefix.o
	$(CC) $(LDFLAGS) timefix.o $(LIBS) -o timefix

install: creatdb destroydb helpr ingres printr restore sysmod purge usersetup
	cp usersetup $(BIN)/usersetup
	chmod 700 $(BIN)/usersetup
	cp purge $(BIN)/purge
	chmod $(MODE) $(BIN)/purge
	cp creatdb $(BIN)/creatdb
	chmod $(MODE) $(BIN)/creatdb
	cp destroydb $(BIN)/destroydb
	chmod $(MODE) $(BIN)/destroydb
	cp helpr $(BIN)/helpr
	chmod $(MODE) $(BIN)/helpr
	cp ingres $(BIN)/ingres
	chmod $(MODE) $(BIN)/ingres
	cp printr $(BIN)/printr
	chmod $(MODE) $(BIN)/printr
	cp restore $(BIN)/restore
	chmod $(MODE) $(BIN)/restore
	cp sysmod $(BIN)/sysmod
	chmod $(MODE) $(BIN)/sysmod

clean:
	rm *.o
