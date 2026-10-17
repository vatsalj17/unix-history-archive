#  To generate real floating point simulator, instead of fake,
#  change the  "fp.o:"  dependency lines below to:
#fp.o:	fp1.s fp2.s fp3.s fpx.s
#	cat fp1.s fp2.s fp3.s fpx.s > xx.s
#	as -o fp.o xx.s
#	ld -r $(LDFLAGS) fp.o
#	mv a.out fp.o
#	rm xx.s

LIB = /lib
DD = tmp

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
# liba.a: Create liba.a in local directory.
#------
liba.a:	get.o put.o atan.o mesg.o sin.o switch.o sqrt.o ttyn.o \
	rand.o crypt.o ecvt.o pow.o exp.o log.o ldiv.o dpadd.o \
	fp.o gamma.o floor.o fmod.o savr5.o
	-rm -f liba.a tmp.a
	$(AR) r tmp.a get.o put.o atan.o mesg.o sin.o switch.o sqrt.o ttyn.o
	$(AR) r tmp.a rand.o crypt.o ecvt.o pow.o exp.o log.o ldiv.o dpadd.o
	$(AR) r tmp.a fp.o gamma.o floor.o fmod.o savr5.o
	-chmod 644 tmp.a
	mv tmp.a liba.a


#------
# install: Create liba.a, if it isn't already, and move it to $(LIB)/liba.a.
#------
install: liba.a
	-mv $(LIB)/liba.a $(LIB)/$(DD)liba.a
	mv liba.a $(LIB)/liba.a
	-chown bin $(LIB)/liba.a
	-rm -f $(LIB)/$(DD)liba.a
	ls -l $(LIB)/liba.a


#------
# clean: Remove all .o files created for liba.a.
#------
clean:
	-rm -f get.o put.o atan.o mesg.o sin.o switch.o sqrt.o ttyn.o
	-rm -f rand.o crypt.o ecvt.o pow.o exp.o log.o ldiv.o dpadd.o
	-rm -f fp.o gamma.o floor.o fmod.o savr5.o


#------
# Dependencies for individual .o files.
#------
get.o:	get.s
put.o:	put.s
atan.o:	atan.s
mesg.o:	mesg.s
sin.o:	sin.s
switch.o:	switch.s
sqrt.o:	sqrt.s
ttyn.o:	ttyn.s

rand.o:	rand.s
crypt.o: crypt.c
ecvt.o:	ecvt.s
pow.o:	pow.s
exp.o:	exp.s
log.o:	log.s
ldiv.o:	ldiv.s
dpadd.o:	dpadd.s

fp.o:	fakfp.s
	$(AS) -o fp.o fakfp.s
	$(LD) -r $(LDFLAGS) fp.o
	mv a.out fp.o

gamma.o:	gamma.s
floor.o:	floor.s
fmod.o:	fmod.s
savr5.o:	savr5.s
