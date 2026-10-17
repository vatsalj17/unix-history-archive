#ifndef __NETDB_H__
#define __NETDB_H__

#include <common/ccompat.h>

struct hostent {
	char	      *	h_name;
	char	     **	h_aliases;
	int		h_addrtype;
	int		h_length;
	char	     **	h_addr_list;
#define	h_addr		h_addr_list [0]
};

struct netent {
	char	      *	n_name;
	char	     **	n_aliases;
	int		n_addrtype;
	unsigned long	n_net;
};

struct servent {
	char	      *	s_name;
	char	     **	s_aliases;
	int		s_port;
	char	      *	s_proto;
};

struct	protoent {
	char	      *	p_name;
	char	     **	p_aliases;
	int		p_proto;
};


__EXTERN_C_BEGIN__

struct hostent * gethostent	__PROTO ((void));
struct hostent * gethostbyaddr	__PROTO ((__CONST__ char * _addrp, int _len,
					  int _type));
struct hostent * gethostbyname	__PROTO ((__CONST__ char * _name));
void		sethostent	__PROTO ((int _stayopen));
void		endhostent	__PROTO ((void));

struct netent *	getnetent	__PROTO ((void));
struct netent *	getnetbyname	__PROTO ((__CONST__ char * _name));
struct netent *	getnetbyaddr	__PROTO ((long _net, int _type));
void		setnetent	__PROTO ((int _stayopen));
void		endnetent	__PROTO ((void));

struct protoent * getprotoent	__PROTO ((void));
struct protoent * getprotobyname __PROTO ((__CONST__ char * _name));
struct protoent * getprotobynumber __PROTO ((int _proto));
void		setprotoent	__PROTO ((int _stayopen));
void		endprotoent	__PROTO ((void));

struct servent * getservent	__PROTO ((void));
struct servent * getservbyname	__PROTO ((__CONST__ char * _name,
					  __CONST__ char * _proto));
struct servent * getservbyport	__PROTO ((int _port, __CONST__ char * _proto));
void		setservent	__PROTO ((int _stayopen));
void		endservent	__PROTO ((void));

__EXTERN_C_END__

extern  int 		h_errno;

#define	HOST_NOT_FOUND		1
#define	TRY_AGAIN		2
#define	NO_RECOVERY		3
#define	NO_DATA			4
#define	NO_ADDRESS		NO_DATA

#define _PATH_NETWORKS		"/etc/networks"
#define _PATH_PROTOCOLS		"/etc/protocols"
#define _PATH_SERVICES		"/etc/services"
#define _PATH_HOSTS		"/etc/hosts"

#endif	/* ! defined (__NETDB_H__) */
