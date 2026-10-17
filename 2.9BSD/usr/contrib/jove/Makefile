jove:	jove_buf.o jove_case.o jove_c.o jove_curs.o jove_delete.o jove_disp.o\
	jove_draw.o jove_extend.o jove_funcs.o jove_inc.o jove_insert.o\
	jove_io.o jove_main.o jove_marks.o jove_meta.o\
	jove_misc.o jove_move.o jove_proc.o jove_re.o jove_screen.o\
	jove_tag.o jove_temp.o jove_term.o jove_ttout.o jove_wind.o\
	format.o interface.o

	/bin/ld -X -i /lib/crt0.o -o jove *.o -ljobs -ltermlib -lc

.c.o:	; cc -c -O $*.c
