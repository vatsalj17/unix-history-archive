unix unix44 unix45 unix55 unix70:	l.o mch_id.o c.o ../sys/LIB1_id ../dev/LIB2_id
	@echo ""
	@echo "The output file will be named unix_id !!!!!"
	@echo ""
	ld -o unix_id -X -i l.o mch_id.o c.o ../sys/LIB1_id ../dev/LIB2_id
	@echo ""
	@echo "Size of unix_id is TEXT+DATA+BSS = TOTAL"
	@echo ""
	size unix_id
	rm *.o

all:	all40 all40i all70

all44 all45 all55 all70:
	cp ../h/param_id.h ../h/param.h
	cp ../dev/hk.c_id ../dev/hk.c
	cd ../sys; cc -c -O *.c; mklib_id; rm *.o
	cd ../dev; cc -c -O *.c; mklib_id; rm *.o

mch_id.o:	mch0.s mch_id.s
	as -o mch_id.o mch0.s mch_id.s

allsystems:
	@echo ""
	@echo "If not super-user, this will not work !!!!"
	@echo ""
	cat nsidconf hphtconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /hphtunix
	cat nsidconf hptmconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /hptmunix
	cat nsidconf hptsconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /hptsunix
	cat nsidconf hmhtconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /hmhtunix
	cat nsidconf hmtmconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /hmtmunix
	cat nsidconf hmtsconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /hmtsunix
	cat nsidconf rphtconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /rphtunix
	cat nsidconf rptmconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /rptmunix
	cat nsidconf rptsconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /rptsunix
	cat nsidconf hkhtconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /hkhtunix
	cat nsidconf hktmconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /hktmunix
	cat nsidconf hktsconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /hktsunix
	cat nsidconf rl01htconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /rl01htunix
	cat nsidconf rl01tmconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /rl01tmunix
	cat nsidconf rl01tsconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /rl01tsunix
	cat nsidconf rl02htconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /rl02htunix
	cat nsidconf rl02tmconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /rl02tmunix
	cat nsidconf rl02tsconf >conf_i
	mkconf <conf_i
	make unix40i
	mv unix_i /rl02tsunix

unix23i unix24i unix34i unix40i unix60i:	l.o mch_i.o c_i.o ../sys/LIB1_i ../dev/LIB2_i
	@echo ""
	@echo "The output file will be named unix_i !!!!!"
	@echo ""
	ld -o unix_i -x  l.o mch_i.o c_i.o ../sys/LIB1_i ../dev/LIB2_i
	@echo ""
	@echo "If size of unix_i > 49152 bytes, UNIX IS TOO BIG !!!!!"
	@echo ""
	@echo "Size of unix_i is TEXT+DATA+BSS = TOTAL"
	@echo ""
	size unix_i
	rm *.o

mch_i.o:	mch0.s mch_i.s
	as -o mch_i.o mch0.s mch_i.s

all23i all34i all40i all60i:
	cp ../h/param_i.h ../h/param.h
	cp ../dev/hk.c_i ../dev/hk.c
	cd ../sys ; cc -c -O *.c ; mklib_i ; rm *.o
	cd ../dev ; cc -c -O *.c ; mklib_i ; rm *.o

c_i.o:	c.c
	cp ../h/param_i.h ../h/param.h
	cc -c -O c.c
	mv c.o c_i.o
c.o:	c.c
	cp ../h/param_id.h ../h/param.h
	cc -c -O c.c

unix23 unix24 unix34 unix40 unix60:	l.o mch_ov.o c_ov.o
	@echo ""
	@echo "The output file will be named unix_ov !!!!!"
	@echo ""
	ovload
	@echo ""
	@echo "The unix_ov sizes must be within the following limits:"
	@echo ""
	@echo "root text segment > 8192 but <= 16384"
	@echo "overlay text segments <= 8192, 7 overlays maximum"
	@echo "bss + data segments <= 24576 total"
	@echo ""
	@echo "root+(overlay 1, overlay 2,...overlay n)+data+bss = root+data = (total)"
	@echo ""
	size unix_ov
	@echo ""
	rm *.o

all23 all24 all34 all40 all60:
	cp ../h/param_ov.h ../h/param.h
	cp ../dev/hk.c_id ../dev/hk.c
	cd ../ovsys; cc -c -O -V ../sys/*.c; mklib_ov
	cd ../ovdev; cc -c -O -V ../dev/*.c

c_ov.o:	c.c
	cp ../h/param_ov.h ../h/param.h
	cc -c -O -V c.c
	mv c.o c_ov.o

mch_ov.o:	mch0.s mch_ov.s
	ovas -o mch_ov.o mch0.s mch_ov.s
