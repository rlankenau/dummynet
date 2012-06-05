/*
 * Copyright (c) 2009 Luigi Rizzo, Marta Carbone, Universita` di Pisa
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * $Id: glue.h 5518 2010-02-26 12:51:46Z luigi $
 *
 * glue code to adapt the FreeBSD version to linux and windows,
 * userland and kernel.
 * This is included before any other headers, so we do not have
 * a chance to override any #define that should appear in other
 * headers.
 * First handle headers for userland and kernel. Then common code
 * (including headers that require a specific order of inclusion),
 * then the user- and kernel- specific parts.
 */
 
#ifndef _GLUE_H
#define	_GLUE_H

/*
 * common definitions to allow portability
 */
#ifndef __FBSDID
#define __FBSDID(x)
#endif  /* FBSDID */

#ifndef KERNEL_MODULE	/* Userland headers */

#if defined(__CYGWIN32__) && !defined(_WIN32)                                   
#define _WIN32                                                                  
#endif                                                                          

#if defined(TCC) && defined(_WIN32)
#include <tcc_glue.h>
#endif /* TCC */

#include <stdint.h>	/* linux needs it in addition to sys/types.h */
#include <sys/types.h>	/* for size_t */
#include <sys/ioctl.h>
#include <time.h>
#include <errno.h>
#include <netinet/ether.h>

#else /* KERNEL_MODULE, kernel headers */

#ifdef __linux__
#include <linux/version.h>

#define ifnet		net_device	/* remap */
#define	_KERNEL		# make kernel structure visible
#define	KLD_MODULE	# add the module glue
#define	INET		# want inet support

#include <linux/stddef.h>	/* linux kernel */
#include <linux/types.h>	/* linux kernel */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17)	// or 2.4.x
#include <linux/linkage.h>	/* linux/msg.h require this */
#include <linux/netdevice.h>	/* just MAX_ADDR_LEN 8 on 2.4 32 on 2.6, also brings in byteorder */
#endif

/* on 2.6.22, msg.h requires spinlock_types.h */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0) && \
	LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
#include <linux/spinlock_types.h>
#endif
/* XXX m_type define conflict with include/sys/mbuf.h,
 * so early include msg.h (to be solved)
*/
#include <linux/msg.h>	

#include <linux/list.h>
#include <linux/in.h>		/* struct in_addr */
#include <linux/in6.h>		/* struct in6_addr */
#include <linux/icmp.h>
/*
 * LIST_HEAD in queue.h conflict with linux/list.h
 * some previous linux include need list.h definition
 */
#undef LIST_HEAD

#define	IF_NAMESIZE	(16)
typedef	uint32_t	in_addr_t;

#define printf(fmt, arg...) printk(KERN_ERR fmt, ##arg)
#endif	/* __linux__ */

#endif /* KERNEL_MODULE end of kernel headers */


/*
 * Part 2: common userland and kernel definitions
 */

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN (6+0)       /* length of an Ethernet address */
#endif

#define ICMP6_DST_UNREACH_NOROUTE       0       /* no route to destination */
#define ICMP6_DST_UNREACH_ADMIN         1       /* administratively prohibited */
#define ICMP6_DST_UNREACH_ADDR          3       /* address unreachable */
#define ICMP6_DST_UNREACH_NOPORT        4       /* port unreachable */

/*
 * linux: sysctl are mapped into /sys/module/ipfw_mod parameters
 * windows: they are emulated via get/setsockopt
 */
#define CTLFLAG_RD		1
#define CTLFLAG_RDTUN	1
#define CTLFLAG_RW		2
#define CTLFLAG_SECURE3	0 // unsupported
#define CTLFLAG_VNET    0	/* unsupported */

/* if needed, queue.h must be included here after list.h */

/*
 * struct thread is used in linux and windows kernel.
 * In windows, we need to emulate the sockopt interface
 * so also the userland needs to have the struct sockopt defined.
 */
struct thread {
        void *sopt_td;
        void *td_ucred;
};

enum sopt_dir { SOPT_GET, SOPT_SET };

struct  sockopt {
        enum    sopt_dir sopt_dir; /* is this a get or a set? */
        int     sopt_level;     /* second arg of [gs]etsockopt */
        int     sopt_name;      /* third arg of [gs]etsockopt */
        void   *sopt_val;       /* fourth arg of [gs]etsockopt */
        size_t  sopt_valsize;   /* (almost) fifth arg of [gs]etsockopt */
        struct  thread *sopt_td; /* calling thread or null if kernel */
};


#define INET_ADDRSTRLEN		(16)	/* missing in netinet/in.h */

/*
 * List of values used for set/getsockopt options.
 * The base value on FreeBSD is defined as a macro,
 * if not available we will use our own enum.
 * The TABLE_BASE value is used in the kernel.
 */
#ifndef IP_FW_TABLE_ADD
#define _IPFW_SOCKOPT_BASE	100	/* 40 on freebsd */
enum ipfw_msg_type {
	IP_FW_TABLE_ADD		= _IPFW_SOCKOPT_BASE,
	IP_FW_TABLE_DEL,
	IP_FW_TABLE_FLUSH,
	IP_FW_TABLE_GETSIZE,
	IP_FW_TABLE_LIST,
	IP_FW_DYN_GET,		/* new addition */

	/* IP_FW3 and IP_DUMMYNET3 are the new API */
	IP_FW3			= _IPFW_SOCKOPT_BASE + 8,
	IP_DUMMYNET3,

	IP_FW_ADD		= _IPFW_SOCKOPT_BASE + 10,
	IP_FW_DEL,
	IP_FW_FLUSH,
	IP_FW_ZERO,
	IP_FW_GET,
	IP_FW_RESETLOG,

	IP_FW_NAT_CFG,
	IP_FW_NAT_DEL,
	IP_FW_NAT_GET_CONFIG,
	IP_FW_NAT_GET_LOG,

	IP_DUMMYNET_CONFIGURE,
	IP_DUMMYNET_DEL	,
	IP_DUMMYNET_FLUSH,
	/* 63 is missing */
	IP_DUMMYNET_GET		= _IPFW_SOCKOPT_BASE + 24,
	_IPFW_SOCKOPT_END
};
#endif /* IP_FW_TABLE_ADD */

/*
 * Part 3: userland stuff
 */

#ifndef KERNEL_MODULE

/*
 * internal names in struct in6_addr (netinet/in6.h) differ,
 * so we remap the FreeBSD names to the platform-specific ones.
 */
#ifndef _WIN32
#define __u6_addr	in6_u
#define __u6_addr32	u6_addr32
#define in6_u __in6_u	/* missing type for ipv6 (linux 2.6.28) */
#else	/* _WIN32 uses different naming */
#define __u6_addr	__u6
#define __u6_addr32	__s6_addr32
#endif	/* _WIN32 */

/* missing in linux netinet/ip.h */
#define IPTOS_ECN_ECT0	0x02    /* ECN-capable transport (0) */
#define IPTOS_ECN_CE	0x03    /* congestion experienced */

/* defined in freebsd netinet/icmp6.h */
#define ICMP6_MAXTYPE	201

/* on freebsd sys/socket.h pf specific */
#define NET_RT_IFLIST	3               /* survey interface list */

/* on freebsd net/if.h XXX used */
struct if_data {
	/* ... */
        u_long ifi_mtu;	/* maximum transmission unit */
};

/*
 * Message format for use in obtaining information about interfaces
 * from getkerninfo and the routing socket.
 * This is used in nat.c
 */
struct if_msghdr {
        u_short ifm_msglen;     /* to skip over unknown messages */
        u_char  ifm_version;    /* future binary compatibility */
        u_char  ifm_type;       /* message type */
        int     ifm_addrs;      /* like rtm_addrs */
        int     ifm_flags;      /* value of if_flags */
        u_short ifm_index;      /* index for associated ifp */
        struct  if_data ifm_data;/* stats and other ifdata */
};

/*
 * Message format for use in obtaining information about interface
 * addresses from getkerninfo and the routing socket
 */
struct ifa_msghdr {
        u_short ifam_msglen;    /* to skip over unknown messages */
        u_char  ifam_version;   /* future binary compatibility */
        u_char  ifam_type;      /* message type */
        int     ifam_addrs;     /* like rtm_addrs */
        int     ifam_flags;     /* value of ifa_flags */
        u_short ifam_index;     /* index for associated ifp */
        int     ifam_metric;    /* value of ifa_metric */
};

#ifndef NO_RTM	/* conflicting with netlink */
/* missing in net/route.h */
#define RTM_VERSION     5       /* Up the ante and ignore older versions */
#define RTM_IFINFO      0xe     /* iface going up/down etc. */
#define RTM_NEWADDR     0xc     /* address being added to iface */
#define RTA_IFA         0x20    /* interface addr sockaddr present */
#endif	/* NO_RTM */

/* SA_SIZE is used in the userland nat.c modified */
#define SA_SIZE(sa)                                             \
    (  (!(sa) ) ?      \
        sizeof(long)            :                               \
        1 + ( (sizeof(struct sockaddr) - 1) | (sizeof(long) - 1) ) )

/* sys/time.h */
/*
 * Getkerninfo clock information structure
 */
struct clockinfo {
        int     hz;             /* clock frequency */
        int     tick;           /* micro-seconds per hz tick */
        int     spare;
        int     stathz;         /* statistics clock frequency */
        int     profhz;         /* profiling clock frequency */
};

/*
 * linux does not have a reentrant version of qsort,
 * so we the FreeBSD stdlib version.
 */
void qsort_r(void *a, size_t n, size_t es, void *thunk,
	int cmp_t(void *, const void *, const void *));

/* prototypes from libutil */
/* humanize_number(3) */
#define HN_DECIMAL              0x01
#define HN_NOSPACE              0x02
#define HN_B                    0x04
#define HN_DIVISOR_1000         0x08

#define HN_GETSCALE             0x10
#define HN_AUTOSCALE            0x20

int     humanize_number(char *_buf, size_t _len, int64_t _number,
            const char *_suffix, int _scale, int _flags);
int     expand_number(const char *_buf, int64_t *_num);

#define setprogname(x)	/* not present in linux */

extern int optreset;	/* not present in linux */

size_t strlcpy(char * dst, const char * src, size_t siz);
long long int strtonum(const char *nptr, long long minval,
	long long maxval, const char **errstr);
 
int sysctlbyname(const char *name, void *oldp, size_t *oldlenp,
	void *newp, size_t newlen);
 
/* no sin_len in sockaddr, we only remap in userland */
#define	sin_len	sin_zero[0]

#else /* KERNEL_MODULE */

/*
 * Part 4: kernel stuff
 */

/* linux and windows kernel do not have bcopy ? */
#define bcopy(_s, _d, _l)	memcpy(_d, _s, _l)
/* definitions useful for the kernel side */
struct route_in6 {
	int dummy;
};

#ifdef __linux__

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17)	// or 2.4.x
#include <linux/in6.h>
#endif

/* skb_dst() was introduced from linux 2.6.31 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31)	// or 2.4.x
#define skb_dst(_dummy) skb->dst
#endif

#endif /* __linux__ */

#endif	/* KERNEL_MODULE */

/*
 * Part 5: windows specific stuff
 */

#ifdef _WIN32
#ifndef KERNEL_MODULE
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3
#define FILE_ANY_ACCESS                 0
#define FILE_READ_DATA            ( 0x0001 )    // file & pipe
#define FILE_WRITE_DATA           ( 0x0002 )    // file & pipe
#endif /* !KERNEL_MODULE */

#define FILE_DEVICE_IPFW		0x00654324
#define IP_FW_BASE_CTL			0x840
#define IP_FW_SETSOCKOPT \
	CTL_CODE(FILE_DEVICE_IPFW, IP_FW_BASE_CTL + 1, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IP_FW_GETSOCKOPT \
	CTL_CODE(FILE_DEVICE_IPFW, IP_FW_BASE_CTL + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*********************************
* missing declarations in altq.c *
**********************************/

#define _IOWR(x,y,t) _IOW(x,y,t)

/**********************************
* missing declarations in ipfw2.c *
***********************************/

#define	ICMP_UNREACH_NET		0	/* bad net */
#define	ICMP_UNREACH_HOST		1	/* bad host */
#define	ICMP_UNREACH_PROTOCOL		2	/* bad protocol */
#define	ICMP_UNREACH_PORT		3	/* bad port */
#define	ICMP_UNREACH_NEEDFRAG		4	/* IP_DF caused drop */
#define	ICMP_UNREACH_SRCFAIL		5	/* src route failed */
#define	ICMP_UNREACH_NET_UNKNOWN	6	/* unknown net */
#define	ICMP_UNREACH_HOST_UNKNOWN	7	/* unknown host */
#define	ICMP_UNREACH_ISOLATED		8	/* src host isolated */
#define	ICMP_UNREACH_NET_PROHIB		9	/* prohibited access */
#define	ICMP_UNREACH_HOST_PROHIB	10	/* ditto */
#define	ICMP_UNREACH_TOSNET		11	/* bad tos for net */
#define	ICMP_UNREACH_TOSHOST		12	/* bad tos for host */
#define	ICMP_UNREACH_FILTER_PROHIB	13	/* admin prohib */
#define	ICMP_UNREACH_HOST_PRECEDENCE	14	/* host prec vio. */
#define	ICMP_UNREACH_PRECEDENCE_CUTOFF	15	/* prec cutoff */

#define __unused


struct ether_addr;
struct ether_addr * ether_aton(const char *a);

/*********************************
* missing declarations in ipv6.c *
**********************************/

struct hostent* gethostbyname2(const char *name, int af);


/********************
* windows wrappings *
*********************/

int my_socket(int domain, int ty, int proto);
#define socket(_a, _b, _c)	my_socket(_a, _b, _c)

/*******************
* SYSCTL emulation *
********************/

#define STRINGIFY(x) #x

#ifdef SYSCTL_NODE
#undef SYSCTL_NODE
#endif
#define SYSCTL_NODE(a,b,c,d,e,f)
#define SYSCTL_DECL(a)
#define SYSCTL_VNET_PROC(a,b,c,d,e,f,g,h,i)

#define GST_HARD_LIMIT 100

/* flag is set with the last 2 bits for access, as defined in glue.h
 * and the rest for type
 */
enum {
	SYSCTLTYPE_INT = 0,
	SYSCTLTYPE_UINT,
	SYSCTLTYPE_SHORT,
	SYSCTLTYPE_USHORT,
	SYSCTLTYPE_LONG,
	SYSCTLTYPE_ULONG,
	SYSCTLTYPE_STRING,
};

struct sysctlentry {
	unsigned int blocklen; //total size of the entry
	char* name;
	unsigned int namelen; //strlen(name) + '\0'
	unsigned int flags; //type and access
	unsigned int datalen;
	void* data;
};

struct sysctltable {
	int count; //number of valid tables
	int totalsize; //total size of valid entries of al the valid tables
	void* namebuffer; //a buffer for all chained names
	struct sysctlentry entry[GST_HARD_LIMIT];
};

#ifdef SYSBEGIN
#undef SYSBEGIN
#endif
#define SYSBEGIN(x) void sysctl_addgroup_##x() {
#ifdef SYSEND
#undef SYSEND
#endif
#define SYSEND }

/* XXX remove duplication */
#define SYSCTL_INT(a,b,c,d,e,f,g) 				\
	sysctl_pushback(STRINGIFY(a) "." STRINGIFY(c) + 1,	\
		(d) | (SYSCTLTYPE_INT << 2), sizeof(*e), e)

#define SYSCTL_VNET_INT(a,b,c,d,e,f,g)				\
	sysctl_pushback(STRINGIFY(a) "." STRINGIFY(c) + 1,	\
		(d) | (SYSCTLTYPE_INT << 2), sizeof(*e), e)

#define SYSCTL_UINT(a,b,c,d,e,f,g)				\
	sysctl_pushback(STRINGIFY(a) "." STRINGIFY(c) + 1,	\
		(d) | (SYSCTLTYPE_UINT << 2), sizeof(*e), e)

#define SYSCTL_LONG(a,b,c,d,e,f,g)				\
	sysctl_pushback(STRINGIFY(a) "." STRINGIFY(c) + 1,	\
		(d) | (SYSCTLTYPE_LONG << 2), sizeof(*e), e)

#define SYSCTL_ULONG(a,b,c,d,e,f,g)				\
	sysctl_pushback(STRINGIFY(a) "." STRINGIFY(c) + 1,	\
		(d) | (SYSCTLTYPE_ULONG << 2), sizeof(*e), e)
#define TUNABLE_INT(a,b)

void keinit_GST();
void keexit_GST();
int kesysctl_emu_set(void* p, int l);
int kesysctl_emu_get(struct sockopt* sopt);
void sysctl_pushback(char* name, int flags, int datalen, void* data);
int do_cmd(int optname, void *optval, uintptr_t optlen);
int sysctlbyname(const char *name, void *oldp, size_t *oldlenp, void *newp,
         size_t newlen);

#endif /* _WIN32 */

#endif /* !_GLUE_H */
