#ifndef NETWORK_UPDATE_H
#define NETWORK_UPDATE_H

#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/rtnetlink.h>
#include <linux/if_ether.h>
#include <linux/ipv6.h>
#include <net/addrconf.h>

#define MAX_NETDEV 32
#define MAX_IFNAME 16

struct net_rate
{
    u64 rx_prev;
    u64 tx_prev;
    u64 rx_kbs;
    u64 tx_kbs;
};

struct netinfo
{
    char ifname[16];

    unsigned char mac[ETH_ALEN];
    bool link_up;

    u64 rx_packets;
    u64 tx_packets;
    u64 rx_errors;
    u64 tx_errors;

    u64 rx_kbs;
    u64 tx_kbs;

    __be32 ipv4;
    struct in6_addr ipv6;
    bool has_ipv6;
};

struct netdev_list
{
    int count;
    char names[MAX_NETDEV][MAX_IFNAME];
};

// API
struct net_device *get_netdev(const char *name);

void update_rate(struct net_device *dev, struct net_rate *r);

void get_mac(struct net_device *dev, unsigned char *out);

void get_net_stats(struct net_device *dev, struct rtnl_link_stats64 *st);

bool get_link_status(struct net_device *dev);

__be32 get_ipv4(struct net_device *dev);

struct in6_addr *get_ipv6(struct net_device *dev);

void get_netinfo(const char *ifname,
                 struct netinfo *ni,
                 struct net_rate *rate);

int get_netdev_list(struct netdev_list *out);

#endif
