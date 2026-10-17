C = /lib/libc.a
A = /lib/liba.a
S = /lib/libS.a
T = /lib/libt.a
Y = /lib/liby.a
I = /usr/include/

all:		absout ac adjust alarm runat ar arcv ask ass banner bas bc \
		cal cat cc cdb chk chmod chown class clri cmp col comm copy \
		coreit cp cr create cron crypt date db dc dcopy dd detab df \
		diff disconnect dsw dtp dtpress du dump echo ed em e edrecon \
		entab errlog evict exec expr fd2 file find game gcat getit \
		getty grep icat init initp initpw initsh ipatch lcopy ld \
		lked ln log receive send logg look lpd lpq lpr print ls mail \
		man maze mc mesg message mkacc mkdir mkfs mknod mount mtabin \
		mv ncheck nice niced nm nohup od oscar pack passwd pcat pg \
		pp pr printc prof ps ptx pv pwcreat pwd pwedit pwscan pwzero \
		qf qf12 qf10 quiz restacc restart repeat prstop restor rm \
		rmdir rmtree rootlog scc sed set sh shutdown size sleep slp \
		smdate sort speed split strip stty su sum syms sync tap \
		taprd tee tektest time timelim touch tp tr trbas tty type typo \
		umount uniq units unlink unload unpack update val wall warn \
		watch wc who write xargs

absout:		/bin/absout

/bin/absout:	$C absout.c
		cc -O -s absout.c
		instal a.out bin 0701 /bin/absout

ac:		/etc/ac

/etc/ac:	$A $C $Iwtmp.h $Ilocal-system $Ipasswd.h \
		ac.h ac1.c ac2.c ac3.c ac4.c
		cc -s ac1.c ac2.c ac3.c ac4.c
		instal a.out bin 0700 /etc/ac
		rm -f ac1.o ac2.o ac3.o ac4.o

adjust:		/etc/adjust

/etc/adjust:	$A $S $C adjust.c $Iwtmp.h $Ilocal-system \
		$Ipasswd.h $Ilnode.h $Istdio.h $Iclassincludes.h
		cc -O -s adjust.c -lS
		instal a.out root 0700 /etc/adjust

alarm:		/bin/runat

runat:		/bin/runat

/bin/alarm:	/bin/runat

/bin/runat:	$A $S $C $Istdio.h $Idate.h alarm.c
		cc -O -s alarm.c -lS
		instal a.out bin 0701 /bin/alarm /bin/runat

ar:		/bin/ar

/bin/ar:	$C ar.c
		cc -O -s ar.c
		instal a.out bin 0701 /bin/ar

arcv:		/etc/arcv

/etc/arcv:	$C arcv.c
		cc -O -s arcv.c
		instal a.out bin 0701 /etc/arcv

ask:		/bin/ask

/bin/ask:	$C ask.c
		cc -O -s ask.c
		instal a.out bin 0701 /bin/ask

ass:		/bin/ass

/bin/ass:	$C ass.c
		cc -O -s ass.c
		instal a.out bin 0701 /bin/ass

banner:		/bin/banner

/bin/banner:	$C banner.c
		cc -O -s banner.c
		instal a.out bin 0701 /bin/banner

bas:		/bin/bas

/bin/bas:	bas.s
		-as bas.s
		ld a.out -s -lc -la -n
		instal - a.out bin 0701 /bin/bas

bc:		/bin/bc

/bin/bc:	$C bc.y
		yacc bc.y
		cc y.tab.c -ly
		rm y.tab.c
		instal a.out bin 0701 /bin/bc

cal:		/bin/cal

/bin/cal:	$C cal.c
		cc -O -s cal.c
		instal a.out bin 0701 /bin/cal

cat:		/bin/cat

/bin/cat:	cat.s
		-as cat.s
		ld a.out -s -lc -la
		instal a.out bin 0701 /bin/cat

cc:		/bin/cc

/bin/cc:	$C cc.c
		cc -O -s -n cc.c
		instal - a.out bin 1701 /bin/cc

cdb:		/bin/cdb

/bin/cdb:	$A $C $Iparam.h $Iuser.h cdb.h cdb1.c cdb2.c cdb3.c
		cc -O -s cdb1.c cdb2.c cdb3.c
		instal a.out bin 0701 /bin/cdb
		rm -f cdb1.o cdb2.o cdb3.o

chk:		/etc/chk

/etc/chk:	$A $C $Ilocal-system $Iino.h $Ifilsys.h chk.c
		cc -O -s chk.c
		instal a.out bin 0700 /etc/chk

chmod:		/bin/chmod

/bin/chmod:	$C chmod.c
		cc -O -s chmod.c
		instal a.out bin 0701 /bin/chmod

chown:		/etc/chown

/etc/chown:	$C $Ilocal-system $Ipasswd.h chown.c
		cc -O -s chown.c
		instal a.out bin 0700 /etc/chown

class:		/etc/class

/etc/class:	$C class.c $Ilocal-system $Ipasswd.h $Iclass.h
		cc -O -s class.c
		instal a.out root 0700 /etc/class

clri:		/etc/clri

/etc/clri:	clri.s
		-as clri.s
		ld a.out -s -lc -la
		instal a.out root 0700 /etc/clri

cmp:		/bin/cmp

/bin/cmp:	$C cmp.c
		cc -O -s cmp.c
		instal a.out bin 0701 /bin/cmp

col:		/bin/col

/bin/col:	$C col.c
		cc -O -s col.c
		instal a.out bin 0701 /bin/col

comm:		/bin/comm

/bin/comm:	$C comm.c
		cc -O -s comm.c
		instal a.out bin 0701 /bin/comm

copy:		/bin/copy

/bin/copy:	$C $Ilocal-system copy.c
		cc -O -s copy.c
		instal a.out bin 0701 /bin/copy

coreit:		/bin/coreit

/bin/coreit:	$C $Iparam.h $Ifile.h $Iuser.h $Ireg.h coreit.c
		cc -O -s coreit.c
		instal a.out bin 0701 /bin/coreit

cp:		/bin/cp

/bin/cp:	$C cp.c
		cc -O -s cp.c
		instal a.out bin 0701 /bin/cp

cr:		/bin/cr

/bin/cr:	cr.s
		-as cr.s
		ld a.out -s -lc -la
		instal a.out bin 0701 /bin/cr

create:		/bin/create

/bin/create:	$C create.c
		cc -O -s create.c
		instal a.out bin 0701 /bin/create

cron:		/etc/cron

/etc/cron:	$C cron.c
		cc -O -s cron.c
		instal a.out root 0700 /etc/cron

crypt:		/bin/crypt

/bin/crypt:	$A $C crypt.c
		cc -O -s crypt.c
		instal a.out bin 0701 /bin/crypt

date:		/bin/date

/bin/date:	$C $Ilocal-system $Iwtmp.h date.c
		cc -O -s date.c
		instal a.out bin 0701 /bin/date

db:		/bin/db

/bin/db:	db1.s db2.s db3.s db4.s
		-as db1.s db2.s db3.s db4.s
		ld a.out -s -lc -la
		instal a.out bin 0701 /bin/db

dc:		/bin/dc

/bin/dc:	dc1.s dc2.s dc3.s dc4.s dc5.s
		-as dc1.s dc2.s dc3.s dc4.s dc5.s
		ld a.out -s -lc -la
		instal a.out bin 0701 /bin/dc

dcopy:		/etc/dcopy

/etc/dcopy:	$A $C $Ilocal-system $Iparam.h $Ifilsys.h $Iino.h dcopy.c
		cc -O -s dcopy.c
		instal a.out bin 0700 /etc/dcopy

dd:		/bin/dd

/bin/dd:	$C dd.c
		cc -O -s -i dd.c
		instal - a.out bin 0701 /bin/dd

detab:		/bin/detab

/bin/detab:	$C detab.c
		cc -O -s detab.c
		instal a.out bin 0701 /bin/detab

df:		/bin/df

/bin/df:	$C df.c
		cc -O -s df.c
		instal a.out root 4701 /bin/df

diff:		/bin/diff

/bin/diff:	$C diff.c
		cc -O -s -i diff.c
		instal - a.out bin 0701 /bin/diff

disconnect:	/bin/disconnect

/bin/disconnect:$C disconnect.c
		cc -O -s disconnect.c
		instal a.out root 4701 /bin/disconnect

dsw:		/bin/dsw

/bin/dsw:	dsw.s
		-as dsw.s
		ld a.out -s -lc -la
		instal a.out bin 0701 /bin/dsw

dtp:		/etc/dtp

/etc/dtp:	dtp.s
		-as dtp.s
		ld a.out -s -lc -la
		instal a.out root 0700 /etc/dtp

dtpress:	/etc/dtpress

/etc/dtpress:	dtpress.s
		-as dtpress.s
		ld a.out -s -lc -la
		instal a.out bin 0701 /etc/dtpress

du:		/bin/du

/bin/du:	$C $Ilocal-system du.c
		cc -O -s du.c
		instal a.out bin 0701 /bin/du

dump:		/etc/dump

/etc/dump:	$A $C $Iino.h $Ifilsys.h dump.c
		cc -O -s dump.c
		instal a.out root 0700 /etc/dump

echo:		/bin/echo

/bin/echo:	$A $C echo.c
		cc -O -s echo.c
		instal a.out bin 0701 /bin/echo

ed:		/bin/e

em:		/bin/e

e:		/bin/e

/bin/em:	/bin/ed

/bin/ed:	/bin/e

/bin/e:		$C em.h em1.c em2.c
		cc -O -s -i em1.c em2.c
		instal - a.out bin 1701 /bin/em /bin/ed /bin/e
		rm -f em1.o em2.o

edrecon:	/bin/edrecon

/bin/edrecon:	edrecon.s
		-as edrecon.s
		ld a.out -s -lc -la
		instal a.out bin 0701 /bin/edrecon

entab:		/bin/entab

/bin/entab:	$C entab.c
		cc -O -s entab.c
		instal a.out bin 0701 /bin/entab

errlog:		/etc/errlog

/etc/errlog:	$A $C $Ilocal-system $Istat.h $Isprintf.h errlog.c
		cc -O -s errlog.c
		instal a.out root 0700 /etc/errlog

evict:		/etc/evict

/etc/evict:	$C $Iparam.h $Iproc.h $Ilocal-system \
		$Iclass.h $Itty.h $Ipasswd.h $Ilnode.h $Istat.h \
		$Iutmp.h $Iuid.h evict.c
		cc -O -s evict.c
		instal a.out root 0700 /etc/evict

exec:		/bin/exec

/bin/exec:	$C exec.c
		cc -O -s exec.c
		instal a.out bin 0701 /bin/exec

expr:		/bin/expr

/bin/expr:	$S $Y $C expr.y
		yacc expr.y
		cc y.tab.c -ly
		rm y.tab.c
		instal a.out bin 0701 /bin/expr

fd2:		/bin/fd2

/bin/fd2:	$C fd2.c
		cc -O -s fd2.c
		instal a.out bin 0701 /bin/fd2

file:		/bin/file

/bin/file:	$C file.c
		cc -O -s file.c
		instal a.out bin 0701 /bin/file

find:		/bin/find

/bin/find:	$C $Ilocal-system $Ipasswd.h find.c
		cc -O -s find.c
		instal a.out bin 0701 /bin/find

game:		/usr/games/game

/usr/games/game:$C game.c
		cc -O -s game.c
		instal a.out bin 0701 /usr/games/game

gcat:		/bin/gcat

/bin/gcat:	$A $C gcat.c
		cc -O -s gcat.c
		instal a.out bin 0701 /bin/gcat

getit:		/etc/getit

/etc/getit:	$C getit.c
		cc -O -s getit.c
		instal a.out root 0700 /etc/getit

getty:		/etc/getty

/etc/getty:	$A $C $Ilocal-system $Ittygroup.h $Iparam.h \
		$Istat16.h $Ilnode.h $Iutmp.h $Iino.h $Ifilsys.h \
		$Ipasswd.h getty.c decays.c decays.h
		cc -O -s -n getty.c
		instal - a.out root 0700 /etc/getty

grep:		/bin/grep

/bin/grep:	$C grep.c
		cc -O -s grep.c
		instal a.out bin 0701 /bin/grep

icat:		/bin/icat

/bin/icat:	$C icat.c
		cc -O -s icat.c
		instal a.out bin 0701 /bin/icat

init:		/etc/init

/etc/init:	$C $Ilocal-system $Iwtmp.h $Idefines.h $Iparam.h \
		$Ipasswd.h $Ilnode.h $Istat16.h $Ipzomb.h init.c
		cc -O -s -n init.c
		instal - a.out root 0700 /etc/init

initp:		/etc/initp

/etc/initp:	$C initp.c $Ilocal-system $Ipasswd.h
		cc -O -s initp.c
		instal a.out root 0700 /etc/initp

initpw:		/etc/initpw

/etc/initpw:	$A $S $C initpw.c
		cc -O -s initpw.c -lS
		instal a.out root 4701 /etc/initpw

initsh:		/etc/initsh

/etc/initsh:	$A $S $C initsh.c
		cc -O -s initsh.c -lS
		instal a.out root 4701 /etc/initsh

ipatch:		/etc/ipatch

/etc/ipatch:	$C ipatch.c
		cc -O -s ipatch.c
		instal a.out root 0700 /etc/ipatch

lcopy:		/etc/lcopy

/etc/lcopy:	$C lcopy.c
		cc -O -s lcopy.c
		instal a.out root 0700 /etc/lcopy

ld:		/bin/ld

/bin/ld:	$C ld.c
		cc -O -s -n ld.c
		instal - a.out bin 1701 /bin/ld

lked:		/bin/lked

/bin/lked:	$C lked.c
		cc -O -s lked.c
		instal a.out bin 0701 /bin/lked

ln:		/bin/ln

/bin/ln:	$C $Ierrnos.h ln.c
		cc -O -s ln.c
		instal a.out bin 0701 /bin/ln

log:		/bin/log

receive:	/bin/log

send:		/bin/log

/bin/send:	/bin/log

/bin/receive:	/bin/log

/bin/log:	$A $C $Ilocal-system $Ierrnos.h $Istat16.h \
		$Istat.h $Isprintf.h log.c
		cc -O -s log.c
		instal a.out root 4701 /bin/log /bin/send /bin/receive

logg:		/etc/logg

/etc/logg:	$C $Ilocal-system $Ipasswd.h logg.c
		cc -O -s logg.c
		instal a.out root 0701 /etc/logg

look:		/etc/look

/etc/look:	$C $Ilocal-system $Istat16.h $Iproc.h $Iuser.h \
		$Itext.h $Ifile.h $Iinode.h $Itty.h $Iparam.h look.c
		cc -O -s look.c
		instal a.out root 0700 /etc/look

lpd:		/etc/lpd

/etc/lpd:	$C $Ilocal-system $Iprinters.h lpd.c
		cc -O -s -n lpd.c
		instal - a.out root 0700 /etc/lpd

lpq:		/bin/lpq

/bin/lpq:	$C lpq.c $Ilocal-system $Istat16.h $Ipasswd.h
		cc -O -s lpq.c
		instal a.out root 4701 /bin/lpq

lpr:		/bin/lpr

print:		/bin/lpr

/bin/print:	/bin/lpr

/bin/lpr:	$A $C $Ilocal-system $Istat.16.h $Ipasswd.h $Iprinters.h lpr.c
		cc -O -s -n lpr.c
		instal - a.out root 5701 /bin/print /bin/lpr

ls:		/bin/ls

/bin/ls:	$A $C $Ilocal-system $Ipasswd.h ls.c
		cc -O -s ls.c
		instal a.out bin 0701 /bin/ls

mail:		/bin/mail

/bin/mail:	$A $C $Ilocal-system $Ipasswd.h $Iuid.h mail.c
		cc -O -s mail.c
		instal a.out root 4701 /bin/mail

man:		/bin/man

/bin/man:	$C man.c
		cc -O -s man.c
		instal a.out bin 0701 /bin/man

maze:		/usr/games/maze

/usr/games/maze:$A $C maze.c
		cc -O -s maze.c
		instal a.out games 0701 /usr/games/maze

mc:		/bin/mc

/bin/mc:	$C mc.c
		cc -O -s mc.c
		instal a.out bin 0701 /bin/mc

mesg:		/bin/mesg

/bin/mesg:	$A $C mesg.c
		cc -O -s mesg.c
		instal a.out bin 0701 /bin/mesg

message:	/bin/message

/bin/message:	$C message.c
		cc -O -s message.c
		instal a.out bin 0701 /bin/message

mkacc:		/etc/mkacc

/etc/mkacc:	$A $S $C mkacc.c $Ilocal-system $Ipasswd.h \
		$Istdio.h $Iclass.h $Iclassincludes.h
		cc -O -s mkacc.c -lS
		instal a.out root 0700 /etc/mkacc

mkdir:		/bin/mkdir

/bin/mkdir:	mkdir.s
		-as mkdir.s
		ld a.out -s -lc -la
		instal a.out root 4701 /bin/mkdir

mkfs:		/etc/mkfs

/etc/mkfs:	$A $C mkfs.c
		cc -O -s mkfs.c
		instal a.out root 0700 /etc/mkfs

mknod:		/etc/mknod

/etc/mknod:	$C mknod.c
		cc -O -s mknod.c
		instal a.out root 0700 /etc/mknod

mount:		/etc/mount

/etc/mount:	$C $Imtab.h mount.c mtab.h
		cc -O -s mount.c
		instal a.out root 0700 /etc/mount

mtabin:		/etc/mtabin

/etc/mtabin:	$C $Imtab.h $Istat.h mtabin.c
		cc -O -s mtabin.c
		instal a.out root 0700 /etc/mtabin

mv:		/bin/mv

/bin/mv:	$C $Ilocal-system mv.c
		cc -O -s mv.c
		instal a.out root 4701 /bin/mv

ncheck:		/etc/ncheck

/etc/ncheck:	$C $Iino.h $Ifilsys.h ncheck.c
		cc -O -s ncheck.c
		instal a.out root 0700 /etc/ncheck

nice:		/bin/nice

/bin/nice:	$C nice.c
		cc -O -s nice.c
		instal a.out bin 0701 /bin/nice

niced:		/etc/niced

/etc/niced:	$C $Iparam.h $Iproc.h niced.c
		cc -O -s niced.c
		instal a.out root 0700 /etc/niced

nm:		/bin/nm

/bin/nm:	$A $C nm.c
		cc -O -s nm.c
		instal a.out bin 0701 /bin/nm

nohup:		/bin/nohup

/bin/nohup:	$C nohup.c
		cc -O -s nohup.c
		instal a.out bin 0701 /bin/nohup

od:		/bin/od

/bin/od:	$A $C od.c
		cc -O -s od.c
		instal a.out bin 0701 /bin/od

oscar:		/bin/oscar

/bin/oscar:	$C $Istat.h oscar.c
		cc -O -s oscar.c
		instal a.out root 4701 /bin/oscar

pack:		/bin/pack

/bin/pack:	$C $Ilocal-system pack.c
		cc -O -s pack.c
		instal a.out bin 0701 /bin/pack

passwd:		/bin/passwd

/bin/passwd:	$A $C $Ilocal-system $Ipasswd.h $Igtty.h passwd.c
		cc -O -s passwd.c
		instal a.out root 4701 /bin/passwd

pcat:		/bin/pcat

/bin/pcat:	$A $C pcat.c
		cc -O -s pcat.c
		instal a.out bin 0701 /bin/pcat

pg:		/bin/pg

/bin/pg:	$C pg.c
		cc -O -s pg.c
		instal a.out bin 0701 /bin/pg

pp:		/bin/pp

/bin/pp:	$C $Ilocal-system $Ipasswd.h $Ilnode.h $Iclass.h pp.c
		cc -O -s pp.c
		instal a.out bin 0701 /bin/pp

pr:		/bin/pr

/bin/pr:	$A $C $Ilocal-system pr.c
		cc -O -s pr.c
		instal a.out bin 0701 /bin/pr

printc:		/bin/printc

/bin/printc:	$C printc.c $Ilocal-system $Ipasswd.h $Iclass.h
		cc -O -s printc.c
		instal a.out bin 0701 /bin/printc

prof:		/bin/prof

/bin/prof:	$A $C prof.c
		cc -O -s prof.c
		instal a.out bin 0701 /bin/prof

ps:		/bin/ps

/bin/ps:	$A $C $Iparam.h $Iproc.h $Itty.h $Iuser.h ps.c
		cc -O -s ps.c
		instal a.out root 4701 /bin/ps

ptx:		/bin/ptx

/bin/ptx:	$C ptx.c
		cc -O -s ptx.c
		instal a.out bin 0701 /bin/ptx

pv:		/bin/pv

/bin/pv:	$C $Ilocal-system $Ipasswd.h $Iclass.h $Istat16.h pv.c
		cc -O -s pv.c
		instal a.out bin 0701 /bin/pv

pwcreat:	/etc/pwcreat

/etc/pwcreat:	$C $Ilocal-system $Ipasswd.h pwcreat.c
		cc -O -s pwcreat.c
		instal a.out root 0700 /etc/pwcreat

pwd:		/bin/pwd

/bin/pwd:	$C pwd.c
		cc -O -s pwd.c
		instal a.out root 4701 /bin/pwd

pwedit:		/bin/pwedit

/bin/pwedit:	$A $C $Ilocal-system $Ipasswd.h $Iclass.h pwedit.c
		cc -O -s pwedit.c
		instal a.out bin 0701 /bin/pwedit

pwscan:		/etc/pwscan

/etc/pwscan:	$C $Ilocal-system $Iparam.h $Ipasswd.h $Istat16.h pwscan.c
		cc -O -s pwscan.c
		instal a.out root 0700 /etc/pwscan

pwzero:		/etc/pwzero

/etc/pwzero:	$C $Ilocal-system $Iparam.h $Ipasswd.h pwzero.c
		cc -O -s pwzero.c
		instal a.out root 0700 /etc/pwzero

qf:		qf12 qf10

qf12:		/bin/qf12

qf10:		/bin/qf10

/bin/qf10:	/bin/qf12

/bin/qf12:	$A $S $C $Istdio.h qf.c
		cc -O -s qf.c -lS
		instal a.out bin 0701 /bin/qf12 /bin/qf10

quiz:		/usr/games/quiz

/usr/games/quiz:$C quiz.c
		cc -O -s quiz.c
		instal a.out games 0701 /usr/games/quiz

restacc:	/etc/restacc

/etc/restacc:	$A $S $C restacc.c $Ilocal-system $Ipasswd.h $Istdio.h
		cc -O -s restacc.c -lS
		instal a.out root 0700 /etc/restacc

restart:	/bin/restart

repeat:		/bin/repeat

prstop:		/bin/prstop

/bin/restart:	/bin/prstop

/bin/repeat:	/bin/prstop

/bin/prstop:	prstop.s
		-as prstop.s
		ld a.out -s -lc -la
		instal a.out root 4701 /bin/restart /bin/repeat /bin/prstop

restor:		/etc/restor

/etc/restor:	$A $C $Iino.h $Ifilsys.h restor.c
		cc -O -s restor.c
		instal a.out root 0700 /etc/restor

rm:		/bin/rm

/bin/rm:	$C $Ilocal-system rm.c
		cc -O -s rm.c
		instal a.out bin 0701 /bin/rm

rmdir:		/bin/rmdir

/bin/rmdir:	$C rmdir.c
		cc -O -s rmdir.c
		instal a.out root 4701 /bin/rmdir

rmtree:		/etc/rmtree

/etc/rmtree:	$C rmtree.c
		cc -O -s rmtree.c
		instal a.out root 0700 /etc/rmtree

rootlog:	/etc/rootlog

/etc/rootlog:	$A $C rootlog.c
		cc -O -s rootlog.c
		instal a.out root 0700 /etc/rootlog

scc:		/etc/scc

/etc/scc:	$C $Idefines.h $Iparam.h $Iproc.h scc.c
		cc -O -s scc.c
		instal a.out bin 0700 /etc/scc

sed:		/bin/sed

/bin/sed:	$C sed.h sed0.c sed1.c
		cc -O -s sed?.c
		instal a.out bin 0701 /bin/sed

set:		/bin/set

/bin/set:	$C set.c
		cc -O -s set.c
		instal a.out bin 0701 /bin/set

sh:		/bin/sh

/bin/sh:	$A $C $Ilocal-system $Ierrnos.h $Istat.h $Ipasswd.h sh.c
		cc -O -s -n sh.c
		instal - a.out bin 1701 /bin/sh /bin/-

shutdown:	/etc/shutdown

/etc/shutdown:	$C $Iparam.h $Iproc.h shutdown.c
		cc -O -s shutdown.c
		instal a.out root 0700 /etc/shutdown

size:		/bin/size

/bin/size:	$C size.c
		cc -O -s size.c
		instal a.out bin 0701 /bin/size

sleep:		/bin/sleep

/bin/sleep:	sleep.s
		-as sleep.s
		ld a.out -s -lc -la
		instal a.out bin 0701 /bin/sleep

slp:		/bin/slp

/bin/slp:	$C slp.c
		cc -O -s slp.c
		instal a.out bin 0701 /bin/slp

smdate:		/etc/smdate

/etc/smdate:	$C $Ilocal-system $Istat.h smdate.c
		cc -O -s smdate.c
		instal a.out root 0700 /etc/smdate

sort:		/bin/sort

/bin/sort:	$C sort.c
		cc -O -s -i sort.c
		instal - a.out bin 0701 /bin/sort

speed:		/bin/speed

/bin/speed:	speed.s
		-as speed.s
		ld a.out -s -lc -la
		instal a.out bin 0701 /bin/speed

split:		/bin/split

/bin/split:	$C split.c
		cc -O -s split.c
		instal a.out bin 0701 /bin/split

strip:		/bin/strip

/bin/strip:	$C strip.c
		cc -O -s strip.c
		instal a.out bin 0701 /bin/strip

stty:		/bin/stty

/bin/stty:	$C $Ilocal-system stty.c
		cc -O -s stty.c
		instal a.out bin 0701 /bin/STTY /bin/stty

su:		/etc/su

/etc/su:	$A $S $C $Iparam.h $Ilocal-system \
		$Ipasswd.h $Iwtmp.h $Istdio.h su.c
		cc -O -s su.c -lS
		instal a.out root 4701 /etc/su

sum:		/bin/sum

/bin/sum:	sum.s
		-as sum.s
		ld a.out -s -lc -la
		instal a.out bin 0701 /bin/sum

syms:		/bin/syms

/bin/syms:	$C syms.c
		cc -O -s syms.c
		instal a.out bin 0701 /bin/syms

sync:		/bin/sync

/bin/sync:	sync.s
		-as sync.s
		ld a.out -s -lc -la
		instal a.out bin 0701 /bin/sync

tap:		/bin/tap

/bin/tap:	$S $C $Ilocal-system $Iparam.h $Iuser.h $Iproc.h \
		$Ifile.h $Istat16.h $Iinode.h $Istdio.h tap.c
		cc -O -s tap.c -lS
		instal a.out root 4701 /bin/tap

taprd:		/etc/taprd

/etc/taprd:	$C $Ilocal-system $Istat16.h taprd.c
		cc -O -s taprd.c
		instal a.out root 0700 /etc/taprd

tee:		/bin/tee

/bin/tee:	$C $Istat.h tee.c
		cc -O -s tee.c
		instal a.out bin 0701 /bin/tee

tektest:	/bin/tektest

/bin/tektest:	$A $S $T $C $Icplot.h tektest.c
		cc -O -s tektest.c -lt -lS
		instal a.out bin 0701 /bin/tektest

time:		/bin/time

/bin/time:	$C $Iparam.h time.c
		cc -O -s time.c
		instal a.out bin 0701 /bin/time

timelim:	/bin/timelim

/bin/timelim:	$C timelim.c
		cc -O -s timelim.c
		instal a.out bin 0701 /bin/timelim

touch:		/bin/touch

/bin/touch:	$C $S touch.c
		cc -O -s touch.c -lS
		instal a.out bin 0701 /bin/touch

tp:		/etc/tp

/etc/tp:	tp1.s tp2.s tp3.s tp4.s
		-as tp1.s tp2.s tp3.s tp4.s
		ld a.out -s -lc -la
		instal a.out root 0700 /etc/tp

tr:		/bin/tr

/bin/tr:	$C tr.c
		cc -O -s tr.c
		instal a.out bin 0701 /bin/tr

trbas:		/bin/trbas

/bin/trbas:	$C trbas.c
		cc -O -s trbas.c
		instal a.out bin 0701 /bin/trbas

tty:		/bin/tty

/bin/tty:	tty.s
		-as tty.s
		ld a.out -s -lc -la
		instal a.out bin 0701 /bin/tty

type:		/bin/type

/bin/type:	$C $Igtty.h type.c
		cc -O -s type.c
		instal a.out bin 0701 /bin/type

typo:		/bin/typo

/bin/typo:	$A $C typo.c
		cc -O -s typo.c
		instal a.out bin 0701 /bin/typo

umount:		/etc/umount

/etc/umount:	$C $Imtab.h $Ierrnos.h $Istat.h umount.c mtab.h
		cc -O -s umount.c
		instal a.out root 0700 /etc/umount

uniq:		/bin/uniq

/bin/uniq:	$C uniq.c
		cc -O -s uniq.c
		instal a.out bin 0701 /bin/uniq

units:		/bin/units

/bin/units:	$A $C units.c
		cc -O -s units.c
		instal a.out bin 0701 /bin/units

unlink:		/etc/unlink

/etc/unlink:	$C unlink.c
		cc -O -s unlink.c
		instal a.out root 0700 /etc/unlink

unload:		/etc/unload

/etc/unload:	$C unload.c
		cc -O -s unload.c
		instal a.out root 0700 /etc/unload

unpack:		/bin/unpack

/bin/unpack:	$A $C $Ilocal-system unpack.c
		cc -O -s unpack.c
		instal a.out bin 0701 /bin/unpack

update:		/etc/update

/etc/update:	$C $Iparam.h $Ipasswd.h $Ilnode.h update.c
		cc -O -s update.c
		instal a.out root 0700 /etc/update

val:		/etc/val

/etc/val:	$C $Ilocal-system $Ipasswd.h $Ittygroup.h val.c
		cc -O -s val.c
		instal a.out root 0700 /etc/val

wall:		/etc/wall

/etc/wall:	$C $Ilocal-system $Iutmp.h wall.c
		cc -O -s wall.c
		instal a.out root 0700 /etc/wall

warn:		/etc/warn

/etc/warn:	$A $C $Iutmp.h $Istat16.h $Iparam.h \
		$Iproc.h $Itext.h $Iinode.h warn.c
		cc -O -s warn.c
		instal a.out root 0700 /etc/warn

watch:		/etc/watch

/etc/watch:	$C watch.c
		cc -O -s watch.c
		instal a.out root 0700 /etc/watch

wc:		/bin/wc

/bin/wc:	$C wc.c
		cc -O -s wc.c
		instal a.out bin 0701 /bin/wc

who:		/bin/who

/bin/who:	$A $C $Ilocal-system $Iutmp.h $Ipasswd.h \
		$Iclass.h $Igtty.h who.c
		cc -O -s who.c
		instal a.out bin 0701 /bin/who

write:		/bin/write

/bin/write:	write.s
		-as write.s
		ld a.out -s -lc -la
		instal a.out bin 0701 /bin/write

xargs:		/bin/xargs

/bin/xargs:	$C xargs.c
		cc -O -s xargs.c
		instal a.out bin 0701 /bin/xargs


