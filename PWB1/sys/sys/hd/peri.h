struct vp {
	int	vp_state;
	char	*vp_buf;
	char	*vp_count;
	int	vp_offset;
};
extern struct vp vp_vp[];

struct du {
	char	*du_buf;
	char	*du_bufp;
	int	du_nxmit;
	char	du_state;
	char	du_dummy;
	int	du_proc;
	char	*du_bufb;
	char	*du_bufe;
	int	du_nleft;
};
extern struct du du_du[];

#ifdef DQS11
#define NDQ 4
#define NBF 2

struct dqsbuf	{
	int bufl,bufc;
	char *bufb;
	struct dqsbuf *bufn;
	struct buf *bufa;
};

struct dqsdat	{
	int  state;
	char qcase,open;
	char *addr;
	int  dly;
	int  ttd,ack0,wack,ackx;
	char eot,enq,nak,etb,etx;
	char slp,time,try;
	int  cc,resp,uoff;
	struct dqsbuf *x,*q,*u;
	struct dqsbuf bf[NBF];
};

struct dqsdat dqsx[];
#endif
