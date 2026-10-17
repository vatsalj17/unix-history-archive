# idbld
# Revised: Tue Mar  1 10:05:08 1994 CST

. /usr/lib/shell_lib.sh
source_path $0 "HOME_DIR="
parent_of $HOME_DIR "CONF_DIR="

. $CONF_DIR/bin/conf_lib.sh

# For testing, make TESTMODE nonempty.

TESTMODE=
if is_empty $TESTMODE; then
	DO=
else
	CONF_DIR=/etc/conf
	DO=/bin/echo
fi


###############
# usage
#
# Report a usage message to standard error.
###############

usage () {
	echo "Usage: /etc/conf/bin/idbld [ -o kernel_file ]"
	echo "  In order to be bootable, kernel_file must be in the root directory."
        echo "  If no output file is specified, the default is /coherent."

	exit 1
}

###############
# Main actions of this script:
#   Run all hard drive mkdev scripts, in order hai, ss.
#   Run all other mkdev scripts.
#   Link a new kernel.
###############

# Parse command line.

TARGET=/coherent

while [ $# -gt 0 ]; do
	ARG=$1
	case $ARG in
	-o)
		shift
		if [ $# -ne 1 ]; then
			usage
		fi
		TARGET=$1
		;;
	*)
		usage
		;;
	esac
	shift
done

if [ $# -ne 0 ]; then
	usage
fi

# Run mkdev scripts.

$DO $CONF_DIR/at/mkdev

# SCSI: only one of hai, ss may be enabled.  aha is obsolete, omitted.

# See if any SCSI device is enabled

scsi_used=
scsi_unused="hai ss"
if is_enabled hai; then
	scsi_used=hai
	scsi_unused="ss"
elif is_enabled ss; then
	scsi_used=ss
	scsi_unused="hai"
fi

# If a device is enabled, allow disabling that device before checking others.

is_empty $scsi_used || {
	$DO $CONF_DIR/$scsi_used/mkdev
	is_enabled $scsi_used || scsi_used=
}

# Now, if no device is enabled, allow enabling others.
is_empty $scsi_used && for scsi_dev in $scsi_unused ; do
	$DO $CONF_DIR/$scsi_dev/mkdev
	is_enabled $scsi_dev && break
done

for M in $CONF_DIR/*/mkdev; do
	case $M in
	*/hai*/mkdev|*/ss/mkdev|*/at/mkdev)
		continue
		;;
	*)
		$DO $M || exit 1
		;;
	esac
done

# Link new target kernel.
$DO $CONF_DIR/bin/idmkcoh -o $TARGET || exit 1

exit 0
