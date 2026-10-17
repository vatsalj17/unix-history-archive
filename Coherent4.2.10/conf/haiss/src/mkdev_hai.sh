#!/bin/sh

###
# haiss/mkdev - get host adapter card tunables.
###

DEVDIR=/dev

. /usr/lib/shell_lib.sh
COMMAND_NAME=$0
source_path $0 "HOME_DIR="
parent_of $HOME_DIR "CONF_DIR="
. $CONF_DIR/bin/conf_lib.sh

while : ; do
	echo "
0) Seagate ST01/02
1) Future Domain TMC-845/850/860/875/885
2) Future Domain TMC-840/841/880/881
"
	tune HAISS_TYPE "Seagate/Future Domain SCSI Controller Model"

	echo "
Possible IRQ's are: 3, 4, 5, 10, 11, 12, 14, 15.
(Not all adapters support all IRQ numbers.)"
	tune HAISS_INTR "SCSI Controller IRQ Number"

	echo "
Possible base physical segment addresses are:
  0xC800, 0xCA00, 0xCC00, 0xCE00, 0xDC00, 0xDE00.
(Not all adapters support all addresses.)"
	tune HAISS_BASE "SCSI Controller Base Segment"

	read_input \
	  "Is Seagate/Future Domain SCSI hardware configuration correct" \
	  ans "y" require_yes_or_no
	is_yes $ans && break
done
exit 0
