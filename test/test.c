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
        // printf("FREQ:\n");
        // for (int j = 0; j < req2.freq_table_count[i]; j++)
        // {
        //     printf("%d ", req2.freq_table[i][j]);
        // }
        // printf("\n");
    }

    printf("AVG: %d%%\n", req2.avg_load);
    printf("MAX: %d%%\n", req2.max_load);

    struct rkmon_cpufreq_request req3;
    req3.cpu = 6;
    ioctl(fd, RKMON_GET_CPUFREQ, &req3);

    printf("CPU %d: cur=%lld min=%lld max=%lld \n",
           req3.cpu, req3.cur, req3.min, req3.max);
    for (int i = 0; i < req3.table_count; i++)
    {
        printf("%d\n", req3.table[i]);
    }

    struct rkmon_net_list req_net_list;
    ioctl(fd, RKMON_GET_NETLIST, &req_net_list);
    for (int i = 0; i < req_net_list.count; i++)
    {
        printf("%s ", req_net_list.names[i]);
    }

    printf("\n");

    struct rkmon_mem_ram mem_ram;
    ioctl(fd, RKMON_GET_MEM_RAM, &mem_ram);
    printf("Avalid: %d\nBuffers: %d\nFree: %d\nFreeSwap: %d\nTotal: %d\nTotalSwap: %d\n",
           mem_ram.avail,
           mem_ram.buffers,
           mem_ram.free,
           mem_ram.freeswap,
           mem_ram.total,
           mem_ram.totalswap);

    // struct rkmon_set_cpufreq_request req4;

    // req4.cpu = 6;
    // req4.freq = 408000;

    // int ret = ioctl(fd, RKMON_SET_CPUFREQ, &req4);
    // printf("%d\n", ret);

    // req3.cpu = 6;
    // ioctl(fd, RKMON_GET_CPUFREQ, &req3);

    // printf("CPU %d: cur=%lld min=%lld max=%lld \n",
    //        req3.cpu, req3.cur, req3.min, req3.max);
    // for (int i = 0; i < req3.table_count; i++)
    // {
    //     printf("%d\n", req3.table[i]);
    // }
    close(fd);
    return 0;
}