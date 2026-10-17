###
# hai/src/mkdev.sh
#
# A mkdev script for the hai driver. This enables and allows tuning of
# the hai driver for scsi devices under Mark Williams Coherent. 
###

DEVDIR=/dev

. /usr/lib/shell_lib.sh
COMMAND_NAME=$0
source_path $0 "HOME_DIR="
parent_of $HOME_DIR "CONF_DIR="

. $CONF_DIR/bin/conf_lib.sh

get_scsi_id_mask() {
	__devicetype=$1
	__oldidmask=$2

	__oldcount=0
	for i in 0 1 2 3 4 5 6 7 ; do
		if val $((($__oldidmask & (1 << $i)) != 0)) ; then
			__oldcount=$(($__oldcount + 1))
		fi
	done

	__idmask=$__oldidmask
	while : ; do
		if val $(($__idmask == 0)) ; then
			echo "No SCSI $__devicetype drives are configured."
		else
			echo "$__devicetype drives are configured at the following SCSI IDs"
			for i in 0 1 2 3 4 5 6 7 ; do
				val $((($__idmask & (1 << $i)) != 0)) && echo -n $i " "
			done
			echo
		fi
		read_input "Is this correct" \
			   __ans \
			   "y" \
			   require_yes_or_no || continue
		is_yes $__ans && break

		read_input \
  "Enter $__devicetype SCSI ID's separated by spaces, or <Enter> if none" \
		  __idlist ""

		__idmask=0
		for __i in $__idlist ; do
			is_numeric $__i || {
				echo $__i is not a number. Please try again.
				continue 2
			}
			val $(($__i < 0 || $__i > 7)) && {
				echo $__i is not a valid SCSI ID. Choose an ID from 0 to 7.
				continue 2
			}

			__idmask=$(($__idmask | (1 << ($__i))))
		done
	done
	eval $3=$__idmask
	eval $4=$__oldcount
}

echo "\nHAI: Host adapter independent SCSI driver"

set_enable_status hai haict haisd hai154x haiss

# Driver configuration..

read_input "
Do you want to change configuration of the Adaptec host adapter,
or add or delete SCSI devices" \
  ANS "n" require_yes_or_no

###
# Change Driver configuration. Do the Common things first then change the 
# configuration of the individual bits.
###

is_yes $ANS || exit 0

echo "
If you are not sure how to set any of the following parameters, just hit
<Enter> to keep the current setting.  Further information can be found
in your host adapter documentation, or any of the following files:

     For all COHERENT parameters: 
          /etc/conf/mtune

     For HAI parameters common to all host adapters and devices:
          /etc/conf/hai/Space.c

     For parameter specific supported host adapters:
          /etc/conf/hai154x/Space.c
          /etc/conf/haiss/Space.c

     For parameters specific to SCSI Devices:
          /etc/conf/haict/Space.c
          /etc/conf/haisd/Space.c
"
echo -n "Hit <Enter>: "
read JUNK

###
# Pick a host adapter first. 
###

get_tunable HAI_HAINDEX_SPEC DFLT
while : ; do
	echo "
You must set the host adapter first. There are two types available:

	0) The Adaptec AHA-1540/1542 and compatibles.
	1) The Seagate ST01/02 or Future Domain TMC-850
	
	q) Exit hai configuration script

"
	read_input "Enter your host adapter type" ANS $DFLT
	case $ANS in
	q)
		exit 1
		;;
	0)
		$CONF_DIR/hai154x/mkdev_hai || exit 1
		$CONF_DIR/bin/idtune -f HAI_HAINDEX_SPEC $ANS
		$CONF_DIR/bin/idenable -e hai154x
		$CONF_DIR/bin/idenable -d haiss
		break
		;;
	1)
		$CONF_DIR/haiss/mkdev_hai || exit 1
		$CONF_DIR/bin/idtune -f HAI_HAINDEX_SPEC $ANS
		$CONF_DIR/bin/idenable -d hai154x
		$CONF_DIR/bin/idenable -e haiss
		break
		;;
	*)
		echo $ANS is an invalid response please try again.
	esac
done

###
# Do common stuff first. Since both haiscsi.c and the host adapter module
# need HAI_DISK_SPEC and HAI_TAPE_SPEC, tune them here and 
# pass a value to their individual mkdev scripts.
###

###
# haisd: SCSI Disk drive configuration
###

get_tunable HAI_DISK_SPEC IDMASK
get_scsi_id_mask "Hard disk" $IDMASK IDMASK OLDCOUNT

$CONF_DIR/bin/idtune -f HAI_DISK_SPEC $IDMASK
if val $(($IDMASK != 0)) ; then 
	$CONF_DIR/haisd/mkdev_hai $OLDCOUNT $IDMASK
	$CONF_DIR/bin/idenable -e haisd
else
	$CONF_DIR/bin/idenable -d haisd
fi

###
# haict: SCSI tape drive configuration
###

get_tunable HAI_TAPE_SPEC IDMASK
get_scsi_id_mask "Tape" $IDMASK IDMASK OLDCOUNT

$CONF_DIR/bin/idtune -f HAI_TAPE_SPEC $IDMASK
if val $(($IDMASK != 0)) ; then 
	$CONF_DIR/haict/mkdev_hai $OLDCOUNT $IDMASK
	$CONF_DIR/bin/idenable -e haict
else
	$CONF_DIR/bin/idenable -d haict
fi

exit 0
