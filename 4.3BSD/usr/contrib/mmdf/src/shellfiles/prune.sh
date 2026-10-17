#! /bin/sh
#
#  Prune - Go through all the messages in the queue and
#          delete all the deadwood entries.
#
PATH=/usr/brl/bin:/usr/ucb:/bin:/usr/bin
export PATH

cd /usr/mmdf/lock/home/addr
for i in msg.*
do
flock $i ed $i << DONE
3,\$g/^. \* /d
f
w
q
DONE
done
