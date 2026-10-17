# include  "d_proto.h"








unsigned short  d_lxill[8],     /*  local transmit illegal codes  */

	        d_lrill[8],     /*  local receive illegal codes  */

        	d_rxill[8],     /*  remote transmit illegal codes  */

	        d_rrill[8],     /*  remote receive illegal codes  */

	        d_lcvec[8];     /*  local transmit encoding vector  */




int  d_lxmax,             /*  local transmit maximum packet size  */

     d_lrmax,             /*  local receive maximum packet size  */

     d_rxmax,             /*  remote transmit maximum packet size  */

     d_rrmax,             /*  remote receive maximum packet size  */

     d_master,              /*  this is non-zero if this module is the  */
			    /*  protocol master.                        */

     d_rcvseq,             /*  the expected sequence number of the next  */
			    /*  packet to be received.                    */
			    /*  protocol lockup caused change: is now the */
			    /*  sequence number of the last packet received */
			    /*  and is incremented when search for new    */
			    /*  packet is initiated                      */

     d_snseq,             /*  the sequence number of the next packet    */
			    /*  be transmitted.                           */
			    /*  Changed like d_rcvseq                   */

     d_maxtext;             /*  the maximum length of the text portion  */
			    /*  of a packet.                            */




char  d_snesc,            /*  escase character to used for encoding  */
			    /*  transmissions.                         */

      d_rcvesc;            /*  escape character for decoding packets  */
			    /*  received.                              */







/*
 *     the following globals are used to implement the DATA transmit
 *     and receive queues
 */

char  d_xqueue[MAXPACKET + 2],       /*  the transmit queue  */

      *d_xqpt =  &d_xqueue[0];       /*  pointer to the next available  */
				     /*  slot in the transmit queue.    */

int  d_xqcnt;                        /*  number of bytes currently loaded  */
				     /*  in the transmit queue             */

char  d_rdqueue[MAXPACKET + 2],      /*  the receive queue  */

      *d_rdqpt = &d_rdqueue[0];      /*  pointer to the next available  */
				     /*  slot in the receive queue.     */

int  d_rdqcnt,                       /*  number of unread characters in  */
				     /*  the receive queue.              */

     d_rdeot;                        /*  non-zero if the packet currently  */
				     /*  on the queue had EOT set.         */








/*
 *     Things which are subject to tailoring for a given installation
 */

char  d_esclist[] = "`~#%$&!|^@";   /*  list of possible escape characters  */
                                    /*  in order of preference              */
