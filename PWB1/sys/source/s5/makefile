LIB = /lib
DD = tmp
LNAME = libPW.a

CC = cc
CCFLAGS = -O
USRINC = /usr/include/

LD = -ld
LDFLAGS = -x

.c.o:
	$(CC) -c $(CCFLAGS) $<
	$(LD) -r $(LDFLAGS) $*.o
	mv a.out $*.o

AS = -as

.s.o:
	$(AS) -o $*.o $<
	$(LD) -r $(LDFLAGS) $*.o
	mv a.out $*.o

AR = ar


#------
# install: Create and install libPW.a.
#------
install: libPW.a
	-mv $(LIB)/$(LNAME) $(LIB)/$(DD)$(LNAME)
	mv libPW.a $(LIB)/$(LNAME)
	-chown bin $(LIB)/$(LNAME)
	-rm -f $(LIB)/$(DD)$(LNAME)
	ls -l $(LIB)/$(LNAME)


#------
# libPW.a: Create libPW.a in local directory.
#------
libPW.a: cgpid.o dscnd.o gmtch.o pexec.o regcmp.o regex.o \
	strf.o udata.o uname.o ustat.o utime.o

libPW.a:
	-rm -f libPW.a tmp.a
	$(AR) r tmp.a cgpid.o dscnd.o gmtch.o pexec.o regcmp.o regex.o \
			udata.o uname.o ustat.o utime.o strf.o
	-chmod 644 tmp.a
	mv tmp.a libPW.a


#------
# clean: Remove all .o files.
#------
clean:
	-rm -f cgpid.o dscnd.o gmtch.o pexec.o regcmp.o regex.o \
			strf.o udata.o uname.o ustat.o utime.o


#------
# Dependencies for individual .o files.
#------
cgpid.o:	cgpid.c
dscnd.o:	dscnd.c $(USRINC)stdio.h $(USRINC)stat.h
gmtch.o:	gmtch.c
pexec.o:	pexec.c
regcmp.o:	regcmp.c
regex.o:	regex.c
strf.o:	strf.c
udata.o:	udata.s
uname.o:	uname.s
ustat.o:	ustat.s
utime.o:	utime.s
