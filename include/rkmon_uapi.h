#ifndef RKMON_UAPI_H
#define RKMON_UAPI_H

#include <linux/ioctl.h>

#define RKMON_MAX_CPUS 16
#define RKMON_MAX_FREQS 32
#define RKMON_MAX_NETDEV 32
#define RKMON_MAX_IFNAME 16
/* ioctl numbers */
#define RKMON_SET_UPDATE_RATE _IOW('r', 1, int)
#define RKMON_GET_NETINFO _IOWR('r', 2, struct rkmon_net_request)
#define RKMON_GET_CPUINFO _IOR('r', 3, struct rkmon_cpu_request)
#define RKMON_GET_CPUFREQ _IOR('r', 4, struct rkmon_cpufreq_request)
#define RKMON_GET_NETLIST _IOW('r', 5, struct rkmon_net_list)
#define RKMON_GET_MEM_RAM _IOW('r', 5, struct rkmon_mem_ram)
// #define RKMON_SET_CPUFREQ _IOW('r', 5, struct rkmon_set_cpufreq_request)

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

struct rkmon_cpu_request
{
    int cpu_count;

    /* загрузка каждого CPU (в процентах) */
    int cpu_load[RKMON_MAX_CPUS];

    /* частота каждого CPU (в kHz) */
    unsigned long long cpu_freq[RKMON_MAX_CPUS];

    /* температура каждого CPU (в миллиградусах или градусах, как удобно) */
    int cpu_temp[RKMON_MAX_CPUS];

    /* общая загрузка */
    int avg_load;

    /* максимальная загрузка */
    int max_load;
};

struct rkmon_cpufreq_request
{
    int cpu;
    unsigned long long cur;
    unsigned long long min;
    unsigned long long max;
    unsigned int table[RKMON_MAX_FREQS];
    int table_count;
};

struct rkmon_set_cpufreq_request
{
    int cpu;
    unsigned long long freq;
};

struct rkmon_net_list
{
    int count;
    char names[RKMON_MAX_NETDEV][RKMON_MAX_IFNAME];
};

struct rkmon_mem_ram
{
    int total;
    int free;
    int avail;
    int buffers;
    int totalswap;
    int freeswap;
};

#endif
