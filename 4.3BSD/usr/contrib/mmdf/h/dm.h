/*  Domain-Table definitions
 *
 *  A Domain table contains entries that reference specific host names
 *  (i.e., symbolic/routing addresses).  A table may either be used
 *  strictly as a domain table, in which case the format is:
 *
 *      domain-name  host-name [ domain-name, ... ]
 *
 *  so that 'a.b.c' will look up 'a' in 'b.c' and find it on the
 *  left-hand side.  The right-hand part of the entry is then used
 *  as a host name and is looked up in the routing tables.  If the
 *  right-hand part has more than one field, then the first part is
 *  taken as the 'next' host to send to and the remaining parts are
 *  taken as routing references, by domain name.  The order is
 *  sequentially forward, so that the second field references the
 *  host that is to receive the message, after the one we hand it to.
 *  (This is the same as the way that SMTP orders its routing info.)
 *
 *  Alternately, a table may be a standard host table, in which case, its
 *  format is:
 *
 *      host-name address-value
 *
 *  such as 'udel-relay 10.0.0.96'.  That is, the table was originally
 *  built as a name-to-net-address table, but is doubling as a domain
 *  table.  In this case, the 'final' sub-domain name is the same as the
 *  host name, such as 'udel-relay.arpa' mapping to 'udel-relay'.
 *
 *  A table fully specifies its location within the domain hierarchy, so
 *  that a site may have a 'udel.arpa' table, containing the name of
 *  sites at udel, and also have an 'arpa' table, containing the names
 *  of other domains/sites within the arpa domain.  In the list of
 *  domain tables, 'udel.arpa' comes first, so that it intercepts
 *  references to the 'udel' sub-domain.
 *
 *  Note that this domain usage involves 'flat' tables, in that you
 *  only do one domain lookup, at this site, to acquire the symbolic
 *  host address.  This lacks some generality, but should provide some
 *  efficiency.  The design is predicated on the expectation that a
 *  site will not have many domain tables (which are subordinate to
 *  other tables at that site).
 */

struct dm_struct
{
    char    *dm_name;           /* internal name of domain              */
    char    *dm_domain;         /* full name of containing domain       */
    char    *dm_lname;          /* name of this host within the domain  */
    char    *dm_show;           /* pretty-print version of name         */
    Table   *dm_table;          /* table containing entries for domain   */
};

typedef struct dm_struct    Domain;
typedef struct dm_struct    Dmn;
/*HACK*/
#define dm_tell(thetable) (((Table *)thetable) -> dm_tpos)

Domain *dm_nm2struct ();

#define DM_NFIELD  16           /* maximum number of routing fields     */

struct dm_rtstruct
{
    int     dm_argc;            /* number of domain value fields        */
    char   *dm_argv[DM_NFIELD]; /* pointers to each field               */
    char    dm_buf[LINESIZE];   /* where to stash domain value string   */
};

typedef struct dm_rtstruct    Dmn_route;
