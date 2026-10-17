###
# haisd/src/mkdev_hai.sh
#
# A mkdev script for the haisd component of the hai driver. This script
# allows enabling/disable the status of the haisd driver component as well
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

if val $(($mask != 0)) ; then
	tune HAISD_MAXREQ "HAISD number of requests to look ahead"

	for i in 0 1 2 3 4 5 6 7; do
		val $((($mask & (1 << $i)) == 0)) && continue

		minor=$(($i * 16))

		/etc/mknod -f $DEVDIR/sd${i}x b 13 $(($minor + 128))
		/etc/mknod -f $DEVDIR/rsd${i}x c 13 $(($minor + 128))
		/etc/mknod -f $DEVDIR/sd${i}a b 13 $minor
		/etc/mknod -f $DEVDIR/rsd${i}a c 13 $minor
		/etc/mknod -f $DEVDIR/sd${i}b b 13 $(($minor + 1))
		/etc/mknod -f $DEVDIR/rsd${i}b c 13 $(($minor + 1))
		/etc/mknod -f $DEVDIR/sd${i}c b 13 $(($minor + 2))
		/etc/mknod -f $DEVDIR/rsd${i}c c 13 $(($minor + 2))
		/etc/mknod -f $DEVDIR/sd${i}d b 13 $(($minor + 3))
		/etc/mknod -f $DEVDIR/rsd${i}d c 13 $(($minor + 3))
		/bin/chmog 600 sys sys $DEVDIR/*sd${i}*
	done
fi

exit 0