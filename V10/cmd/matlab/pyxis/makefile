.SUFFIXES: .f .o
OBJS = src/ofault.o src/setjmp_.o src/sgset.o src/main.o\
	src/matlab.o src/parse.o src/comand.o src/clause.o src/expr.o\
	src/term.o src/factor.o src/funs.o src/stackp.o src/stackg.o\
	src/stack1.o src/stack2.o src/print.o src/prntid.o src/getsym.o\
	src/getlin.o src/getch.o src/getval.o src/matfn1.o src/matfn2.o\
	src/matfn3.o src/matfn4.o src/matfn5.o src/matfn6.o\
	src/error.o src/pythag.o src/rat.o\
	src/files.o src/helper.o src/flop.o src/xchar.o src/user.o src/prompt.o\
	src/savlod.o src/formz.o src/edit.o src/plot.o\
	src/wgeco.o src/wgefa.o src/wgesl.o src/wgedi.o src/wpofa.o\
	src/rref.o src/hilber.o src/htridi.o src/htribk.o src/imtql2.o\
	src/corth.o src/comqr3.o src/wsvdc.o src/wqrdc.o src/wqrsl.o\
	src/magic.o src/base.o src/urand.o src/wmul.o src/wdiv.o src/wsign.o\
	src/wsqrt.o src/wasum.o src/wlog.o src/watan.o src/iwamax.o src/wnrm2.o\
	src/wrscal.o src/wscal.o src/waxpy.o src/wdotur.o src/wdotui.o\
	src/wdotcr.o src/wdotci.o src/wcopy.o src/wset.o src/wswap.o src/rset.o\
	src/rswap.o src/rrot.o src/rrotg.o src/eqid.o src/putid.o src/round.o

.f.o:
	f77 -c -w66 -O $<
	mv *.o src

matlab:	${OBJS}
	f77 -z src/*.o -o matlab
