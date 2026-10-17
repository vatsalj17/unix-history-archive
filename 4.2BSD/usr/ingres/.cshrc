set	tty=`tty`
alias	a	alias
alias	to	echo "From kalash \($tty:t\): \!:2* > /dev/tty\!^"

set	history = 25 \
	mail = /usr/spool/mail/kalash \
	notify

alias wi whereis
alias hi history
alias so source
alias z suspend
alias	ta	vi -t
alias la uptime
alias cribbage /usr/games/cribbage
alias mille /usr/games/mille
alias m Mail
alias l ls -F
set ignoreeof p=/usr/public  g=/usr/games 
set ut=/usr/tmp t=/tmp s=/usr/spool m=/usr/spool/mail i=/usr/include src=/usr/src 
set cmd=/usr/src/cmd sys=/usr/sys 
alias Make make -f makefile -f makeSCCS
alias z suspend
alias netwho "net -n -l network -m\!* who"
alias pd pushd
alias pop popd
alias msgs msgs -l
set path=(~/bin /usr/ucb /usr/bin /bin /usr/local /usr/new /usr/hosts .)
setenv EXINIT "set ai magic sm sh=/bin/csh lisp slow"
setenv ROGUEOPTS "askme,jump,name=Lord of the Universe,fruit=Apple,terse,passgo"
setenv MAILER	/usr/ucb/Mail

set	ignoreeof \
	prompt = '\!> '
alias ma "make \!* >& errs &"
alias bye "clear ; exec /usr/games/fortune"
alias e /usr/bin/ex
alias ec echo \*
alias hi history
alias so source
alias una unalias
set prompt="\! #> "
umask 22
