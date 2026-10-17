CFLAGS=-O

autores:	autores.o
	cc -o autores $(CFLAGS) autores.o

clean:
	rm -f autores errs *.o

install:	autores
	install -s -m 700 -o mmdf -g cur autores /usr/lib/mmdf
