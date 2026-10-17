# DEBUG=DEBUG
FP =			# set to -f on machines without floating point
CFLAGS= -O -n -D$(DEBUG)
OBJECTS= main.o globals.o enqueue.o deliver.o send.o time.o misc.o

all:	remind prfile delrem

remind: $(OBJECTS)
	cc $(CFLAGS) $(FP) $(OBJECTS) -o remind
	chown root remind
	chmod 04755 remind

$(OBJECTS):	remind.h

prfile:	remind.h prfile.o globals.o
	cc $(CFLAGS) prfile.o globals.o -o prfile
	chown root prfile
	chmod 04755 prfile

delrem: delrem.o
	cc $(CFLAGS) delrem.o -o delrem
	chown root delrem
	chmod 04755 delrem
