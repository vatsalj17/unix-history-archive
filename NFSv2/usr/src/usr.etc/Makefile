#
# @(#)Makefile nfs.1.0 85/06/28 SAI; 1.1 85/05/31 SMI; from UCB 4.27 83/07/06
#
DESTDIR=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	

# Shell scripts that need only be installed and are never removed.
#
SCRIPT=

# C programs that live in the current directory and do not need
# explicit make lines.
#
STD=	analyze catman  dcheck dumpfs \
	icheck mkproto \
	ncheck nfsstat quot \
	rpcinfo rpc.rstatd rpc.rwalld rpc.mountd \
	rpc.yppasswdd savecore 

# Programs that must run setuid to root
#
SETUID=	

# C programs that live in the current directory and need explicit make lines.
#
NSTD=

all:	${SUBDIR} ${STD} ${NSTD}

#${SUBDIR}: FRC
#	cd $@; make ${MFLAGS}

${STD}:
	cc ${CFLAGS} -o $@ $@.c

install: ${STD} ${NSTD}
	-mkdir ${DESTDIR}/usr/etc && chown bin ${DESTDIR}/usr/etc && \
		chmod 755 ${DESTDIR}/usr/etc
#	for i in ${SUBDIR}; do \
#		(cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); done
#	for i in ${SCRIPT}; do (install -c $$i.sh ${DESTDIR}/usr/etc/$$i); done
	for i in ${STD} ${NSTD}; do (install -s $$i ${DESTDIR}/usr/etc/$$i); done
#	rm -f ${DESTDIR}/usr/etc/quotaoff
#	for i in ${SETUID}; do (chown root ${DESTDIR}/usr/etc/$$i; \
#		chmod 4755 ${DESTDIR}/usr/etc/$$i); done
#	ln ${DESTDIR}/usr/etc/quotaon ${DESTDIR}/usr/etc/quotaoff

clean:
	rm -f a.out core *.s *.o
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done
	rm -f ${STD} ${NSTD}

FRC:

depend:
	cat </dev/null >x.c
	for i in ${STD} ${NSTD}; do \
		(echo $$i: $$i.c >>makedep; \
		/bin/grep '^#[ 	]*include' x.c $$i.c | sed \
			-e '/\.\.\/h/d' \
			-e '/\.\.\/ufs/d' \
			-e 's,<\(.*\)>,"/usr/include/\1",' \
			-e 's/:[^"]*"\([^"]*\)".*/: \1/' \
			-e 's/\.c//' >>makedep); done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep x.c
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# Files listed in ${NSTD} have explicit make lines given below.

# DO NOT DELETE THIS LINE -- make depend uses it

analyze: analyze.c
analyze: /usr/include/stdio.h
analyze: /usr/include/sys/param.h
analyze: /usr/include/sys/dir.h
analyze: /usr/include/machine/pte.h
analyze: /usr/include/nlist.h
analyze: /usr/include/sys/map.h
analyze: /usr/include/sys/user.h
analyze: /usr/include/sys/proc.h
analyze: /usr/include/sys/text.h
analyze: /usr/include/sys/cmap.h
analyze: /usr/include/sys/vm.h
catman: catman.c
catman: /usr/include/stdio.h
catman: /usr/include/sys/types.h
catman: /usr/include/sys/stat.h
catman: /usr/include/sys/time.h
catman: /usr/include/sys/dir.h
catman: /usr/include/ctype.h
dcheck: dcheck.c
dcheck: /usr/include/sys/param.h
dcheck: /usr/include/sys/time.h
dcheck: /usr/include/sys/vnode.h
dcheck: /usr/include/ufs/inode.h
dcheck: /usr/include/ufs/fs.h
dcheck: /usr/include/sys/dir.h
dcheck: /usr/include/stdio.h
dumpfs: dumpfs.c
dumpfs: /usr/include/sys/param.h
dumpfs: /usr/include/sys/time.h
dumpfs: /usr/include/sys/vnode.h
dumpfs: /usr/include/ufs/inode.h
dumpfs: /usr/include/ufs/fs.h
dumpfs: /usr/include/stdio.h
dumpfs: /usr/include/fstab.h
icheck: icheck.c
icheck: /usr/include/stdio.h
icheck: /usr/include/sys/param.h
icheck: /usr/include/sys/time.h
icheck: /usr/include/sys/vnode.h
icheck: /usr/include/ufs/inode.h
icheck: /usr/include/ufs/fs.h
mkproto: mkproto.c
mkproto: /usr/include/stdio.h
mkproto: /usr/include/sys/param.h
mkproto: /usr/include/sys/time.h
mkproto: /usr/include/sys/vnode.h
mkproto: /usr/include/ufs/inode.h
mkproto: /usr/include/ufs/fs.h
mkproto: /usr/include/ufs/fsdir.h
ncheck: ncheck.c
ncheck: /usr/include/sys/param.h
ncheck: /usr/include/sys/time.h
ncheck: /usr/include/sys/vnode.h
ncheck: /usr/include/ufs/inode.h
ncheck: /usr/include/ufs/fs.h
ncheck: /usr/include/sys/dir.h
ncheck: /usr/include/stdio.h
nfsstat: nfsstat.c
nfsstat: /usr/include/stdio.h
nfsstat: /usr/include/sys/types.h
nfsstat: /usr/include/sys/socket.h
nfsstat: /usr/include/net/if.h
nfsstat: /usr/include/netinet/in.h
nfsstat: /usr/include/netinet/if_ether.h
nfsstat: /usr/include/nlist.h
quot: quot.c
quot: /usr/include/stdio.h
quot: /usr/include/ctype.h
quot: /usr/include/sys/param.h
quot: /usr/include/sys/time.h
quot: /usr/include/sys/vnode.h
quot: /usr/include/ufs/inode.h
quot: /usr/include/ufs/fs.h
quot: /usr/include/sys/file.h
quot: /usr/include/fstab.h
quot: /usr/include/pwd.h
quot: /usr/include/utmp.h
rpcinfo: rpcinfo.c
rpcinfo: /usr/include/rpc/rpc.h
rpcinfo: /usr/include/stdio.h
rpcinfo: /usr/include/sys/socket.h
rpcinfo: /usr/include/sys/time.h
rpcinfo: /usr/include/netdb.h
rpcinfo: /usr/include/rpc/pmap_prot.h
rpcinfo: /usr/include/rpc/pmap_clnt.h
rpc.rstatd: rpc.rstatd.c
rpc.rstatd: /usr/include/signal.h
rpc.rstatd: /usr/include/stdio.h
rpc.rstatd: /usr/include/rpc/rpc.h
rpc.rstatd: /usr/include/sys/socket.h
rpc.rstatd: /usr/include/nlist.h
rpc.rstatd: /usr/include/sys/dk.h
rpc.rstatd: /usr/include/sys/errno.h
rpc.rstatd: /usr/include/sys/vmmeter.h
rpc.rstatd: /usr/include/net/if.h
rpc.rstatd: /usr/include/sys/time.h
rpc.rstatd: /usr/include/rpcsvc/rstat.h
rpc.rwalld: rpc.rwalld.c
rpc.rwalld: /usr/include/rpcsvc/netwall.h
rpc.rwalld: /usr/include/rpc/rpc.h
rpc.rwalld: /usr/include/stdio.h
rpc.rwalld: /usr/include/netdb.h
rpc.rwalld: /usr/include/sys/socket.h
rpc.mountd: rpc.mountd.c
rpc.mountd: /usr/include/sys/param.h
rpc.mountd: /usr/include/ufs/fs.h
rpc.mountd: /usr/include/rpc/rpc.h
rpc.mountd: /usr/include/sys/stat.h
rpc.mountd: /usr/include/sys/socket.h
rpc.mountd: /usr/include/sys/file.h
rpc.mountd: /usr/include/sys/time.h
rpc.mountd: /usr/include/stdio.h
rpc.mountd: /usr/include/signal.h
rpc.mountd: /usr/include/sys/wait.h
rpc.mountd: /usr/include/sys/ioctl.h
rpc.mountd: /usr/include/sys/errno.h
rpc.mountd: /usr/include/nfs/nfs.h
rpc.mountd: /usr/include/rpcsvc/mount.h
rpc.mountd: /usr/include/netdb.h
rpc.yppasswdd: rpc.yppasswdd.c
rpc.yppasswdd: /usr/include/stdio.h
rpc.yppasswdd: /usr/include/signal.h
rpc.yppasswdd: /usr/include/sys/file.h
rpc.yppasswdd: /usr/include/rpc/rpc.h
rpc.yppasswdd: /usr/include/pwd.h
rpc.yppasswdd: /usr/include/rpcsvc/yppasswd.h
rpc.yppasswdd: /usr/include/errno.h
rpc.yppasswdd: /usr/include/sys/socket.h
savecore: savecore.c
savecore: /usr/include/stdio.h
savecore: /usr/include/nlist.h
savecore: /usr/include/sys/param.h
savecore: /usr/include/sys/dir.h
savecore: /usr/include/sys/stat.h
savecore: /usr/include/sys/vfs.h
savecore: /usr/include/sys/time.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
