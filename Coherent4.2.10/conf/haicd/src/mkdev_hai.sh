###
# haicd/src/mkdev_hai.sh
#
# A mkdev script for the haicd component of the hai driver. This script
# allows enabling/disable the status of the haict driver component as well
# as tuning its parameters.
###

DEVDIR=/dev
count=$1
mask=$2

. /usr/lib/shell_lib.sh
COMMAND_NAME=$0
source_path $0 "HOME_DIR="
parent_of $HOME_DIR "CONF_DIR="

. $CONF_DIR/bin/conf_lib.sh

if is_empty $count || is_empty $mask ; then
	exit 1
fi

count=0
linkdevcdrom=1
if val $(($mask != 0)) ; then
	unit=0
	for i in 0 1 2 3 4 5 6 7; do
		val $((($mask & (1 << $i)) == 0)) && continue

		minor=$(($i * 16))

		/etc/mknod -f $DEVDIR/Scdrom${unit} c 13 $minor
		/bin/chmog 444 sys sys $DEVDIR/*Scdrom${unit}

		if val $(($linkdevcdrom != 0)) ; then
			read_input "Set default CD-ROM device to SCSI CD-ROM drive at ID $i" \
				   ans \
				   "n" \
				   require_yes_or_no 
			if is_yes $ans ; then
				ln -f $DEVDIR/Scdrom${unit} $DEVDIR/cdrom
				linkdevcdrom=0
			fi
		fi
		unit=$(($unit + 1))
	done
fi

exit 0