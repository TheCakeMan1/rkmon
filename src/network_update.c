#include <linux/kernel.h>
#include <linux/module.h>
#include "network_update.h"

int get_netdev_list(struct netdev_list *out)
{
    struct net_device *dev;
    int idx = 0;

    memset(out, 0, sizeof(*out));

    read_lock(&dev_base_lock);

    for_each_netdev(&init_net, dev)
    {

        if (idx >= MAX_NETDEV)
            break;

        strncpy(out->names[idx], dev->name, MAX_IFNAME - 1);
        idx++;
    }

    read_unlock(&dev_base_lock);

    out->count = idx;
    return idx;
}

struct net_device *get_netdev(const char *name)
{
    return dev_get_by_name(&init_net, name);
}

void update_rate(struct net_device *dev, struct net_rate *r)
{
    struct rtnl_link_stats64 st;

    dev_get_stats(dev, &st);

    r->rx_kbs = (st.rx_bytes - r->rx_prev) / 1024;
    r->tx_kbs = (st.tx_bytes - r->tx_prev) / 1024;

    r->rx_prev = st.rx_bytes;
    r->tx_prev = st.tx_bytes;
}

void get_mac(struct net_device *dev, unsigned char *out)
{
    memcpy(out, dev->dev_addr, ETH_ALEN);
}

void get_net_stats(struct net_device *dev, struct rtnl_link_stats64 *st)
{
    dev_get_stats(dev, st);
}

bool get_link_status(struct net_device *dev)
{
    return netif_carrier_ok(dev);
}

__be32 get_ipv4(struct net_device *dev)
{
    struct in_device *in_dev = dev->ip_ptr;
    if (!in_dev || !in_dev->ifa_list)
        return 0;

    return in_dev->ifa_list->ifa_address;
}

struct in6_addr *get_ipv6(struct net_device *dev)
{
    struct inet6_dev *idev = dev->ip6_ptr;

    if (!idev || list_empty(&idev->addr_list))
        return NULL;

    return &list_first_entry(&idev->addr_list,
                             struct inet6_ifaddr,
                             if_list)
                ->addr;
}

void get_netinfo(const char *ifname,
                 struct netinfo *ni,
                 struct net_rate *rate)
{
    struct net_device *dev;
    struct rtnl_link_stats64 st;

    memset(ni, 0, sizeof(*ni));

    dev = dev_get_by_name(&init_net, ifname);
    if (!dev)
        return;

    strncpy(ni->ifname, ifname, sizeof(ni->ifname));

    // MAC
    get_mac(dev, ni->mac);

    // link
    ni->link_up = get_link_status(dev);

    // stats
    get_net_stats(dev, &st);

    ni->rx_packets = st.rx_packets;
    ni->tx_packets = st.tx_packets;

    ni->rx_errors = st.rx_errors;
    ni->tx_errors = st.tx_errors;

    // speed
    update_rate(dev, rate);

    ni->rx_kbs = rate->rx_kbs;
    ni->tx_kbs = rate->tx_kbs;

    // IPv4
    ni->ipv4 = get_ipv4(dev);

    // IPv6
    {
        struct in6_addr *ipv6 = get_ipv6(dev);
        if (ipv6)
        {
            ni->ipv6 = *ipv6;
            ni->has_ipv6 = true;
        }
    }

    dev_put(dev);
}