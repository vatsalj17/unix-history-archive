#ifndef lint
static	char sccsid[] = "@(#)ypserv_map.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/*
 * This contains all yp server functions which know about and manipulate the
 * yellow pages server's map data bases.
 */

#include "ypsym.h"
#include <ctype.h>
char current_map[YPDBPATH_LENGTH + YPMAXDOMAIN + YPMAXMAP + 3];
struct map_list_item *current_pmap;
char ypmaps[] = YPMAPS;

/*
 * This performs an existence check on the dbm data base files <name>.pag and
 * <name>.dir.  pname is a ptr to the filename.  This should be an absolute path.
 * Returns TRUE if the map exists and is accessable; else FALSE.
 *
 * Note:  The file name should be a "base" form, without a file "extension" of
 * .dir or .pag appended.  See ypmkfilename for a function which will generate
 * the name correctly.  Errors in the stat call will be reported at this level,
 * however, the non-existence of a file is not considered an error, and so will
 * not be reported.
 */
bool
ypcheck_map_existence(pname)
	char *pname;
{
	char dbfile[MAXNAMLEN + 1];
	struct stat filestat;
	int len;

	if (!pname || ((len = strlen(pname)) == 0) ||
	    (len + 5) > (MAXNAMLEN + 1) ) {
		return(FALSE);
	}
		
	errno = 0;
	strcpy(dbfile, pname);
	strcat(dbfile, ".dir");

	if (stat(dbfile, &filestat) != -1) {
		strcpy(dbfile, pname);
		strcat(dbfile, ".pag");

		if (stat(dbfile, &filestat) != -1) {
			return(TRUE);
		} else {

			if (errno != ENOENT) {
				fprintf(stderr,
				"ypserv:  Stat error on map file %s.\n", dbfile);
			}

			return(FALSE);
		}

	} else {

		if (errno != ENOENT) {
			fprintf(stderr, "ypserv:  Stat error on map file %s.\n",
			    dbfile);
		}


		return(FALSE);
	}

}

/*
 * This tests whether a particular map is supported. The definition of a
 * supported map is a well-formed (that is, has a retrievable yp order number)
 * known map in a supported domain.  Supported maps are not necessarily
 * updatable:  we may not know a valid peer address for the map's master.
 * Returns TRUE if the map is supported, else FALSE.
 */
struct map_list_item *
ypcheck_map(map, domain)
	char *map;
	char *domain;
{
	struct domain_list_item *pdom;
	struct map_list_item *pmap;
	int lenm, lend;

	if (!map || ((lenm = strlen(map)) == 0) || (lenm > YPMAXMAP) ||
	    !domain || ((lend = strlen(domain)) == 0) || (lend > YPMAXDOMAIN)) {
		return( (struct map_list_item *) NULL);
	}

	if ( (pdom = ypcheck_domain(domain)) !=
	    (struct domain_list_item *) NULL) {		/* Supported domain? */
		    
		if ( (pmap = yppoint_at_map(map, pdom)) !=
		    (struct map_list_item *) NULL) {	/* Known map? */ 
			    
			if (pmap->map_supported) {	/* Supported map? */
				return(pmap);
			} else {
				return((struct map_list_item *) NULL);
			}
			
		} else {
			return((struct map_list_item *) NULL);
		}
				
	} else {
		return((struct map_list_item *) NULL);
	}
	
}

/*
 * This builds the list of all known maps for each supported domain. 
 */
void
ypget_all_maps()
{
	struct domain_list_item *pdom;

	/* Do for all supported domains. */
	
	for (pdom = yppoint_at_first_domain(TRUE);
	    pdom != (struct domain_list_item *) NULL;
	    pdom = yppoint_at_next_domain(pdom, TRUE) ) { 
		ypget_dom_all_maps(pdom);
	}
}

/*
 * This builds a maplist for a domain which contains all the existing maps
 * within the domain.  The domain's maplist may be changed.  In addition, for
 * maps for which this is the first assignment of a master, or a change
 * of master, an entry will be made on the map transfer list.
 *
 * Note:  It is assumed that no lower level will change the current map inside
 * the "Do for all known maps" loop, without preserving the current map.
 */
void
ypget_dom_all_maps(pdom)
	struct domain_list_item *pdom;
{
	bool status;
	datum key;
	datum val;
	int error;
	struct peer_list_item *pmaster;
	char map[YPMAXMAP + 1];
	struct map_list_item *pmap;

	if (!pdom) {
		return;
	}
	
	if (!ypset_current_map(ypmaps, pdom->dom_name, &error) ) {
		fprintf(stderr,
	"ypserv:  ypget_dom_all_maps can't access ypmaps for domain %s.\n",
		  pdom->dom_name);
		return;
	}

	key = firstkey();

	if (key.dptr != NULL) {

		/* Do for all known maps */

		for (; key.dptr != NULL; key = nextkey(key)) {

			/*
			 * Knock out key-value pairs from the
			 * map file which are yp private symbols.
			 */

			if (key.dsize >= YPSYMBOL_PREFIX_LENGTH &&
			    (!bcmp(key.dptr, YPSYMBOL_PREFIX,
			    YPSYMBOL_PREFIX_LENGTH) ) ) {
				continue;
			}
					
			val = fetch(key);

			if (val.dptr != (char *) NULL) {
				pmaster = ypget_map_master(pdom, &val);
				bcopy(key.dptr, map, key.dsize);
				map[key.dsize] = '\0';

				if (ypadd_one_map(map, pdom, pmaster)) {

					if ((pmap = yppoint_at_map(map, pdom)) !=
					    (struct map_list_item *) NULL) {

						if ( (pmap->map_master ==
					       (struct peer_list_item *) NULL) ||
						 (pmap->map_master != pmaster)) {
							pmap->map_master =
							    pmaster;
						 (void) ypadd_xfr(pmap);
						}
						
					} else {
						fprintf(stderr,
		  "ypserv:  ypget_dom_all_maps: can't point at existing map.\n");
					}
					
				} else {
					fprintf(stderr,
	     "ypserv:  ypget_dom_all_maps: failed to add map %s in domain %s.\n",
	     				    pmap->map_name, pdom->dom_name);
				}

			} else {
				fprintf(stderr,
		  "ypserv:  ypget_dom_all_maps:  garbage key %.*s, domain %s.\n",
				    key.dsize, key.dptr, pdom->dom_name);
			}
		}

	} else {
		fprintf(stderr,
	      "ypserv:  ypget_dom_all_maps:  no entries in ypmaps, domain %s.\n",
		    pdom->dom_name);
	}
}

/*
 * This goes through the lists of known maps for each supported domain, and
 * marks as supported each map which exists, and contains a retrievable order
 * number.  Maps already marked as supported will be skipped.  The map_supported
 * field of supported maps will be set TRUE, and the map_order field will be
 * set to the value of the symbol in the map (all work done at lower levels).
 */
void
ypget_supported_maps()
{
	struct domain_list_item *pdom;

	for (pdom = yppoint_at_first_domain(TRUE);
	    pdom != (struct domain_list_item *) NULL;
	    pdom = yppoint_at_next_domain(pdom, TRUE) ) {
		ypget_dom_supported_maps(pdom);
	}
}

/*
 * Maps within the domain may be marked as existing, and as supported.  Maps
 * already supported will remain unchanged.
 *
 * Note:  1.  In any case in which map_supported remains FALSE, map_order is
 * invalid.  2.  Nothing may be assumed about the current map over the call
 * to ypget_supported_maps.
 */
void
ypget_dom_supported_maps(pdom)
	struct domain_list_item *pdom;
{
	struct map_list_item *pmap;
	char map[MAXNAMLEN + 1];
	int error;
		
	if (!pdom) {
		return;
	}
	
	for (pmap = yppoint_at_maplist(pdom);
	    pmap != (struct map_list_item *) NULL;
	    pmap = yppoint_at_next_map(pmap) ) {

		if (pmap->map_supported == TRUE) {
			continue;
		}

		ypmkfilename(pdom->dom_name, pmap->map_name, map);

		if (ypcheck_map_existence(map) ) {
					
			pmap->map_exists = TRUE;

			if(ypget_map_order(pmap) ) {
				pmap->map_supported = TRUE;
			}
		}
	}
}

/*
 * This allocates memory for a single map_list_item, fills in the map_name
 * field from the passed name, links the item to the map list of the passed
 * domain, marks the map as unsupported, and points the map_master field to
 *the input master.  The map_alternate field will be set to NULL.  Returns TRUE
 * if link succeeded, else FALSE.
 *
 * Note:  This function will return TRUE if a list element already exists
 * on the list of known maps with the same mapname.  The existing map list
 * element will have field map_in_new_map set TRUE.
 */
bool
ypadd_one_map(map, pdom, master)
	char *map;
	struct domain_list_item *pdom;
	struct peer_list_item *master;
{
	struct map_list_item *pmap;
	int len;

	if (!map || ((len = strlen(map)) == 0) || (len > YPMAXMAP) || !pdom) {
		return(FALSE);
	}

	if ((pmap = yppoint_at_map(map, pdom)) != (struct map_list_item *)NULL) {
		pmap->map_in_new_map = TRUE;
		return(TRUE);
	}

	if ( (pmap =
	    (struct map_list_item *) malloc(sizeof(struct map_list_item) ) ) !=
	    (struct map_list_item *) NULL) {
		pmap->map_pnext = pdom->dom_pmaplist; /* An element, or NULL */
		pdom->dom_pmaplist = pmap;	/* Make new one the head */
		strcpy(pmap->map_name, map);
		pmap->map_domain = pdom;
		pmap->map_exists = FALSE;
		pmap->map_supported = FALSE;
		pmap->map_in_new_map = TRUE;
		pmap->map_order = 0;
		pmap->map_last_polled = 0;
		pmap->map_master = master;
		pmap->map_alternate = (struct peer_list_item *) NULL;
		return(TRUE);
	} else {
		fprintf(stderr, "ypserv:  ypadd_one_map had malloc failure.\n");
		return(FALSE);
	}
	
}

/*
 * This removes a map_list_item from a named domain.
 */
void
ypdel_one_map(pmap)
	struct map_list_item *pmap;
{
	struct map_list_item *scan;

	if (!pmap) {
		return;
	}

	ypdel_xfr(pmap);
	
	if ( (scan = yppoint_at_maplist(pmap->map_domain) ) !=
	    (struct map_list_item *) NULL) {

		if (scan == pmap) {		/* pmap is head entry */
			pmap->map_domain->dom_pmaplist = pmap->map_pnext;
			free(pmap);
		} else {			/* pmap is not head, or not
						 *   on list */
			for (;
			    ( (scan != (struct map_list_item *) NULL) &&
			    (scan->map_pnext != pmap) );
			    scan = scan->map_pnext ) {
			}


			if (scan !=  (struct map_list_item *) NULL) {
				scan->map_pnext = pmap->map_pnext;
				free(pmap);
			} else {
				fprintf(stderr,
			"ypserv:  ypdel_one_map can't find map on maplist.\n");
			}
			
		}
	} else {
		fprintf(stderr,
		"ypserv:  ypdel_one_map called for domain with null maplist.\n");
	}

}

/*
 * This returns a pointer to the map_list_item associated with a named map in a
 * named domain.  Returns ptr to the map_list_item if it was found, else NULL.
 */
struct map_list_item *
yppoint_at_map(map, pdom)
	char *map;
	struct domain_list_item *pdom;
{
	struct map_list_item *pmap;
	int len;

	if ( (map == (char *) NULL) || ( (len = strlen(map) ) == 0) ||
	    (len > YPMAXMAP) || (pdom == (struct domain_list_item *) NULL)) {
		return( (struct map_list_item *) NULL);
	}

	pmap = yppoint_at_maplist(pdom);

	while ( (pmap != (struct map_list_item *) NULL) &&
	    (strcmp(map, pmap->map_name) != 0) ) {
		pmap = pmap->map_pnext;
	}

	return(pmap);
}

/*
 * This adds a named map to the list of maps associated with a particular
 * domain, and checks to see if it is supportable or not.  It performs the
 * functions of ypget_known_maps and ypget_supported_maps for a single named
 * map.  Returns TRUE is success, FALSE otherwise.   The domain's maplist may
 * be changed.  In addition, the map's newly made list entry may be marked as
 * existing, and perhaps as supported, as is appropriate.
 *
 * Note:  If the named map already exists in maplist of the passed domain,
 * the function will return TRUE, but will perform no other action.
 */
bool
ypadd_named_map(map, pdom)
	char *map;
	struct domain_list_item *pdom;
{
	char mapname[MAXNAMLEN + 1];
	struct map_list_item *pmap;
	int len;

	if ( (map == (char *) NULL) || ( (len = strlen(map) ) == 0) ||
	    (len > YPMAXMAP) || (pdom == (struct domain_list_item *) NULL)) {
		return(FALSE);
	}

	if (pmap =  yppoint_at_map(map, pdom) ) {
		return(TRUE);
	}
					
	ypmkfilename(pdom->dom_name, map, mapname);
					    
	if (ypadd_one_map(map, pdom, NULL)) {
		pmap = yppoint_at_map(map, pdom);
			
		if (ypcheck_map_existence(mapname)) {
			pmap->map_exists = TRUE;
							
			if(ypget_map_order(pmap) ) {
				pmap->map_supported = TRUE;
				return(TRUE);
			} else {
				return(FALSE);
			}

		} else {
			return(FALSE);
		}
						
	} else {
		fprintf(stderr,
		    "ypserv: ypadd_named_map can't add map %s in domain %s.\n",
        	    map, pdom->dom_name);
		return(FALSE);
    	}
}

/*
 * This returns a ptr to the master of a particular map in a particular domain.
 */
struct peer_list_item *
yppoint_at_map_master(pmap)
	struct map_list_item *pmap;
{
	if (pmap) {
		return(pmap->map_master);
	} else {
		return( (struct peer_list_item *) NULL);
	}
}

/*
 * This returns a ptr to the alternate master of a particular map in a
 * particular domain.
 */
struct peer_list_item *
yppoint_at_map_alternate(pmap)
	struct map_list_item *pmap;
{
	if (pmap) {
		return(pmap->map_alternate);
	} else {
		return( (struct peer_list_item *) NULL);
	}
}

/*
 * This returns a string mapname, given a pointer to a map_list_item;
 */
char *
yppoint_at_mapname(pmap)
	struct map_list_item *pmap;
{
	if (pmap) {
		return(pmap->map_name);
	} else {
		return( (char *) NULL);
	}
}

/*
 * This returns a pointer to the next map_list_item on a map list, or NULL.
 */
struct map_list_item *
yppoint_at_next_map(pmap)
	struct map_list_item *pmap;
{
	if (pmap) {
		return(pmap->map_pnext);
	} else {
		return( (struct map_list_item *) NULL);
	}
}

/*
 * The retrieves the order number of a named map, as known from the order
 * number datum in the map data base, and loads it into the map_order field of
 * the passed map_list_item.
 */
bool
ypget_map_order(pmap)
	struct map_list_item *pmap;
{
	bool status;
	int error;
	datum key;
	datum val;
	char toconvert[MAX_ASCII_ORDER_NUMBER_LENGTH + 1];

	if (!pmap || !pmap->map_exists) {
		return(FALSE);
	}
	
	if (ypset_current_map(pmap->map_name,
	    pmap->map_domain->dom_name, &error) ) {
		key.dptr = order_key;
		key.dsize = ORDER_KEY_LENGTH;
		val = fetch(key);

		if (val.dptr != (char *) NULL) {

			if (val.dsize > MAX_ASCII_ORDER_NUMBER_LENGTH) {
				return(FALSE);
			}

			/*
			 * This is getting recopied here because val.dptr
			 * points to static memory owned by the dbm package,
			 * and we have no idea whether numeric characters
			 * follow the order number characters, nor whether
			 * the mess is null-terminated at all.
			 */

			bcopy(val.dptr, toconvert, val.dsize);
			toconvert[val.dsize] = '\0';
			pmap->map_order = (unsigned long) atol(toconvert);
			return(TRUE);
		} else {
		    return(FALSE);
		}
		    
	} else {
		return(FALSE);
	}
}

/*
 * The returns the order number of a named map, as known from the map_order
 * field from the map's list entry.
 */
bool
ypreturn_map_order(map, domain, order)
	char *map;
	char *domain;
	unsigned long *order;
{
	struct map_list_item *pmap;

	/* (Let ypcheck_map do parameter checking) */

	if ( (pmap = ypcheck_map(map, domain) ) != 
	    (struct map_list_item *) NULL) {
		*order = pmap->map_order;
		return(TRUE);
	} else {
		return(FALSE);
	}

}

/*
 * This makes a map into the current map, and calls dbminit on that map so that
 * any successive dbm operation is performed upon that map.  The map need not
 * be supported (that is, it need not be well-formed) but it must exist.
 * Returns YP_xxxx error code in error if FALSE.
 */
bool
ypset_current_map(map, domain, error)
	char *map;
	char *domain;
	int *error;
{
	char mapname[YPDBPATH_LENGTH + YPMAXDOMAIN + YPMAXMAP + 3];
	struct domain_list_item *pdom;
	struct map_list_item *pmap;
	bool retval;
	int lenm, lend;

	if (!map || ((lenm = strlen(map)) == 0) || (lenm > YPMAXMAP) ||
	    !domain || ((lend = strlen(domain)) == 0) || (lend > YPMAXDOMAIN)) {
		return(FALSE);
	}

	if ( (pdom = ypcheck_domain(domain) ) !=
	    (struct domain_list_item *) NULL ) {

		if ( (pmap = yppoint_at_map(map, pdom) ) !=
		    (struct map_list_item *) NULL ) {

			if (!pmap->map_exists) {
				return(FALSE);
			}
	
			ypmkfilename(domain, map, mapname);
			
			if (strcmp(mapname, current_map) == 0) {
				retval = TRUE;
			} else {

				if (current_map[0] != '\0') {
					dbmclose(current_map);
				};

				strcpy(current_map, mapname);
				current_pmap = pmap;

				if (dbminit(mapname) >= 0) {
					retval = TRUE;
				} else {
					current_map[0] = '\0';
					current_pmap =
					    (struct map_list_item *) NULL;
					*error = YP_BADDB;
					retval = FALSE;
				}
			}

		} else {
			*error = YP_NOMAP;
			retval = FALSE;
		}

	} else {
		*error = YP_NODOM;
		retval = FALSE;
	}

	return(retval);
}

/*
 * This is a clumsy flavor of yppoint_at_peer which pulls the peer name out of
 * a datum.  The peer name should begin at the first (0th) character position
 * in the data, but this function will take care of stuff which has whitespace
 * following the peer name, or stuff which is in dbm's static data space with
 * unknown garbage (or the end of the world) after it.
 */
struct peer_list_item *
ypget_map_master(pdom, precord)
	struct domain_list_item *pdom;
	datum *precord;
{
	char *scan;
	char *peerstring;
	int charcount = 0;
	char charsave = '\0';
	struct peer_list_item *retval;

	if (!pdom || !precord || precord->dsize == 0) {
		return((struct peer_list_item *) NULL);
	}
		
	scan = precord->dptr;

	while ( (charcount < precord->dsize) && (!isspace(*scan) ) ) {
		scan++;
		charcount++;
	}

	if (charcount == precord->dsize) {	/* No whitespace in record */

		if ( (peerstring = (char *) malloc(precord->dsize + 1) ) !=
		    (char *) NULL) {
			    bcopy(precord->dptr, peerstring, precord->dsize);
			    peerstring[ precord->dsize] = '\0';
		} else {
			fprintf(stderr,
			    "ypserv: ypget_map_master: malloc failure.\n");
			return((struct peer_list_item *) NULL);
		}
		
	} else {				/* There is whitespace */
		charsave = *scan;
		*scan = '\0';
		peerstring = precord->dptr;
	}

	retval = yppoint_at_peer(peerstring, pdom);

	if (charsave == '\0') {			/* We allocated memory */
		free(peerstring);
	} else {				/* We changed a char */
		*scan = charsave;
	}

	return(retval);
}
