O=	main.o db.o serve.o
Y=	main.O db.O serve.O

all:	nd cmd

nd:	$Y $O 
	-cyntax $Y -lipc 
	cc -o nd $O -lipc -lcbt

$O:	nd.h

cmd:	cmd.o cmd.O
	-cyntax cmd.O -lipc 
	cc -o cmd cmd.o -lc -lipc

%.o:	%.c
	cc -c $(CFLAGS) $%.c

%.O:	%.c
	cyntax -c $%.c

clean:
	rm -f nd cmd *.o *.O

install:
	cp nd /usr/notary/notaryd
	chmod 775 /usr/notary/notaryd 
	/etc/chown root,bin /usr/notary/notaryd
