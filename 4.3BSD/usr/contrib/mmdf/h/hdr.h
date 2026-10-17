/* hdr_read return values */

#define HDR_NAM -1                /* no name                            */
#define HDR_EOH  0                /* end of headers                     */
#define HDR_OK   1                /* nice header                        */
#define HDR_NEW  2                /* new header field                   */
#define HDR_MOR  3                /* continuation of header field       */
