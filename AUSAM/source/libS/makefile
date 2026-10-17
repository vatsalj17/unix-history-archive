LIB = /lib
DD = TMP
USRINC = /usr/include

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
# install: Create & install libS.a and libp.a.
#------
install: in_libS.a in_libp.a


#------
# in_stdio.h:	Install stdio.h in $(USRINC).
#------
in_stdio.h:
	-mv $(USRINC)/stdio.h $(USRINC)/$(DD)stdio.h
	cp stdio.h $(USRINC)/stdio.h
	-chmod 644 $(USRINC)/stdio.h
	-chown bin $(USRINC)/stdio.h
	-rm -f $(USRINC)/$(DD)stdio.h
	ls -l $(USRINC)/stdio.h


#------
# libS.a: Create libS.a in local directory.
#------
libS.a:	intss.o fgetc.o fputc.o freopen.o fgets.o fputs.o getpw.o fseek.o rew.o \
	rdwr.o stuff.o system.o tmpnam.o fopen.o scanf.o doscan.o fprintf.o \
	gets.o getw.o printf.o puts.o putw.o sprintf.o ungetc.o data.o ctype.o

libS.a:	filbuf.o setbuf.o fltpr.o doprnt.o ffltpr.o strout.o flsbuf.o \
	error.o ftell.o gcvt.o calloc.o clrerr.o

libS.a:
	-rm -f libS.a tmp.a
	$(AR) r tmp.a intss.o fgetc.o fputc.o freopen.o \
			fgets.o fputs.o getpw.o fseek.o rew.o
	$(AR) r tmp.a rdwr.o stuff.o system.o tmpnam.o fopen.o scanf.o doscan.o fprintf.o
	$(AR) r tmp.a gets.o getw.o printf.o puts.o putw.o sprintf.o ungetc.o 
	$(AR) r tmp.a filbuf.o setbuf.o fltpr.o doprnt.o ffltpr.o strout.o flsbuf.o
	$(AR) r tmp.a error.o ftell.o gcvt.o calloc.o clrerr.o data.o ctype.o
	-chmod 644 tmp.a
	mv tmp.a libS.a


#------
# in_libS.a: Create libS.a, if it isn't already, and move it to $(LIB)/libS.a.
#------
in_libS.a: libS.a
	-mv $(LIB)/libS.a $(LIB)/$(DD)libS.a
	mv libS.a $(LIB)/libS.a
	-chmod 644 $(LIB)/libS.a
	-chown bin $(LIB)/libS.a
	-rm -f $(LIB)/$(DD)libS.a
	ls -l $(LIB)/libS.a


#------
# libp.a: Create (substitute) libp.a in local directory.
#------
libp.a:	portli.o \
	intss.o fgetc.o fputc.o freopen.o fgets.o fputs.o getpw.o fseek.o rew.o \
	rdwr.o stuff.o system.o tmpnam.o fopen.o scanf.o doscan.o fprintf.o \
	gets.o getw.o printf.o puts.o putw.o sprintf.o ungetc.o data.o ctype.o

libp.a:	filbuf.o setbuf.o fltpr.o doprnt.o ffltpr.o strout.o flsbuf.o \
	error.o ftell.o gcvt.o calloc.o clrerr.o

libp.a:
	-rm -f libp.a tmp.a
	$(AR) r tmp.a portli.o intss.o fgetc.o fputc.o freopen.o \
			fgets.o fputs.o getpw.o fseek.o rew.o
	$(AR) r tmp.a rdwr.o stuff.o system.o tmpnam.o fopen.o scanf.o doscan.o fprintf.o
	$(AR) r tmp.a gets.o getw.o printf.o puts.o putw.o sprintf.o ungetc.o
	$(AR) r tmp.a filbuf.o setbuf.o fltpr.o doprnt.o ffltpr.o strout.o flsbuf.o
	$(AR) r tmp.a error.o ftell.o gcvt.o calloc.o clrerr.o data.o ctype.o
	-chmod 644 tmp.a
	mv tmp.a libp.a


#------
# in_libp.a: Create libp.a, if it isn't already, and move it to $(LIB)/libp.a.
#------
in_libp.a: libp.a
	-mv $(LIB)/libp.a $(LIB)/$(DD)libp.a
	mv libp.a $(LIB)/libp.a
	-chmod 644 $(LIB)/libp.a
	-chown bin $(LIB)/libp.a
	-rm -f $(LIB)/$(DD)libp.a
	ls -l $(LIB)/libp.a


#------
# clean: Remove all .o files created for libS.a & libp.a.
#------
clean:
	-rm -f intss.o fgetc.o fputc.o freopen.o \
			fgets.o fputs.o getpw.o fseek.o rew.o
	-rm -f rdwr.o stuff.o system.o tmpnam.o fopen.o scanf.o doscan.o fprintf.o
	-rm -f gets.o getw.o printf.o puts.o putw.o sprintf.o \
			ungetc.o data.o ctype.o
	-rm -f filbuf.o setbuf.o fltpr.o doprnt.o ffltpr.o strout.o flsbuf.o
	-rm -f error.o ftell.o gcvt.o calloc.o clrerr.o portli.o


#------
# Dependencies for indiv files.
#------
intss.o:	intss.c
fgetc.o:	fgetc.c stdio.h
fputc.o:	fputc.c stdio.h
freopen.o:	freopen.c stdio.h
fgets.o:	fgets.c stdio.h
fputs.o:	fputs.c stdio.h
getpw.o:	getpw.c stdio.h
fseek.o:	fseek.c stdio.h
rew.o:	rew.c stdio.h

rdwr.o:	rdwr.c stdio.h
stuff.o:	stuff.c
system.o:	system.c
tmpnam.o:	tmpnam.c
fopen.o:	fopen.c stdio.h
scanf.o:	scanf.c stdio.h
doscan.o:	doscan.c stdio.h
fprintf.o:	fprintf.c stdio.h

gets.o:	gets.c stdio.h
getw.o:	getw.c stdio.h
printf.o:	printf.c stdio.h
puts.o:	puts.c stdio.h
putw.o:	putw.c stdio.h
sprintf.o:	sprintf.c stdio.h
ungetc.o:	ungetc.c stdio.h
data.o:	data.c stdio.h

filbuf.o:	filbuf.c stdio.h
setbuf.o:	setbuf.c stdio.h
fltpr.o:	fltpr.s
doprnt.o:	doprnt.s
ffltpr.o:	ffltpr.s
strout.o:	strout.c stdio.h
flsbuf.o:	flsbuf.c stdio.h

error.o:	error.c stdio.h
ftell.o:	ftell.c stdio.h
gcvt.o:	gcvt.c
calloc.o:	calloc.c stdio.h
clrerr.o:	clrerr.c stdio.h

ctype.o:	ctype.c stdio.h

portli.o:	portli.c stdio.h
