# If your system lacks the ANSI routines memmove and strtoul
# restore the commented-out definition of $(ANSI)

ANSI = # memmove.o strtoul.o
OBJ = fsort.o rsort.o number.o field.o tables.o merge.o $(ANSI)

sort:	$(OBJ) files.o
	$(CC) $(CFLAGS) $(OBJ) files.o -o sort
	rm files.o

# A cheat on classic System V stdio.  If "make fast" compiles
# successfully, you'll get a faster sort.

fast:	$(OBJ) cfiles.o cgets.o
	$(CC) $(CFLAGS) $(OBJ) cfiles.o cgets.o -o sort
	rm cfiles.o

test: checksum
	PATH=.:$$PATH sorttest

fsort.o: fsort.h
rsort.o: fsort.h
number.o: fsort.h
field.o: fsort.h
tables.o: fsort.h
files.o: fsort.h
merge.o: fsort.h
cfiles.o: fsort.h

clean:
	rm -f *.o sort in in1 in2 out xx xsort ysort *.out 
	rm -f unwritablefile linecount -k - -o checksum* xx.c
