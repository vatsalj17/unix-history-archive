###
# haict/src/mkdev_hai.sh
#
# A mkdev script for the haict component of the hai driver. This script
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

get_tunable PHYS_MEM_SPEC phys_mem
get_tunable HAICT_CACHE cache
ct_mem=$(($cache * $count))
other_mem=$(($phys_mem - $ct_mem))
if val $(($other_mem < 0)) ; then
	other_mem=0
fi

count=0
linkdevtape=1
if val $(($mask != 0)) ; then
	tune HAICT_CACHE "HAI Tape Cache size "

	unit=0
	for i in 0 1 2 3 4 5 6 7; do
		val $((($mask & (1 << $i)) == 0)) && continue

		minor=$(($i * 16 + 128))

		/etc/mknod -f $DEVDIR/nrStp${unit} c 13 $(($minor + 1))
		/bin/ln -f $DEVDIR/nrStp${unit} $DEVDIR/nrmt${unit}
		/etc/mknod -f $DEVDIR/rStp${unit} c 13 $(($minor + 3))
		/bin/ln -f $DEVDIR/rStp${unit} $DEVDIR/mt${unit}
		/bin/chmog 666 sys sys $DEVDIR/*Stp${unit}

		if val $(($linkdevtape != 0)) ; then
			read_input "Set default tape device to SCSI tape drive at ID $i" \
				   ans \
				   "n" \
				   require_yes_or_no 
			if is_yes $ans ; then
				ln -f $DEVDIR/nrStp${unit} $DEVDIR/tape
				linkdevtape=0
			fi
		fi
		count=$(($count + 1))
		unit=$(($unit + 1))
	done
fi

get_tunable HAICT_CACHE cache
ct_mem=$(($cache * $count))

while : ; do
	echo "
Physical memory configuration:

     HAI tape module:         $ct_mem
     Other device drivers:    $other_mem
     -----------------------------------
     Total required:          $(($ct_mem + $other_mem))

     Current allocated:       $phys_mem
"
	if val $(($phys_mem != $ct_mem + $other_mem)) ; then
		change_pmem=y
	else
		change_pmem=n
	fi
	read_input "Change Physical memory allocation" \
		   ans \
		   $change_pmem \
		   require_yes_or_no

	is_yes $ans || break

	read_input "How many bytes of physical memory should be allocated" \
		   i \
		   $(($ct_mem + $other_mem)) \
		   ""
	
	is_numeric $i || {
		echo $i is not a number. Please try again.
		continue
	}
	$CONF_DIR/bin/idtune -f PHYS_MEM_SPEC $i
	phys_mem=$i
done

exit 0