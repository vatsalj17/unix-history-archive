CFLAGS=		-O
FP =			# -f for machines without floating point
FMT =		-i	# -n for machines without floating point

all:		adb

cp:		adb
		cp adb ${DESTDIR}/bin/adb
		-strip ${DESTDIR}/bin/adb

cmp:		adb
		cmp adb ${DESTDIR}/bin/adb

OBJS =		access.o command.o expr.o findfn.o \
		format.o input.o opset.o main.o \
		message.o output.o pcs.o print.o \
		runpcs.o setup.o sym.o

adb:		${OBJS}
		cc ${FP} ${FMT} -o adb ${OBJS}

clean:
		rm -f *.o

access.o:	defs.h
command.o:	defs.h
expr.o:		defs.h
findrtn.o:	defs.h
format.o:	defs.h
input.o:	defs.h
main.o:		defs.h
message.o:	mac.h mode.h
output.o:	defs.h
pcs.o:		defs.h
print.o:	defs.h
runpcs.o:	defs.h
setup.o:	defs.h
sym.o:		defs.h
