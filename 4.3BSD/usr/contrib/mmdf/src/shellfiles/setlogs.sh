#!/bin/sh

if test "$1" = erase
then
	> msg.log
	> chan.log
	> ph.log
	> ph.trn
	> omsg.log
	> ochan.log
	> oph.log
	> oph.trn
fi

cd /usr/mmdf/log
mv msg.log  omsg.log
mv chan.log ochan.log
mv ph.log   oph.log
mv ph.trn   oph.trn
> msg.log
> chan.log
> ph.log
> ph.trn
chmod 662 msg.log chan.log ph.log ph.trn
