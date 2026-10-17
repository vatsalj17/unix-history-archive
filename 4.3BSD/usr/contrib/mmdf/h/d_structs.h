#
/*
 *     structure of file descriptors returned from 'pipe'
 */

struct  pipedesc
    {
    int  p_readfd;                  /*  read file descriptor  */
    int  p_writefd;                 /*  write file descriptor  */
};




/*
 *     this structure contains the names and relationships of the available
 *     dial out ports, their lock files, and the acu name.
 *
 *     Feb 84  Dan Long  added p_type for line selection
 */

struct  dialports
    {
    char  *p_port;                  /*  path name of port  */
    char  *p_lock;                  /*  path name of lock file  */
    char  *p_acu;                   /*  path name of acu device  */
    char  *p_acupref;               /*  initialization string for acu */
    char  *p_acupost;               /*  dialing termination string, for acu */
    int    p_speed;                 /*  port speed index  */
    char  *p_ltype;                 /*  line type (e.g. WATS) */
    char  *p_access;                /*  path name of access file  */
};




/*
 *     this structure identifies the available direct connect lines, their
 *     speeds, lock files, and access files.
 */

struct directlines
    {
    char  *l_name;                    /*  line name  */
    char  *l_tty;                     /*  local tty path name  */
    char  *l_lock;                    /*  lock file path name  */
    int  l_speed;                     /*  speed index  */
    char  *l_access;                  /*  path name of access list  */
};




/*
 *     structure passed to 'd_dial' which contains the phone number/port
 *     speed pairings for the possible connection paths.
 */

struct  telspeed
    {
    int  t_speed;                   /*  speed index  */
    char  t_number[40];             /*  phone number in canonical form  */
};




/*  structure of speed names/index pairs.  */

struct  speedtab
    {
    char  *s_speed;                 /*  pointer to speed name string  */
    int  s_index;                   /*  speed index  */
};


/*  A structure used in keeping track of packets as they are sent  */

struct pkbuff
    {
    char p_data[MAXPACKET + 2];	/*  the actual data to be sent	*/
    int p_length;		/*  # of chars in the data	*/
    int p_seqnum;		/*  the sequence number		*/
    int p_acktype;		/*  the ack type expected	*/
    int p_ntries;		/*  # of times it has been sent	*/
    int p_timeo;		/*  time out value for ACK	*/
    char p_pktype;		/*  the packet type		*/
};



/*  Used to keep track of the script files that are open  */
struct opfile
  {
  char o_fname[82];
  FILE *o_chan;
  unsigned o_line;
  int o_nfields;
  char *o_fields[MAXFIELDS];
};
