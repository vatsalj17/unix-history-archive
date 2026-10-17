#
#  Makefile for the INGRES control module
#
#	@(#)Makefile	8.4	4/22/86
#

INGRES=	../..
H=	../h
LIB=	$(INGRES)/lib

OBJS=	main.o error.o proc_err.o initp.o setp.o resetp.o \
	init_qt.o call.o sysdump.o mapvars.o \
	do_seq.o readinput.o do_st.o send_off.o call_fn.o \
	readmon.o readqry.o writeqry.o range.o treepr.o \
	pb_prime.o pb_get.o pb_tput.o pb_put.o pb_flush.o \
	pb_read.o pb_write.o pb_rphys.o pb_wphys.o pb_dump.o \
	cm_reset.o cm_cleanup.o cm_close.o \
	prvect.o getp.o monitor.o av_files.o
HDRS=	pipes.h proc.h state.h ctlmod.h 
SRCS1=	main.c error.c proc_err.c initp.c setp.c resetp.c \
	init_qt.c call.c sysdump.c mapvars.c \
	do_seq.c readinput.c do_st.c send_off.c call_fn.c \
	readmon.c readqry.c writeqry.c range.c treepr.c
SRCS2=	pb_prime.c pb_get.c pb_tput.c pb_put.c pb_flush.c \
	pb_read.c pb_write.c pb_rphys.c pb_wphys.c pb_dump.c \
	cm_reset.c cm_cleanup.c cm_close.c \
	prvect.c getp.c monitor.c av_files.c

CFLAGS=	-O -I$H
LINTFLAGS= -huc -I$H
LINT=	lint
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Control Module
CTAGS=	ctags
AR=	-ar
ARFLAGS=rvu
RANLIB=	ranlib
GET=	sccs get
CP=	cp
CHMOD=	chmod
LIBMODE=664

all: ctlmod

ctlmod: $(OBJS)
	$(AR) $(ARFLAGS) ctlmod $?
	$(CHMOD) $(LIBMODE) ctlmod
	$(RANLIB) ctlmod

install: $(LIB)/ctlmod

$(LIB)/ctlmod: ctlmod
	$(CP) ctlmod $(LIB)/ctlmod
	$(RANLIB) $(LIB)/ctlmod

new:
	rm -f ctlmod

clean:
	rm -f *.o
	rm -f a.out ctlmod core

sources: $(HDRS) $(SRCS1) $(SRCS2)

$(HDRS):
	$(GET) $(REL) $@
$(SRCS1):
	$(GET) $(REL) $@
$(SRCS2):
	$(GET) $(REL) $@

ctags:
	$(CTAGS) *.c

print: sources
	$(PR) Makefile TraceFlags *.[hm] *.[csy]

lint:
	$(LINT) $(LINTFLAGS) $(SRCS1) $(SRCS2)

vgrind: sources
	cp /dev/null index
	$(VGRIND) -h "$(HEADER) (Release 8)" -n Makefile TraceFlags
	$(VGRIND) -h "$(HEADER) (Release 8)" *.[hm] *.[csy]
	sed /SCCSID/d < index > tmp
	mv tmp index
	$(VGRIND) -h "$(HEADER) (Release 8)" -x index

tags: sources
	-fgrep ../ tags > othertags
	ctags  $(SRCS) $(HDRS)
	sort -o tags tags othertags
	-rm -f othertags

#
# DANGER	DANGER	DANGER	DANGER	DANGER	DANGER
# The following two entries should NOT be executed by the user.
# These entries are only for the Makefile in conf to construct
# the system tag files.
#
systags: 
	ctags $(SRCS1) $(SRCS2) $(HDRS)
	sed "s/	/	ctlmod\//" tags > systags
	cat systags >> ../tags

#
# We assume that systags has already been done.
#
installtags:
	sed "/	ctlmod/d" ../tags | sed "s/	/	..\//" > othertags
	sort -o tags tags othertags
	-rm -f othertags systags


depend:
	grep '^#[ 	]*include' $(SRCS1) $(SRCS2) | sed -n -f ../h/depend.sed > makedep
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

main.o:ctlmod.h
main.o:pipes.h
main.o:$H/resp.h
main.o:$H/ingres.h
main.o:$H/aux.h
main.o:$H/lock.h
main.o:$H/sccs.h
error.o:ctlmod.h
error.o:pipes.h
error.o:$H/pv.h
error.o:$H/sccs.h
proc_err.o:ctlmod.h
proc_err.o:pipes.h
proc_err.o:$H/sccs.h
initp.o:ctlmod.h
initp.o:$H/ingres.h
initp.o:$H/aux.h
initp.o:$H/tree.h
initp.o:$H/sccs.h
setp.o:ctlmod.h
setp.o:$H/ingres.h
setp.o:$H/tree.h
setp.o:$H/aux.h
setp.o:$H/sccs.h
resetp.o:ctlmod.h
resetp.o:$H/ingres.h
resetp.o:$H/tree.h
resetp.o:$H/sccs.h
init_qt.o:ctlmod.h
init_qt.o:$H/ingres.h
init_qt.o:$H/aux.h
init_qt.o:$H/tree.h
init_qt.o:$H/sccs.h
call.o:ctlmod.h
call.o:pipes.h
call.o:$H/resp.h
call.o:$H/ingres.h
call.o:$H/aux.h
call.o:$H/tree.h
call.o:$H/sccs.h
sysdump.o:ctlmod.h
sysdump.o:$H/tree.h
sysdump.o:$H/sccs.h
mapvars.o:$H/ingres.h
mapvars.o:$H/symbol.h
mapvars.o:$H/tree.h
mapvars.o:$H/sccs.h
do_seq.o:ctlmod.h
do_seq.o:pipes.h
do_seq.o:$H/resp.h
do_seq.o:$H/sccs.h
readinput.o:ctlmod.h
readinput.o:pipes.h
readinput.o:$H/resp.h
readinput.o:$H/ingres.h
readinput.o:$H/aux.h
readinput.o:$H/tree.h
readinput.o:$H/sccs.h
do_st.o:ctlmod.h
do_st.o:pipes.h
do_st.o:$H/sccs.h
send_off.o:ctlmod.h
send_off.o:pipes.h
send_off.o:$H/symbol.h
send_off.o:$H/tree.h
send_off.o:$H/resp.h
send_off.o:$H/sccs.h
call_fn.o:ctlmod.h
call_fn.o:$H/resp.h
call_fn.o:$H/sccs.h
readmon.o:ctlmod.h
readmon.o:pipes.h
readmon.o:$H/sccs.h
readqry.o:ctlmod.h
readqry.o:$H/ingres.h
readqry.o:$H/aux.h
readqry.o:$H/tree.h
readqry.o:$H/symbol.h
readqry.o:$H/sccs.h
readqry.o:$H/errors.h
writeqry.o:$H/ingres.h
writeqry.o:$H/aux.h
writeqry.o:$H/tree.h
writeqry.o:$H/symbol.h
writeqry.o:$H/sccs.h
range.o:$H/ingres.h
range.o:$H/symbol.h
range.o:$H/range.h
range.o:$H/tree.h
range.o:$H/sccs.h
range.o:$H/errors.h
treepr.o:$H/ingres.h
treepr.o:$H/symbol.h
treepr.o:$H/tree.h
treepr.o:$H/aux.h
treepr.o:$H/sccs.h
pb_prime.o:ctlmod.h
pb_prime.o:pipes.h
pb_prime.o:$H/sccs.h
pb_get.o:ctlmod.h
pb_get.o:pipes.h
pb_get.o:$H/sccs.h
pb_tput.o:ctlmod.h
pb_tput.o:pipes.h
pb_tput.o:$H/sccs.h
pb_put.o:ctlmod.h
pb_put.o:pipes.h
pb_put.o:$H/sccs.h
pb_flush.o:ctlmod.h
pb_flush.o:pipes.h
pb_flush.o:$H/sccs.h
pb_read.o:ctlmod.h
pb_read.o:pipes.h
pb_read.o:$H/sccs.h
pb_write.o:ctlmod.h
pb_write.o:pipes.h
pb_write.o:$H/sccs.h
pb_rphys.o:pipes.h
pb_rphys.o:$H/sccs.h
pb_wphys.o:$H/useful.h
pb_wphys.o:pipes.h
pb_wphys.o:$H/sccs.h
pb_dump.o:pipes.h
pb_dump.o:$H/sccs.h
cm_reset.o:ctlmod.h
cm_reset.o:$H/sccs.h
cm_cleanup.o:ctlmod.h
cm_cleanup.o:$H/sccs.h
cm_close.o:ctlmod.h
cm_close.o:$H/sccs.h
prvect.o:ctlmod.h
prvect.o:$H/tree.h
prvect.o:$H/pv.h
prvect.o:$H/sccs.h
getp.o:ctlmod.h
getp.o:$H/pv.h
getp.o:$H/sccs.h
monitor.o:$H/useful.h
monitor.o:$H/opsys.h
monitor.o:$H/pmon.h
monitor.o:$H/sccs.h
av_files.o:$H/ingres.h
av_files.o:$H/aux.h
av_files.o:$H/opsys.h
av_files.o:ctlmod.h
av_files.o:$H/sccs.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
