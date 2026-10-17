#csh login file
set noglob; eval `tset -s -Q -n -m du:\?adm3a`; unset noglob
setenv	PATH .:/bin:/usr/ucb:/usr/bin
setenv	EXINIT	'set noterse prompt'
# The next line sets up vispell if the # is deleted.
# setenv EXINIT 'map #1 Gi/\<A\>"add@a|map #2 1G\!Gvispell'
msgs -q
