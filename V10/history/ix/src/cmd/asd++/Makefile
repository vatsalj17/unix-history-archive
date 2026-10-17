CC = nCC
LIBOBJ = checksum.o decl.o impl.o param.o path.o
OBJ = $(LIBOBJ) dkinstall.o 
LIB = lib.a -lnstring -lipc
LIBPROGS = dkinstall 

all: $(LIBPROGS)

dkinstall:	lib.a dkinstall.o
	$(CC) -o dkinstall dkinstall.o $(LIB)

install:dkinstall
	cp dkinstall /usr/lib
	chmod 775 /usr/lib/dkinstall
	/etc/chown bin,bin /usr/lib/dkinstall

lib.a:	$(LIBOBJ)
	rm -f lib.a
	ar qc lib.a $(LIBOBJ)
	ranlib lib.a

$(OBJ):	decl.h

clean:
	rm -f *.o lib.a $(LIBPROGS)
