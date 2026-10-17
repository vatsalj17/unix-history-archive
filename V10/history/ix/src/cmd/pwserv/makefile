CFLAGS=-O
CYN=	main.O ok.O pwfile.O
OBJ=	main.o ok.o pwfile.o

pwserv:	$(OBJ)
	cc -o pwserv $(OBJ) -lipc

cyntax: $(CYN)
	cyntax -Vlog:1 $(CYN) -lipc

$(CYN) $(OBJ):	pwserv.h
%.O:	%.c
	cyntax -c -Vlog:1 $%.c

clean:
	rm -f $(OBJ) $(CYN) pwserv
install: pwserv
	cp pwserv /etc
	strip /etc/pwserv
