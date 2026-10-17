#! /bin/sh
: This shell file makes a new smtp host table, based upon
: the NIC host table.
nictable -C < /etc/nic-table > NEWsmtp
mv -f smtp smtp.bak; mv -f NEWsmtp smtp

: Here, we make a new Arpa Domain table.
nictable -D < /etc/nic-table > NEWarpa
mv -f arpa arpa.bak; mv -f NEWarpa arpa
