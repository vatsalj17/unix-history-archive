BIN = /bin
LIB = /lib

STICKY = OLD

#		To link ncc to cc, instead of copying ncc to cc, set CP=ln.
CP = cp


#------
# n-c-o: Shuffle cc->occ, then ncc->cc, using $(BIN) & $(LIB) as deliver directories.
#	 Neither version is recompiled!
#------
n-c-o:	cc-occ
	echo "\nNCC->CC:"
	-mv $(BIN)/cc $(BIN)/$(STICKY)cc
	-mv $(LIB)/c0 $(LIB)/$(STICKY)c0
	-mv $(LIB)/c1 $(LIB)/$(STICKY)c1
	-mv $(LIB)/c2 $(LIB)/$(STICKY)c2
	-mv $(LIB)/cpp $(LIB)/$(STICKY)cpp
	$(CP) $(BIN)/ncc $(BIN)/cc
	$(CP) $(LIB)/nc0 $(LIB)/c0
	$(CP) $(LIB)/nc1 $(LIB)/c1
	$(CP) $(LIB)/nc2 $(LIB)/c2
	$(CP) $(LIB)/ncpp $(LIB)/cpp
	-chown bin $(BIN)/cc $(LIB)/c0 $(LIB)/c1 \
			$(LIB)/c2 $(LIB)/cpp
	-chmod 1755 $(BIN)/cc $(LIB)/c0 $(LIB)/c1 \
			$(LIB)/c2 $(LIB)/cpp
	ls -l $(BIN)/cc $(LIB)/c0 $(LIB)/c1 \
			$(LIB)/c2 $(LIB)/cpp
	echo "\nRemove following STICKY files after next reboot:"
	-ls -l $(BIN)/$(STICKY)* $(LIB)/$(STICKY)*


#------
# cc-occ: Copy cc version to occ, using $(BIN) & $(LIB).
#------
cc-occ:
	echo "CC->OCC:"
	-mv $(BIN)/occ $(BIN)/$(STICKY)occ
	-mv $(LIB)/oc0 $(LIB)/$(STICKY)oc0
	-mv $(LIB)/oc1 $(LIB)/$(STICKY)oc1
	-mv $(LIB)/oc2 $(LIB)/$(STICKY)oc2
	-mv $(LIB)/ocpp $(LIB)/$(STICKY)ocpp
	cp $(BIN)/cc $(BIN)/occ
	cp $(LIB)/c0 $(LIB)/oc0
	cp $(LIB)/c1 $(LIB)/oc1
	cp $(LIB)/c2 $(LIB)/oc2
	cp $(LIB)/cpp $(LIB)/ocpp
	-chown bin $(BIN)/occ $(LIB)/oc0 $(LIB)/oc1 \
			$(LIB)/oc2 $(LIB)/ocpp
	-chmod 1755 $(BIN)/occ $(LIB)/oc0 $(LIB)/oc1 \
			$(LIB)/oc2 $(LIB)/ocpp
	ls -l $(BIN)/occ $(LIB)/oc0 $(LIB)/oc1 \
			$(LIB)/oc2 $(LIB)/ocpp
	echo "Remove following STICKY files after next reboot:"
	-ls -l $(BIN)/$(STICKY)occ $(LIB)/$(STICKY)oc0 \
			$(LIB)/$(STICKY)oc1 \
			$(LIB)/$(STICKY)oc2 $(LIB)/$(STICKY)ocpp
