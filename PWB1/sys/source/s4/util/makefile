LIB = /usr/lib

CC = cc
CCFLAGS = -O

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
# install: Create and install libpw.a.
#------
install: libpw.a libwrt.a
	mv libpw.a $(LIB)
	mv libwrt.a $(LIB)
	-chown bin $(LIB)/libpw.a $(LIB)/libwrt.a
	ls -l $(LIB)/libpw.a $(LIB)/libwrt.a


#------
# libpw.a: Create libpw.a in local directory.
#------
libpw.a:	curdir.o fdfopen.o giveup.o lockit.o rename.o userdir.o\
	username.o xalloc.o xcreat.o xlink.o xopen.o \
	xpipe.o xunlink.o xmsg.o dname.o fatal.o setsig.o userexit.o

libpw.a:	clean.o syswrite.o alloca.o anystr.o bal.o cat.o imatch.o\
	index.o move.o patoi.o \
	patol.o repeat.o repl.o satoi.o sname.o substr.o trnslat.o \
	verify.o any.o zero.o zeropad.o strend.o

libpw.a:
	-rm -f libpw.a tmp.a
	$(AR) r tmp.a curdir.o fdfopen.o giveup.o lockit.o rename.o userdir.o\
		username.o xalloc.o xcreat.o xlink.o xopen.o \
		xpipe.o xunlink.o xmsg.o dname.o fatal.o setsig.o userexit.o
	$(AR) r tmp.a clean.o syswrite.o alloca.o anystr.o bal.o cat.o \
		imatch.o index.o move.o patoi.o \
		patol.o repeat.o repl.o satoi.o sname.o substr.o trnslat.o \
		verify.o any.o zero.o zeropad.o strend.o
	-chmod 644 tmp.a
	mv tmp.a libpw.a

#------
# libwrt.a: Create libwrt.a in local directory.
#------
libwrt.a:	write.o syswrite.o
	-rm -f libwrt.a tmp.a
	$(AR) r tmp.a write.o syswrite.o
	-chmod 644 tmp.a
	mv tmp.a libwrt.a


#------
# clean: Remove all .o files.
#------
clean:
	-rm -f *.o


#------
# Dependencies for individual .o files.
#------
curdir.o:	curdir.c
fdfopen.o:	fdfopen.c
giveup.o:	giveup.c
lockit.o:	lockit.c
rename.o:	rename.c
userdir.o:	userdir.c
username.o:	username.c
xalloc.o:	xalloc.c
xcreat.o:	xcreat.c
xlink.o:	xlink.c
xopen.o:	xopen.c
xpipe.o:	xpipe.c
xunlink.o:	xunlink.c
xmsg.o:	xmsg.c
dname.o:	dname.c
fatal.o:	fatal.c
setsig.o:	setsig.c
userexit.o:	userexit.c
clean.o:	clean.c
syswrite.o:	syswrite.s
alloca.o:	alloca.s
anystr.o:	anystr.c
bal.o:	bal.c
cat.o:	cat.c
imatch.o:	imatch.c
index.o:	index.c
move.o:	move.c
patoi.o:	patoi.c
patol.o:	patol.c
repeat.o:	repeat.c
repl.o:	repl.c
satoi.o:	satoi.c
sname.o:	sname.c
substr.o:	substr.c
trnslat.o:	trnslat.c
verify.o:	verify.c
any.o:	any.c
zero.o:	zero.c
zeropad.o:	zeropad.c
strend.o:	strend.c
write.o:	write.c
