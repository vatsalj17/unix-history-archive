#
#       Makefile nfs.1.0 85/06/27 SAI; from 1.1 85/05/30
#

SRC=	getnetgrent.c innetgr.c mountxdr.c \
	rstatxdr.c rusersxdr.c\
	ypprot_err.c yp_bind.c yperr_string.c clnt_domatch.c\
	yp_enum.c _ypclnt_enum.c yp_match.c ypxdr.c yppasswdxdr.c
OBJS=	getnetgrent.o innetgr.o mountxdr.o \
	rstatxdr.o rusersxdr.o\
	ypprot_err.o yp_bind.o yperr_string.o clnt_domatch.o\
	yp_enum.o _ypclnt_enum.o yp_match.o ypxdr.o yppasswdxdr.o
INC=	mount.h rstat.h rusers.h yp_prot.h ypclnt.h netwall.h yppasswd.h

CFLAGS= -O -Dvax

.c.o:
	${CC} -p -c ${CFLAGS} $*.c
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

rpcsvclib rpcsvclib_p: ${OBJS}
	@echo "building profiled rpcsvclib"
	@cd profiled; ar cru ../rpcsvclib_p ${OBJS}
	@echo "building normal rpcsvclib"
	@ar cru rpcsvclib ${OBJS}

tags: $(SRC) $(KSRC) $(INC)
	ctags -tw $(SRC) $(KSRC) $(INC)

ref: tags
	sed 's,	/.*,,' tags | \
	awk ' { printf("%-26s%-16s%s\n", $$1, $$2, $$3) }' > ref

lint:
	lint -bnuvx $(SRC)

print:
	pr $(INC) $(SRC) $(KSRC) | lpr -Pvp

clean:
	rm -f $(OBJS) rpcsvclib linted made profiled/*.o rpcsvclib_p
