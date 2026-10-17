lmask p	/bin/setlab -p - /bin/osetlab
set -e
	/bin/cp	/bin/setlab /bin/osetlab	# make a backup
lmask p	/bin/setlab -p -\ n /bin/osetlab
	/bin/cmp /bin/setlab /bin/osetlab	# vet the backup
lmask p	/bin/setlab -p xnp /bin/osetlab
lmask p	/bin/osetlab -p - /bin/setlab		# does backup work?
	/bin/echo backup setlab: /bin/osetlab
	/bin/cp	./setlab /bin/setlab		# install
lmask p	/bin/osetlab -p -\ n /bin/setlab
	/bin/cmp ./setlab /bin/setlab		# vet
lmask p /bin/osetlab -p xnp /bin/setlab
