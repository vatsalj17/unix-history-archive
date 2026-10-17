all:	cmd40 cmd70

cmd40:
	@echo ""
	@echo "If not super-user, this will not work !"
	@echo ""
	cp /sys/h/param_ov.h /usr/include/sys/param.h
	cmake -cp dcheck40
	cmake -cp dump40
	cmake -cp dumpdir40
	cmake -cp icheck40
	cmake -cp ncheck40
	cmake -cp restor40

cmd70:
	@echo ""
	@echo "If not super-user, this will not work !"
	@echo ""
	cp /sys/h/param_id.h /usr/include/sys/param.h
	cmake dcheck; mv dcheck /bin/dcheck70
	cmake dump; mv dump /bin/dump70
	cmake dumpdir; mv dumpdir /bin/dumpdir70
	cmake icheck; mv icheck /bin/icheck70
	cmake ncheck; mv ncheck /bin/ncheck70
	cmake restor; mv restor /bin/restor70
	chown bin /bin/*70

cmd40i:
	@echo ""
	@echo "If not super-user, this will not work !"
	@echo ""
	cp /sys/h/param_i.h /usr/include/sys/param.h
	cmake -cp dcheck40
	cmake -cp dump40
	cmake -cp dumpdir40
	cmake -cp icheck40
	cmake -cp ncheck40
	cmake -cp restor40
