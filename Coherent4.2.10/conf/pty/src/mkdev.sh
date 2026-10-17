# pty/mkdev - install pseudoterminals

# Get current setting of a tunable parameter into a shell variable, or die

get_tunable () {
	set "$2" $($CONF_DIR/bin/idtune -p $1)

	is_empty $3 && {
		echo "pty/mkdev: can't find number of pty's configured"
		exit 100
	}

	eval $1=$3
}

# Verify that argument is between 0 and 128, inclusive.

require_0_to_128 () {
	if [ $1 -ge 0 -a $1 -le 128 ]
	then
		return 0
	else
		echo "Enter a number from 0 to 128"
		return 1
	fi
}

. /usr/lib/shell_lib.sh
COMMAND_NAME=$0
source_path $0 "HOME_DIR="
parent_of $HOME_DIR "CONF_DIR="

TMP_TTYS=/tmp/_ttys
TMP_TTYTYPE=/tmp/_ttytype

# Default number of ptys is whatever is already configured.

case $($CONF_DIR/bin/idenable -p pty) in
pty*Y)	
	get_tunable NUPTY_SPEC NUM_PTY
	;;

pty*N)	NUM_PTY=0
	;;

*)	echo "pty/mkdev: can't tell if pty device is enabled"
	exit 1
	;;
esac


while : ; do
	echo
	echo "To use the X windowing system or the screens package"
	echo "you must have the pty driver installed."
	echo
	echo "The more pty devices you have, the more windows"
	echo "you can open, but each pty device uses memory."
	echo
	echo "You may specify from 0 to 128 pty devices."
	echo "Your system is currently configured for $NUM_PTY ptys."
	echo

	read_input "How many pty devices would you like" NUM_PTY $NUM_PTY \
		require_0_to_128 || continue

	echo

	if [ $NUM_PTY -eq 0 ]
	then
		echo "Pty driver will be disabled - no ptys specified."
	else
		echo "Pty driver will be enabled with $NUM_PTY ptys."
	fi

	read_input "Is this correct" ANS "y" require_yes_or_no || continue
	is_yes $ANS && break
done

# Update /etc/ttys and /dev entries

# Strip old pty entries from /etc/ttys.

/bin/egrep -v "ttyp" < /etc/ttys > $TMP_TTYS


# Removing nodes will have to wait for node.d revision.

# Append pty lines to ttys file, and create device nodes.

[ $NUM_PTY -eq 0 ] || {
	x=0
	while val $(($x < $NUM_PTY)) ; do
		/bin/echo 0lPttyp$x >> $TMP_TTYS
		/etc/mknod -f /dev/ttyp$x c 9 $x
		/etc/mknod -f /dev/ptyp$x c 9 $(($x + 128))
		/bin/chmog 666 bin bin /dev/ttyp$x /dev/ptyp$x
		x=$(($x + 1))
	done
}


/bin/cp $TMP_TTYS /etc/ttys

# Update /etc/ttytype

# Strip old pty entries from /etc/ttytype.

/bin/egrep -v "ttyp" < /etc/ttytype > $TMP_TTYTYPE

# Append pty lines to ttytype file.

[ $NUM_PTY -eq 0 ] || {
	x=0
	while val $(($x < $NUM_PTY)) ; do
		/bin/echo "xterm	ttyp$x		Psuedo TTY device" \
		   >> $TMP_TTYTYPE
		x=$(($x + 1))
	done
}


/bin/cp $TMP_TTYTYPE /etc/ttytype

# Delete temporary files.

/bin/rm -f $KB_DATA $TMP_TTYS $TMP_TTYTYPE


if [ $NUM_PTY -eq 0 ]
then
# Disable the device
	$CONF_DIR/bin/idenable -d pty
else
# Enable and tune the device
	$CONF_DIR/bin/idenable -e pty
	$CONF_DIR/bin/idtune -f NUPTY_SPEC $NUM_PTY
fi

exit 0
