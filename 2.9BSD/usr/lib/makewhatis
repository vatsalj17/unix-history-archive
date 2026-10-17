# makewhatis
# (c) 1979 Regents of the University of California
set nonomatch
rm -f /tmp/whatis
cd /usr/man
foreach i (man1 man2 man3 man4 man5 man6 man7 man8 manl manu manp )
	if -e $i then
		cd $i
		set j=(*.*)
		set noglob
		if $j[1] != '*.*' /usr/ucb/getNAME $j >>! /tmp/whatis
		unset noglob
		cd ..
	endif
end
ed - /tmp/whatis <<'EOF'
g/\\-/s//-/
g/\\\*-/s//-/
g/ VAX-11/s///
g/\\f./s///
1,$s/^... *[^	 ]* *\([^ 	][^ 	]*\).*	\([^-]*\)/\2(\1)	/
g/	 /s//	/g
w /tmp/whatis2
'EOF'
/usr/ucb/expand -24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100 /tmp/whatis2 | sort >! /usr/lib/whatis
rm /tmp/whatis
rm /tmp/whatis2
