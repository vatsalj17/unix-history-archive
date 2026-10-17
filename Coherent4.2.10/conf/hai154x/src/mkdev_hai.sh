#!/bin/sh

###
# hai154x/src/mkdev_hai.sh
#
# A mkdev script for the hai154x module. This enables and allows tuning of
# the hai154x driver for scsi devices under Mark Williams Coherent. 
###

DEVDIR=/dev

. /usr/lib/shell_lib.sh
COMMAND_NAME=$0
source_path $0 "HOME_DIR="
parent_of $HOME_DIR "CONF_DIR="

. $CONF_DIR/bin/conf_lib.sh

echo "
The adaptec AHA-154x host adapters use a base I/O port, an
interrupt vector, and a DMA channel. The driver module must be
configured to use the values selected on the card. This script will
allow you to tune the driver for your card. If you are not sure how to
set any of the following parameters, just hit <Enter> to keep the
current setting.
"

while : ; do
	echo "
The AHA-154x can use I/O port: 0x130, 0x134, 0x230, 0x234, 0x330, 0x334."
	tune HAI154X_BASE "AHA-154x I/O Port Address"

	echo "
The AHA-154x can use Interrupt Channel: 9, 10, 11, 12, 14, 15."
	tune HAI154X_INTR "AHA-154x Interrupt Channel"

	echo "
The AHA-154x can use DMA channel: 0, 5, 6, 7."
	tune HAI154X_DMA "AHA-154x DMA Channel"

	echo "
The AHA-154x host adapter may have SCSI ID 0 through 7.
It is almost always kept at 7."
	tune HAI154X_HAID "AHA-154x Host Adapter SCSI id"

	read_input "\nIs AHA-154x hardware configuration correct" \
	  ans "y" require_yes_or_no
	is_yes $ans && break
done

exit 0
