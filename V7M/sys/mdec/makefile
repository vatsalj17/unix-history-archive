sumsg:
	@echo ""
	@echo "Must be super-user or this will not work !!!!"
	@echo ""

all: sumsg hkub hpub rlub rpub

hpuboot: sumsg hpub

hpub:
	as hpuboot.s
	strip a.out
	dd if=a.out of=/mdec/hpuboot bs=16 skip=1
	chmod 600 /mdec/hpuboot
	rm a.out

rpuboot: sumsg rpub

rpub:
	as rpuboot.s
	strip a.out
	dd if=a.out of=/mdec/rpuboot bs=16 skip=1
	chmod 600 /mdec/rpuboot
	rm a.out

hkuboot: sumsg hkub

hkub:
	as hkuboot.s
	strip a.out
	dd if=a.out of=/mdec/hkuboot bs=16 skip=1
	chmod 600 /mdec/hkuboot
	rm a.out

rluboot: sumsg rlub

rlub:
	as rluboot.s
	strip a.out
	dd if=a.out of=/mdec/rluboot bs=16 skip=1
	chmod 600 /mdec/rluboot
	rm a.out
