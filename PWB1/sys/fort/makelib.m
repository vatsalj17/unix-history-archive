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

FC = fc

.f.o:
	$(FC) -c $<
	$(LD) -r $(LDFLAGS) $*.o
	mv a.out $*.o

AR = ar


#------
# libf.a: Create libf.a in local directory, from rt1 and rt2 directories.
#------
libf.a:	rt1/abs.o rt1/aimag.o rt1/aint.o rt1/alog10.o \
	rt1/amax0.o rt1/amax1.o rt1/amin0.o rt1/amin1.o \
	rt1/amod.o rt1/atan.o rt1/ccos.o rt1/cexp.o rt1/clog.o \
	rt1/cmplx.o rt1/conjg.o rt1/csin.o rt1/csqrt.o

libf.a:	rt1/dble.o rt1/dccos.o rt1/dcexp.o rt1/dclog.o \
	rt1/dcsin.o rt1/dcsqrt.o rt1/dim.o rt1/dimag.o \
	rt1/float.o rt1/iabs.o rt1/idim.o rt1/idint.o \
	rt1/ierr.o rt1/ifix.o rt1/isign.o

libf.a:	rt1/mod.o rt1/real.o rt1/sign.o rt1/sngl.o rt1/tanh.o \
	rt1/alog.o rt1/atan2.o rt1/cabs.o rt1/cos.o \
	rt1/exp.o rt1/sin.o rt1/sqrt.o \
	rt2/ctime.o rt2/getarg.o rt2/nice.o \
	rt2/rand.o rt2/setfil.o rt2/uio.o

libf.a:
	-rm -f libf.a tmp.a
	$(AR) r tmp.a rt1/abs.o rt1/aimag.o rt1/aint.o rt1/alog10.o \
			rt1/amax0.o rt1/amax1.o rt1/amin0.o rt1/amin1.o
	$(AR) r tmp.a rt1/amod.o rt1/atan.o rt1/ccos.o rt1/cexp.o rt1/clog.o \
			rt1/cmplx.o rt1/conjg.o rt1/csin.o rt1/csqrt.o
	$(AR) r tmp.a rt1/dble.o rt1/dccos.o rt1/dcexp.o rt1/dclog.o \
			rt1/dcsin.o rt1/dcsqrt.o rt1/dim.o rt1/dimag.o
	$(AR) r tmp.a rt1/float.o rt1/iabs.o rt1/idim.o rt1/idint.o \
			rt1/ierr.o rt1/ifix.o rt1/isign.o
	$(AR) r tmp.a rt1/mod.o rt1/real.o rt1/sign.o rt1/sngl.o rt1/tanh.o
	$(AR) r tmp.a rt1/alog.o rt1/atan2.o rt1/cabs.o rt1/cos.o \
			rt1/exp.o rt1/sin.o rt1/sqrt.o
	$(AR) r tmp.a rt2/ctime.o rt2/getarg.o rt2/nice.o \
			rt2/rand.o rt2/setfil.o rt2/uio.o
	-chmod 644 tmp.a
	mv tmp.a libf.a


#------
# filib.a: Create filib.a in local directory, from io & rt directories.
#------
filib.a: rt/r1.o rt/r2.o rt/r3.o rt/r4.o rt/r5.o rt/r6.o rt/r7.o rt/r8.o rt/r9.o \
	rt/ra.o rt/rb.o rt/rc.o rt/rd.o rt/re.o rt/rf.o rt/rg.o rt/rh.o rt/rx.o \
	io/io.o

filib.a:
	-rm -f filib.a tmp.a
	$(AR) r tmp.a io/io.o \
		rt/r1.o rt/r2.o rt/r3.o rt/r4.o rt/r5.o rt/r6.o rt/r7.o rt/r8.o rt/r9.o
	$(AR) r tmp.a rt/ra.o rt/rb.o rt/rc.o rt/rd.o rt/re.o rt/rf.o rt/rg.o rt/rh.o rt/rx.o
	-chmod 644 tmp.a
	mv tmp.a filib.a


#------
# fr0.o: Create fr0.0 in local directory, from rt/r0.o.
#------
fr0.o:	rt/r0.o
	cp rt/r0.o fr0.o
	-chmod 644 fr0.o


#------
# install: Create libf.a, if it isn't already, and move it to $(LIB)/libf.a.
#	   Also create and install filib.a & fr0.0.
#------
install: libf.a filib.a fr0.o
	-mv $(LIB)/libf.a $(LIB)/$(DD)libf.a
	-mv $(LIB)/filib.a $(LIB)/$(DD)filib.a
	-mv $(LIB)/fr0.o $(LIB)/$(DD)fr0.o
	mv libf.a $(LIB)/libf.a
	mv filib.a $(LIB)/filib.a
	mv fr0.o $(LIB)/fr0.o
	-chown bin $(LIB)/libf.a $(LIB)/filib.a $(LIB)/fr0.o
	-rm -f $(LIB)/$(DD)libf.a $(LIB)/$(DD)filib.a $(LIB)/$(DD)fr0.o
	ls -l $(LIB)/libf.a $(LIB)/filib.a $(LIB)/fr0.o


#------
# clean: Remove all created .o files.
#------
clean:
	-rm -f rt1/abs.o rt1/aimag.o rt1/aint.o rt1/alog10.o \
			rt1/amax0.o rt1/amax1.o rt1/amin0.o rt1/amin1.o
	-rm -f rt1/amod.o rt1/atan.o rt1/ccos.o rt1/cexp.o rt1/clog.o \
			rt1/cmplx.o rt1/conjg.o rt1/csin.o rt1/csqrt.o
	-rm -f rt1/dble.o rt1/dccos.o rt1/dcexp.o rt1/dclog.o \
			rt1/dcsin.o rt1/dcsqrt.o rt1/dim.o rt1/dimag.o
	-rm -f rt1/float.o rt1/iabs.o rt1/idim.o rt1/idint.o \
			rt1/ierr.o rt1/ifix.o rt1/isign.o
	-rm -f rt1/mod.o rt1/real.o rt1/sign.o rt1/sngl.o rt1/tanh.o
	-rm -f rt1/alog.o rt1/atan2.o rt1/cabs.o rt1/cos.o \
			rt1/exp.o rt1/sin.o rt1/sqrt.o
	-rm -f rt2/ctime.o rt2/getarg.o rt2/nice.o \
			rt2/rand.o rt2/setfil.o rt2/uio.o
	-rm -f rt/r1.o rt/r2.o rt/r3.o rt/r4.o rt/r5.o rt/r6.o rt/r7.o rt/r8.o rt/r9.o
	-rm -f rt/ra.o rt/rb.o rt/rc.o rt/rd.o rt/re.o rt/rf.o rt/rg.o rt/rh.o rt/rx.o
	-rm -f io/io.o rt/r0.o


#------
# Dependencies for indiv files.
#------
rt1/abs.o:	rt1/abs.s
rt1/aimag.o:	rt1/aimag.s
rt1/aint.o:	rt1/aint.s
rt1/alog10.o:	rt1/alog10.s
rt1/amax0.o:	rt1/amax0.s
rt1/amax1.o:	rt1/amax1.s
rt1/amin0.o:	rt1/amin0.s
rt1/amin1.o:	rt1/amin1.s

rt1/amod.o:	rt1/amod.s
rt1/atan.o:	rt1/atan.s
rt1/ccos.o:	rt1/ccos.f
rt1/cexp.o:	rt1/cexp.f
rt1/clog.o:	rt1/clog.f
rt1/cmplx.o:	rt1/cmplx.s
rt1/conjg.o:	rt1/conjg.s
rt1/csin.o:	rt1/csin.f
rt1/csqrt.o:	rt1/csqrt.f

rt1/dble.o:	rt1/dble.s
rt1/dccos.o:	rt1/dccos.f
rt1/dcexp.o:	rt1/dcexp.f
rt1/dclog.o:	rt1/dclog.f
rt1/dcsin.o:	rt1/dcsin.f
rt1/dcsqrt.o:	rt1/dcsqrt.f
rt1/dim.o:	rt1/dim.s
rt1/dimag.o:	rt1/dimag.s

rt1/float.o:	rt1/float.s
rt1/iabs.o:	rt1/iabs.s
rt1/idim.o:	rt1/idim.s
rt1/idint.o:	rt1/idint.s
rt1/ierr.o:	rt1/ierr.s
rt1/ifix.o:	rt1/ifix.s
rt1/isign.o:	rt1/isign.s

rt1/mod.o:	rt1/mod.s
rt1/real.o:	rt1/real.s
rt1/sign.o:	rt1/sign.s
rt1/sngl.o:	rt1/sngl.s
rt1/tanh.o:	rt1/tanh.f

rt1/alog.o:	rt1/alog.s
rt1/atan2.o:	rt1/atan2.s
rt1/cabs.o:	rt1/cabs.s
rt1/cos.o:	rt1/cos.s
rt1/exp.o:	rt1/exp.s
rt1/sin.o:	rt1/sin.s
rt1/sqrt.o:	rt1/sqrt.s

rt2/ctime.o: rt2/ctime.s
rt2/getarg.o: rt2/getarg.s
rt2/nice.o: rt2/nice.s
rt2/rand.o: rt2/rand.s
rt2/setfil.o: rt2/setfil.s
rt2/uio.o: rt2/uio.s

rt/r0.o:	rt/r0.s

rt/r1.o:	rt/r1.s
rt/r2.o:	rt/r2.s
rt/r3.o:	rt/r3.s
rt/r4.o:	rt/r4.s
rt/r5.o:	rt/r5.s
rt/r6.o:	rt/r6.s
rt/r7.o:	rt/r7.s
rt/r8.o:	rt/r8.s
rt/r9.o:	rt/r9.s
rt/ra.o:	rt/ra.s
rt/rb.o:	rt/rb.s
rt/rc.o:	rt/rc.s
rt/rd.o:	rt/rd.s
rt/re.o:	rt/re.s
rt/rf.o:	rt/rf.s
rt/rg.o:	rt/rg.s
rt/rh.o:	rt/rh.s
rt/rx.o:	rt/rx.s

io/io.o:	io/io1.s io/io2.s io/io3.s io/io4.s io/io5.s io/io6.s io/io7.s io/iox.s
	cat io/io1.s io/io2.s io/io3.s io/io4.s io/io5.s io/io6.s io/io7.s io/iox.s \
		> io/xxio.s
	$(AS) -o io/io.o io/xxio.s
	$(LD) -r $(LDFLAGS) io/io.o
	mv a.out io/io.o
	rm -f io/xxio.s
