#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include "../include/rkmon_uapi.h"

int main(int argc, char **argv)
{
    int fd;
    struct rkmon_net_request req;

    if (argc < 2)
    {
        printf("Usage: %s <interface>\n", argv[0]);
        return 1;
    }

    fd = open("/dev/rkmon", O_RDWR);
    if (fd < 0)
    {
        perror("open /dev/rkmon");
        return 1;
    }

    memset(&req, 0, sizeof(req));
    strncpy((char *)req.ifname, argv[1], sizeof(req.ifname) - 1);

    if (ioctl(fd, RKMON_GET_NETINFO, &req) < 0)
    {
        perror("ioctl RKMON_GET_NETINFO");
        close(fd);
        return 1;
    }

    printf("Interface: %s\n", req.info.ifname);

    printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           req.info.mac[0], req.info.mac[1], req.info.mac[2],
           req.info.mac[3], req.info.mac[4], req.info.mac[5]);

    struct in_addr ip4;
    ip4.s_addr = req.info.ipv4;
    printf("IPv4: %s\n", inet_ntoa(ip4));

    if (req.info.has_ipv6)
    {
        char ipv6_str[64];
        inet_ntop(AF_INET6, req.info.ipv6, ipv6_str, sizeof(ipv6_str));
        printf("IPv6: %s\n", ipv6_str);
    }
    else
    {
        printf("IPv6: -\n");
    }

    printf("Link: %s\n", req.info.link_up ? "UP" : "DOWN");

    printf("RX: %llu KB/s  packets=%llu  errors=%llu\n",
           (unsigned long long)req.info.rx_kbs,
           (unsigned long long)req.info.rx_packets,
           (unsigned long long)req.info.rx_errors);

    printf("TX: %llu KB/s  packets=%llu  errors=%llu\n",
           (unsigned long long)req.info.tx_kbs,
           (unsigned long long)req.info.tx_packets,
           (unsigned long long)req.info.tx_errors);

    struct rkmon_cpu_request req2;

    ioctl(fd, RKMON_GET_CPUINFO, &req2);

    printf("CPU count: %d\n", req2.cpu_count);

    for (int i = 0; i < req2.cpu_count; i++)
    {
        printf("CPU %d: load=%d%% freq=%llu kHz temp=%dC\n",
               i, req2.cpu_load[i], req2.cpu_freq[i], req2.cpu_temp[i]);
    }

    printf("AVG: %d%%\n", req2.avg_load);
    printf("MAX: %d%%\n", req2.max_load);

    close(fd);
    return 0;
}