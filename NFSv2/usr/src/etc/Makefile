#	Makefile nfs.1.0 85/06/28 SAI; from 4.29 83/08/28
#
DESTDIR=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	config dump dump.4.1 fsck ftpd getty htable implog restore \
	routed rwhod termcap yp ypserv

# Shell scripts that need only be installed and are never removed.
#
SCRIPT=	fastboot fasthalt mklost+found

# C programs that live in the current directory and do not need
# explicit make lines.
#
STD=	ac accton arff bad144 badsect biod catman chown clri comsat \
	cron diskpart dmesg edquota flcopy fsirand \
	gettable halt ifconfig in.rlogind inetd init kgmon \
	newfs nfsd mkfs mknod mount \
	portmap pstat quotaon quotacheck reboot renice repquota \
	rexecd rmt route rshd rwall rxformat sa showmount \
	shutdown swapon telnetd tftpd trpt tunefs umount update vipw ypbind

# C programs that live in the current directory and need explicit make lines.
#
NSTD=

all:	${SUBDIR} ${STD} ${NSTD}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS}

${STD}:
	cc ${CFLAGS} -o $@ $@.c

install:
	for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); done
	for i in ${SCRIPT}; do (install -c $$i.sh ${DESTDIR}/etc/$$i); done
	for i in ${STD} ${NSTD}; do (install $$i ${DESTDIR}/etc/$$i); done
	rm -f ${DESTDIR}/etc/quotaoff
	ln ${DESTDIR}/etc/quotaon ${DESTDIR}/etc/quotaoff

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

ac: ac.c
ac: /usr/include/stdio.h
ac: /usr/include/ctype.h
ac: /usr/include/utmp.h
ac: /usr/include/sys/time.h
ac: /usr/include/sys/types.h
ac: /usr/include/sys/timeb.h
accton: accton.c
arff: arff.c
arff: /usr/include/sys/types.h
arff: /usr/include/sys/stat.h
arff: /usr/include/sys/time.h
arff: /usr/include/signal.h
arff: /usr/include/stdio.h
arff: /usr/include/sys/file.h
bad144: bad144.c
bad144: /usr/include/sys/types.h
bad144: /usr/include/sys/dkbad.h
bad144: /usr/include/sys/ioctl.h
bad144: /usr/include/sys/file.h
bad144: /usr/include/machine/dkio.h
bad144: /usr/include/stdio.h
bad144: /usr/include/disktab.h
badsect: badsect.c
badsect: /usr/include/stdio.h
badsect: /usr/include/sys/param.h
badsect: /usr/include/ufs/fs.h
badsect: /usr/include/sys/dir.h
badsect: /usr/include/sys/stat.h
badsect: /usr/include/ufs/inode.h
biod: biod.c
biod: /usr/include/stdio.h
catman: catman.c
catman: /usr/include/stdio.h
catman: /usr/include/sys/types.h
catman: /usr/include/sys/stat.h
catman: /usr/include/sys/time.h
catman: /usr/include/sys/dir.h
catman: /usr/include/ctype.h
chown: chown.c
chown: /usr/include/stdio.h
chown: /usr/include/ctype.h
chown: /usr/include/sys/types.h
chown: /usr/include/sys/stat.h
chown: /usr/include/pwd.h
clri: clri.c
clri: /usr/include/sys/param.h
clri: /usr/include/ufs/inode.h
clri: /usr/include/ufs/fs.h
comsat: comsat.c
comsat: /usr/include/sys/types.h
comsat: /usr/include/sys/socket.h
comsat: /usr/include/sys/stat.h
comsat: /usr/include/sys/wait.h
comsat: /usr/include/netinet/in.h
comsat: /usr/include/stdio.h
comsat: /usr/include/sgtty.h
comsat: /usr/include/utmp.h
comsat: /usr/include/signal.h
comsat: /usr/include/errno.h
comsat: /usr/include/netdb.h
cron: cron.c
cron: /usr/include/sys/types.h
cron: /usr/include/stdio.h
cron: /usr/include/ctype.h
cron: /usr/include/signal.h
cron: /usr/include/sys/time.h
cron: /usr/include/sys/stat.h
dcheck: dcheck.c
dcheck: /usr/include/sys/param.h
dcheck: /usr/include/ufs/inode.h
dcheck: /usr/include/ufs/fs.h
dcheck: /usr/include/sys/dir.h
dcheck: /usr/include/stdio.h
diskpart: diskpart.c
diskpart: /usr/include/sys/param.h
diskpart: /usr/include/stdio.h
diskpart: /usr/include/disktab.h
dmesg: dmesg.c
dmesg: /usr/include/stdio.h
dmesg: /usr/include/sys/param.h
dmesg: /usr/include/nlist.h
dmesg: /usr/include/signal.h
dmesg: /usr/include/sys/vm.h
dmesg: /usr/include/sys/msgbuf.h
dumpfs: dumpfs.c
dumpfs: /usr/include/sys/param.h
dumpfs: /usr/include/ufs/inode.h
dumpfs: /usr/include/ufs/fs.h
dumpfs: /usr/include/stdio.h
dumpfs: /usr/include/fstab.h
edquota: edquota.c
edquota: /usr/include/stdio.h
edquota: /usr/include/signal.h
edquota: /usr/include/errno.h
edquota: /usr/include/pwd.h
edquota: /usr/include/ctype.h
edquota: /usr/include/fstab.h
edquota: /usr/include/sys/param.h
edquota: /usr/include/sys/stat.h
edquota: /usr/include/sys/file.h
edquota: /usr/include/sys/quota.h
flcopy: flcopy.c
flcopy: /usr/include/sys/file.h
fsirand: fsirand.c
fsirand: /usr/include/stdio.h
fsirand: /usr/include/sys/param.h
fsirand: /usr/include/sys/time.h
fsirand: /usr/include/ufs/fs.h
fsirand: /usr/include/sys/vnode.h
fsirand: /usr/include/ufs/inode.h
gettable: gettable.c
gettable: /usr/include/sys/types.h
gettable: /usr/include/sys/socket.h
gettable: /usr/include/netinet/in.h
gettable: /usr/include/stdio.h
gettable: /usr/include/netdb.h
halt: halt.c
halt: /usr/include/stdio.h
halt: /usr/include/sys/reboot.h
halt: /usr/include/sys/types.h
halt: /usr/include/sys/time.h
halt: /usr/include/errno.h
halt: /usr/include/signal.h
halt: /usr/include/utmp.h
icheck: icheck.c
icheck: /usr/include/stdio.h
icheck: /usr/include/sys/param.h
icheck: /usr/include/ufs/inode.h
icheck: /usr/include/ufs/fs.h
ifconfig: ifconfig.c
ifconfig: /usr/include/sys/types.h
ifconfig: /usr/include/sys/socket.h
ifconfig: /usr/include/sys/ioctl.h
ifconfig: /usr/include/netinet/in.h
ifconfig: /usr/include/net/if.h
ifconfig: /usr/include/stdio.h
ifconfig: /usr/include/errno.h
ifconfig: /usr/include/ctype.h
ifconfig: /usr/include/netdb.h
in.rlogind: in.rlogind.c
in.rlogind: /usr/include/stdio.h
in.rlogind: /usr/include/sys/types.h
in.rlogind: /usr/include/sys/stat.h
in.rlogind: /usr/include/sys/socket.h
in.rlogind: /usr/include/sys/wait.h
in.rlogind: /usr/include/netinet/in.h
in.rlogind: /usr/include/errno.h
in.rlogind: /usr/include/pwd.h
in.rlogind: /usr/include/signal.h
in.rlogind: /usr/include/sgtty.h
in.rlogind: /usr/include/stdio.h
in.rlogind: /usr/include/netdb.h
in.rlogind: /usr/include/utmp.h
inetd: inetd.c
inetd: /usr/include/rpc/rpc.h
inetd: /usr/include/rpc/pmap_prot.h
inetd: /usr/include/sys/ioctl.h
inetd: /usr/include/sys/socket.h
inetd: /usr/include/sys/file.h
inetd: /usr/include/sys/wait.h
inetd: /usr/include/arpa/inet.h
inetd: /usr/include/errno.h
inetd: /usr/include/stdio.h
inetd: /usr/include/signal.h
inetd: /usr/include/netdb.h
init: init.c
init: /usr/include/signal.h
init: /usr/include/sys/types.h
init: /usr/include/utmp.h
init: /usr/include/setjmp.h
init: /usr/include/sys/reboot.h
init: /usr/include/errno.h
init: /usr/include/sys/file.h
init: /usr/include/sys/ioctl.h
kgmon: kgmon.c
kgmon: /usr/include/sys/param.h
kgmon: /usr/include/machine/pte.h
kgmon: /usr/include/sys/vm.h
kgmon: /usr/include/stdio.h
kgmon: /usr/include/nlist.h
kgmon: /usr/include/ctype.h
kgmon: /usr/include/sys/gprof.h
mkfs: mkfs.c
mkfs: /usr/include/stdio.h
mkfs: /usr/include/a.out.h
mkfs: /usr/include/sys/param.h
mkfs: /usr/include/sys/time.h
mkfs: /usr/include/sys/vnode.h
mkfs: /usr/include/ufs/fsdir.h
mkfs: /usr/include/ufs/inode.h
mkfs: /usr/include/ufs/fs.h
mknod: mknod.c
mkproto: mkproto.c
mkproto: /usr/include/stdio.h
mkproto: /usr/include/sys/param.h
mkproto: /usr/include/ufs/inode.h
mkproto: /usr/include/ufs/fs.h
mkproto: /usr/include/sys/dir.h
mount: mount.c
mount: /usr/include/sys/param.h
mount: /usr/include/rpc/rpc.h
mount: /usr/include/sys/errno.h
mount: /usr/include/sys/time.h
mount: /usr/include/nfs/nfs.h
mount: /usr/include/rpcsvc/mount.h
mount: /usr/include/sys/socket.h
mount: /usr/include/sys/stat.h
mount: /usr/include/netdb.h
mount: /usr/include/stdio.h
mount: /usr/include/mntent.h
ncheck: ncheck.c
ncheck: /usr/include/sys/param.h
ncheck: /usr/include/ufs/inode.h
ncheck: /usr/include/ufs/fs.h
ncheck: /usr/include/sys/dir.h
ncheck: /usr/include/stdio.h
newfs: newfs.c
newfs: /usr/include/sys/param.h
newfs: /usr/include/sys/stat.h
newfs: /usr/include/ufs/fs.h
newfs: /usr/include/sys/dir.h
newfs: /usr/include/stdio.h
newfs: /usr/include/disktab.h
newfs: /usr/include/sys/ioctl.h
nfsd: nfsd.c
nfsd: /usr/include/sys/param.h
nfsd: /usr/include/rpc/rpc.h
nfsd: /usr/include/sys/socket.h
nfsd: /usr/include/sys/errno.h
nfsd: /usr/include/sys/time.h
nfsd: /usr/include/nfs/nfs.h
nfsd: /usr/include/stdio.h
nfsd: /usr/include/signal.h
portmap: portmap.c
portmap: /usr/include/rpc/rpc.h
portmap: /usr/include/rpc/pmap_prot.h
portmap: /usr/include/stdio.h
portmap: /usr/include/netdb.h
portmap: /usr/include/sys/socket.h
portmap: /usr/include/sys/time.h
portmap: /usr/include/sys/ioctl.h
pstat: pstat.c
pstat: /usr/include/sys/param.h
pstat: /usr/include/sys/dir.h
pstat: /usr/include/sys/file.h
pstat: /usr/include/sys/user.h
pstat: /usr/include/sys/proc.h
pstat: /usr/include/sys/text.h
pstat: /usr/include/sys/vnode.h
pstat: /usr/include/ufs/inode.h
pstat: /usr/include/sys/map.h
pstat: /usr/include/sys/ioctl.h
pstat: /usr/include/sys/tty.h
pstat: /usr/include/sys/conf.h
pstat: /usr/include/sys/vm.h
pstat: /usr/include/nlist.h
pstat: /usr/include/machine/pte.h
quot: quot.c
quot: /usr/include/stdio.h
quot: /usr/include/ctype.h
quot: /usr/include/sys/param.h
quot: /usr/include/ufs/inode.h
quot: /usr/include/ufs/fs.h
quot: /usr/include/sys/file.h
quot: /usr/include/fstab.h
quot: /usr/include/pwd.h
quot: /usr/include/utmp.h
quotaon: quotaon.c
quotaon: /usr/include/sys/param.h
quotaon: /usr/include/sys/file.h
quotaon: /usr/include/stdio.h
quotaon: /usr/include/fstab.h
quotaon: /usr/include/mtab.h
quotacheck: quotacheck.c
quotacheck: /usr/include/stdio.h
quotacheck: /usr/include/ctype.h
quotacheck: /usr/include/signal.h
quotacheck: /usr/include/sys/param.h
quotacheck: /usr/include/ufs/inode.h
quotacheck: /usr/include/ufs/fs.h
quotacheck: /usr/include/sys/quota.h
quotacheck: /usr/include/sys/stat.h
quotacheck: /usr/include/fstab.h
quotacheck: /usr/include/pwd.h
reboot: reboot.c
reboot: /usr/include/stdio.h
reboot: /usr/include/sys/reboot.h
reboot: /usr/include/errno.h
reboot: /usr/include/signal.h
reboot: /usr/include/sys/types.h
reboot: /usr/include/sys/time.h
reboot: /usr/include/utmp.h
renice: renice.c
renice: /usr/include/sys/time.h
renice: /usr/include/sys/resource.h
renice: /usr/include/stdio.h
renice: /usr/include/pwd.h
repquota: repquota.c
repquota: /usr/include/stdio.h
repquota: /usr/include/sys/param.h
repquota: /usr/include/sys/quota.h
repquota: /usr/include/sys/stat.h
repquota: /usr/include/fstab.h
repquota: /usr/include/pwd.h
rexecd: rexecd.c
rexecd: /usr/include/sys/ioctl.h
rexecd: /usr/include/sys/param.h
rexecd: /usr/include/sys/socket.h
rexecd: /usr/include/sys/wait.h
rexecd: /usr/include/netinet/in.h
rexecd: /usr/include/stdio.h
rexecd: /usr/include/errno.h
rexecd: /usr/include/pwd.h
rexecd: /usr/include/signal.h
rexecd: /usr/include/netdb.h
rmt: rmt.c
rmt: /usr/include/stdio.h
rmt: /usr/include/sgtty.h
rmt: /usr/include/sys/types.h
rmt: /usr/include/sys/mtio.h
rmt: /usr/include/errno.h
route: route.c
route: /usr/include/sys/types.h
route: /usr/include/sys/socket.h
route: /usr/include/sys/ioctl.h
route: /usr/include/sys/mbuf.h
route: /usr/include/net/route.h
route: /usr/include/netinet/in.h
route: /usr/include/stdio.h
route: /usr/include/errno.h
route: /usr/include/ctype.h
route: /usr/include/netdb.h
route: /usr/include/nlist.h
rshd: rshd.c
rshd: /usr/include/sys/ioctl.h
rshd: /usr/include/sys/param.h
rshd: /usr/include/sys/socket.h
rshd: /usr/include/sys/wait.h
rshd: /usr/include/netinet/in.h
rshd: /usr/include/stdio.h
rshd: /usr/include/errno.h
rshd: /usr/include/pwd.h
rshd: /usr/include/signal.h
rshd: /usr/include/netdb.h
rwall: rwall.c
rwall: /usr/include/stdio.h
rwall: /usr/include/utmp.h
rwall: /usr/include/rpc/rpc.h
rwall: /usr/include/rpcsvc/netwall.h
rxformat: rxformat.c
rxformat: /usr/include/stdio.h
rxformat: /usr/include/sys/file.h
rxformat: /usr/include/errno.h
rxformat: /usr/include/vaxuba/rxreg.h
sa: sa.c
sa: /usr/include/stdio.h
sa: /usr/include/sys/types.h
sa: /usr/include/sys/acct.h
sa: /usr/include/signal.h
sa: /usr/include/utmp.h
sa: /usr/include/pwd.h
savecore: savecore.c
savecore: /usr/include/stdio.h
savecore: /usr/include/nlist.h
savecore: /usr/include/sys/param.h
savecore: /usr/include/sys/dir.h
savecore: /usr/include/sys/stat.h
savecore: /usr/include/ufs/fs.h
savecore: /usr/include/sys/time.h
showmount: showmount.c
showmount: /usr/include/stdio.h
showmount: /usr/include/rpc/rpc.h
showmount: /usr/include/sys/socket.h
showmount: /usr/include/netdb.h
showmount: /usr/include/sys/time.h
showmount: /usr/include/sys/errno.h
showmount: /usr/include/nfs/nfs.h
showmount: /usr/include/rpcsvc/mount.h
shutdown: shutdown.c
shutdown: /usr/include/stdio.h
shutdown: /usr/include/ctype.h
shutdown: /usr/include/signal.h
shutdown: /usr/include/setjmp.h
shutdown: /usr/include/utmp.h
shutdown: /usr/include/sys/time.h
shutdown: /usr/include/sys/resource.h
shutdown: /usr/include/sys/types.h
shutdown: /usr/include/sys/errno.h
shutdown: /usr/include/rpc/rpc.h
shutdown: /usr/include/nfs/nfs.h
shutdown: /usr/include/rpcsvc/mount.h
shutdown: /usr/include/rpcsvc/netwall.h
shutdown: /usr/include/sys/socket.h
shutdown: /usr/include/netdb.h
swapon: swapon.c
swapon: /usr/include/stdio.h
swapon: /usr/include/fstab.h
telnetd: telnetd.c
telnetd: /usr/include/sys/types.h
telnetd: /usr/include/sys/socket.h
telnetd: /usr/include/sys/wait.h
telnetd: /usr/include/netinet/in.h
telnetd: /usr/include/arpa/telnet.h
telnetd: /usr/include/stdio.h
telnetd: /usr/include/signal.h
telnetd: /usr/include/errno.h
telnetd: /usr/include/sgtty.h
telnetd: /usr/include/netdb.h
telnetd: /usr/include/utmp.h
tftpd: tftpd.c
tftpd: /usr/include/sys/types.h
tftpd: /usr/include/sys/socket.h
tftpd: /usr/include/sys/ioctl.h
tftpd: /usr/include/sys/wait.h
tftpd: /usr/include/sys/stat.h
tftpd: /usr/include/netinet/in.h
tftpd: /usr/include/arpa/tftp.h
tftpd: /usr/include/signal.h
tftpd: /usr/include/stdio.h
tftpd: /usr/include/errno.h
tftpd: /usr/include/ctype.h
tftpd: /usr/include/netdb.h
tftpd: /usr/include/setjmp.h
trpt: trpt.c
trpt: /usr/include/sys/param.h
trpt: /usr/include/sys/socket.h
trpt: /usr/include/sys/socketvar.h
trpt: /usr/include/sys/protosw.h
trpt: /usr/include/net/route.h
trpt: /usr/include/net/if.h
trpt: /usr/include/netinet/in.h
trpt: /usr/include/netinet/in_pcb.h
trpt: /usr/include/netinet/in_systm.h
trpt: /usr/include/netinet/ip.h
trpt: /usr/include/netinet/ip_var.h
trpt: /usr/include/netinet/tcp.h
trpt: /usr/include/netinet/tcp_fsm.h
trpt: /usr/include/netinet/tcp_seq.h
trpt: /usr/include/netinet/tcp_timer.h
trpt: /usr/include/netinet/tcp_var.h
trpt: /usr/include/netinet/tcpip.h
trpt: /usr/include/netinet/tcp_debug.h
trpt: /usr/include/stdio.h
trpt: /usr/include/errno.h
trpt: /usr/include/nlist.h
tunefs: tunefs.c
tunefs: /usr/include/sys/param.h
tunefs: /usr/include/sys/stat.h
tunefs: /usr/include/ufs/fs.h
tunefs: /usr/include/sys/time.h
tunefs: /usr/include/sys/vnode.h
tunefs: /usr/include/ufs/inode.h
tunefs: /usr/include/stdio.h
tunefs: /usr/include/fstab.h
umount: umount.c
umount: /usr/include/sys/param.h
umount: /usr/include/sys/file.h
umount: /usr/include/stdio.h
umount: /usr/include/mntent.h
umount: /usr/include/errno.h
umount: /usr/include/sys/time.h
umount: /usr/include/rpc/rpc.h
umount: /usr/include/nfs/nfs.h
umount: /usr/include/rpcsvc/mount.h
umount: /usr/include/sys/socket.h
umount: /usr/include/netdb.h
update: update.c
update: /usr/include/signal.h
vipw: vipw.c
vipw: /usr/include/sys/types.h
vipw: /usr/include/sys/stat.h
vipw: /usr/include/sys/file.h
vipw: /usr/include/stdio.h
vipw: /usr/include/errno.h
vipw: /usr/include/signal.h
ypbind: ypbind.c
ypbind: /usr/include/dbm.h
ypbind: /usr/include/stdio.h
ypbind: /usr/include/errno.h
ypbind: /usr/include/signal.h
ypbind: /usr/include/sys/time.h
ypbind: /usr/include/sys/wait.h
ypbind: /usr/include/rpc/rpc.h
ypbind: /usr/include/sys/dir.h
ypbind: /usr/include/netdb.h
ypbind: /usr/include/sys/socket.h
ypbind: /usr/include/sys/ioctl.h
ypbind: /usr/include/rpc/pmap_clnt.h
ypbind: /usr/include/rpcsvc/yp_prot.h
ypbind: /usr/include/rpcsvc/ypclnt.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
