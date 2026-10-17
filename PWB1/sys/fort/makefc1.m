USRF = /usr/fort
DD = tmp

LD = -ld
LDFLAGS = -s

AS = -as

.s.o:
	$(AS) -o $*.o fx/fhd.s $<


#------
# install: Create fc1 and install in $(USRF)/fc1.
#------
install: fc1
	-mv $(USRF)/fc1 $(USRF)/$(DD)fc1
	mv fc1 $(USRF)/fc1
	-chown bin $(USRF)/fc1
	-rm -f $(USRF)/$(DD)fc1
	ls -l $(USRF)/fc1


#------
# fc1: Create fc1 in current directory.
#------
fc1:	f1/f1.o f2/f2.o f3/f3.o f4/f4.o fx/fx.o
	$(LD) $(LDFLAGS) -o fc1 f1/f1.o f2/f2.o f3/f3.o f4/f4.o fx/fx.o -l
	-chmod 755 fc1


#------
# clean: Remove all created .o files.
#------
clean:
	-rm -f f1/f1.o f2/f2.o f3/f3.o f4/f4.o fx/fx.o
	-rm -f f1/f11.o f1/f12.o f1/f13.o f1/f14.o f1/f15.o f1/f16.o f1/f17.o
	-rm -f f2/f21.o f2/f22.o f2/f23.o f2/f24.o
	-rm -f f3/f31.o f3/f32.o f3/f33.o f3/f34.o f3/f35.o f3/f36.o f3/f37.o f3/f38.o f3/f39.o
	-rm -f f4/f41.o f4/f42.o f4/f43.o f4/f44.o f4/f45.o f4/f46.o f4/f47.o
	-rm -f fx/fx1.o fx/fx2.o fx/fx3.o fx/fx4.o fx/fx5.o fx/fx6.o fx/fx7.o fx/fx8.o
	-rm -f fx/fx9.o fx/fxa.o fx/fxb.o fx/fxc.o fx/fxd.o fx/fxe.o fx/fxf.o fx/fxg.o
	-rm -f fx/fxh.o fx/fxi.o fx/fxx.o


#------
# Dependencies for indiv files.
#------
f1/f1.o:	f1/f11.o f1/f12.o f1/f13.o f1/f14.o f1/f15.o f1/f16.o f1/f17.o
	$(LD) -r f1/f11.o f1/f12.o f1/f13.o f1/f14.o f1/f15.o f1/f16.o f1/f17.o -o f1/f1.o

f1/f11.o:	f1/f11.s fx/fhd.s
f1/f12.o:	f1/f12.s fx/fhd.s
f1/f13.o:	f1/f13.s fx/fhd.s
f1/f14.o:	f1/f14.s fx/fhd.s
f1/f15.o:	f1/f15.s fx/fhd.s
f1/f16.o:	f1/f16.s fx/fhd.s
f1/f17.o:	f1/f17.s fx/fhd.s

f2/f2.o:	f2/f21.o f2/f22.o f2/f23.o f2/f24.o
	$(LD) -r f2/f21.o f2/f22.o f2/f23.o f2/f24.o -o f2/f2.o

f2/f21.o:	f2/f21.s fx/fhd.s
f2/f22.o:	f2/f22.s fx/fhd.s
f2/f23.o:	f2/f23.s fx/fhd.s
f2/f24.o:	f2/f24.s fx/fhd.s

f3/f3.o: f3/f31.o f3/f32.o f3/f33.o f3/f34.o f3/f35.o f3/f36.o f3/f37.o f3/f38.o f3/f39.o
	$(LD) -r f3/f31.o f3/f32.o f3/f33.o f3/f34.o f3/f35.o f3/f36.o f3/f37.o \
			f3/f38.o f3/f39.o -o f3/f3.o

f3/f31.o:	f3/f31.s fx/fhd.s
f3/f32.o:	f3/f32.s fx/fhd.s
f3/f33.o:	f3/f33.s fx/fhd.s
f3/f34.o:	f3/f34.s fx/fhd.s
f3/f35.o:	f3/f35.s fx/fhd.s
f3/f36.o:	f3/f36.s fx/fhd.s
f3/f37.o:	f3/f37.s fx/fhd.s
f3/f38.o:	f3/f38.s fx/fhd.s
f3/f39.o:	f3/f39.s fx/fhd.s

f4/f4.o:	f4/f41.o f4/f42.o f4/f43.o f4/f44.o f4/f45.o f4/f46.o f4/f47.o
	$(LD) -r f4/f41.o f4/f42.o f4/f43.o f4/f44.o f4/f45.o f4/f46.o f4/f47.o -o f4/f4.o

f4/f41.o:	f4/f41.s fx/fhd.s
f4/f42.o:	f4/f42.s fx/fhd.s
f4/f43.o:	f4/f43.s fx/fhd.s
f4/f44.o:	f4/f44.s fx/fhd.s
f4/f45.o:	f4/f45.s fx/fhd.s
f4/f46.o:	f4/f46.s fx/fhd.s
f4/f47.o:	f4/f47.s fx/fhd.s

fx/fx.o: fx/fx1.o fx/fx2.o fx/fx3.o fx/fx4.o fx/fx5.o fx/fx6.o fx/fx7.o fx/fx8.o \
	fx/fx9.o fx/fxa.o fx/fxb.o fx/fxc.o fx/fxd.o fx/fxe.o fx/fxf.o fx/fxg.o \
	fx/fxh.o fx/fxi.o fx/fxx.o
	$(LD) -r -o fx/fx.o fx/fx1.o fx/fx2.o fx/fx3.o fx/fx4.o \
		fx/fx5.o fx/fx6.o fx/fx7.o fx/fx8.o \
		fx/fx9.o fx/fxa.o fx/fxb.o fx/fxc.o fx/fxd.o fx/fxe.o fx/fxf.o fx/fxg.o \
		fx/fxh.o fx/fxi.o fx/fxx.o

fx/fx1.o:	fx/fx1.s fx/fhd.s
fx/fx2.o:	fx/fx2.s fx/fhd.s
fx/fx3.o:	fx/fx3.s fx/fhd.s
fx/fx4.o:	fx/fx4.s fx/fhd.s
fx/fx5.o:	fx/fx5.s fx/fhd.s
fx/fx6.o:	fx/fx6.s fx/fhd.s
fx/fx7.o:	fx/fx7.s fx/fhd.s
fx/fx8.o:	fx/fx8.s fx/fhd.s
fx/fx9.o:	fx/fx9.s fx/fhd.s
fx/fxa.o:	fx/fxa.s fx/fhd.s
fx/fxb.o:	fx/fxb.s fx/fhd.s
fx/fxc.o:	fx/fxc.s fx/fhd.s
fx/fxd.o:	fx/fxd.s fx/fhd.s
fx/fxe.o:	fx/fxe.s fx/fhd.s
fx/fxf.o:	fx/fxf.s fx/fhd.s
fx/fxg.o:	fx/fxg.s fx/fhd.s
fx/fxh.o:	fx/fxh.s fx/fhd.s
fx/fxi.o:	fx/fxi.s fx/fhd.s
fx/fxx.o:	fx/fxx.s fx/fhd.s
