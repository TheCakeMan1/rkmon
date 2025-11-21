#ifndef RKMON_UAPI_H
#define RKMON_UAPI_H

#include <linux/ioctl.h>

/* ioctl numbers */
#define RKMON_SET_UPDATE_RATE _IOW('r', 1, int)
#define RKMON_GET_NETINFO _IOWR('r', 2, struct rkmon_net_request)

/* This struct must be 100% identical in kernel and userspace */
struct netinfo_uapi
{
    char ifname[16];

    unsigned char mac[6];
    int link_up;

    unsigned long long rx_packets;
    unsigned long long tx_packets;
    unsigned long long rx_errors;
    unsigned long long tx_errors;

    unsigned long long rx_kbs;
    unsigned long long tx_kbs;

    unsigned int ipv4;
    unsigned char ipv6[16];
    int has_ipv6;
};

struct rkmon_net_request
{
    char ifname[16];
    struct netinfo_uapi info;
};

#endif
