#
/*  the definitions needed to use the package which accesses
 *  the /etc/ttys file.
 */

struct ttys
    {
    int t_valid;
    int t_code;
    char t_name[8];
};

#define		BADDATA		-2
