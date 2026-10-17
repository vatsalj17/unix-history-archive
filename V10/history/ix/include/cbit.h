struct cbit {
	/* these 3 are universal */
	char *name;		/* official (wordy) name of category */
	int floor;		/* default value (only bottom bit used) */
	char *owner;		/* public name of issuing authority */

	/* these 4 are local to our machine */
	char *nickname;		/* our version of name */
	int bitslot;		/* where we store it */
	char *exerciser;	/* who we are */
	char *certificate;	/* signed by owner, gives us right to hold */
};

extern struct cbit *cbitread();
extern struct cbit *cbitlookup();
extern struct cbit *cbitparse();
extern char *cbitcert();

#define FLOORMASK	1

struct mapping {
	char *partner;		/* remote machine */
	int shuffle[8*LABSIZ];	/* shuffle[j] is where WE store her bit j */
	struct label lambda;	/* her ceiling in her terms */
	struct label lstar;	/* her ceiling in my terms */
	struct label dirty;	/* bits in our floor she does not know about */
};

extern transin();
extern transout();
extern struct mapping * buildmap();
