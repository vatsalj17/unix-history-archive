o=	ins.o qdo.o xs.o log.o verify.o
O=	ins.O qdo.O xs.O log.O verify.O
CYF=	-VLog:2 
CFLAGS=	 -O 
ins:	$o $O
	cyntax $(CYF) $O
	cc -o ins $o
$O $o:	ins.h
%.O:	%.c
	cyntax $(CYF) -c $%.c
install:	ins
	cp ins /usr/bin/ins
	chmod 775 /usr/bin/ins
	/etc/chown bin,bin /usr/bin/ins

