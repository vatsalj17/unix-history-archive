: FILE BACKUP SHELL PROCEDURE:
: This procedure is merely a sample.  You should use it as a prototype
: for your own current situation.
: The command `vc88' should be changed to `vc50' if you have DEC RP03 disk.
onintr INTR

: restart
echo "Is this a restart of the filesave(`y' or `n')?  "
= g </dev/tty
if $g != y goto start
echo "Enter filesystem backup you wish to restart at(e.g. `f2' or `root')."
= l </dev/tty
: start
= f start
echo "System X filesave:"
echo "If anything goes wrong type BREAK and get help!"
echo "Enter number of spare drive:"
= x </dev/tty
echo SPARE, $x >/etc/log/filesave
echo "Enter name of pack on drive 0 (e.g.: ``p04??''):"
= a </dev/tty
echo DRIVE 0 PACK, $a >>/etc/log/filesave
echo "Enter name of pack on drive 1:"
= b </dev/tty
echo DRIVE 1 PACK, $b >>/etc/log/filesave
echo "Enter name of pack on drive 3:"
= e </dev/tty
echo DRIVE 3 PACK, $e >>/etc/log/filesave
= f $l root
goto $f
: root
: f3
echo "Mount pack for `root', and `f3' on drive $x --"
echo "When ready, enter name of pack:"
= c </dev/tty
vc88 root /dev/rrp0 $a /dev/rrp$x0 $c </dev/tty
if $r -ne 0 goto failed
vc88 f3 /dev/rrp4 $a /dev/rrp$x4 $c </dev/tty
if $r -ne 0 goto failed
echo "\n\nREMOVE PACK $c ON DRIVE $x\n\n"

: f1
: f2
: usr
= f  f1
echo "Mount pack for `f1', `f2' and `usr' on drive $x --"
echo "When ready, enter name of pack:"
= d </dev/tty
vc88 f1 /dev/rrp14 $b /dev/rrp$x4 $d  </dev/tty
if $r -ne 0 goto failed
vc88 f2 /dev/rrp15 $b /dev/rrp$x5 $d </dev/tty
if $r -ne 0 goto failed
vc88 usr /dev/rrp16 $b /dev/rrp$x6 $d </dev/tty
if $r -ne 0 goto failed
echo "\n\nREMOVE $d FROM DRIVE $x\n\n"

: f4
= f f4
echo "Mount pack for `f4' on drive $x --"
echo "When ready, enter name of pack:"
= h </dev/tty
vc88 f4 /dev/rrp35 $e /dev/rrp$x5 $h </dev/tty
if $r -ne 0 goto failed
echo "\n\nREMOVE $h from DRIVE $x\n\n"
echo "Put the pack copies ("$c, $d and $h") on the shelf."
: ficheck
= f ficheck
check
echo "If `check' is clean enter initials:     \c"
= i </dev/tty
echo Initials: $i >>/etc/log/filesave
echo "Enter counts from status phone 'e.g. xxx up xxx down'\c"
= h </dev/tty
echo up-down counts, $h >>/etc/log/filesave
echo "When filesave is completed , set the date to the correct time"
echo "then set CONSOLE ADDRESS SWITCHES to `173020' and then enter `CNTRL/d'"
echo "System will then reboot itself MULTI-USER"
cat /etc/log/filesave >>/etc/log/filesavelog
rm /etc/log/filesave
exit

: failed
echo "The last backup attempted failed."
echo "Answer `y' to the next question to return to the same pack copy."
echo "If you wish to return to a different spot answer `restart'."

: INTR
onintr INTR
echo "\nWant to go on ? (`y' or `n'):"
= g </dev/tty
if x$g = xy goto $f
if $g = restart goto restart
exit 1
