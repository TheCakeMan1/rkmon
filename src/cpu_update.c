#include "cpu_update.h"

u64 prev_busy[MAX_CPUS];
u64 prev_total[MAX_CPUS];
u64 cpu_freq[MAX_CPUS];
int cpu_loads[MAX_CPUS];

void update_cpu_load_all(void)
{
    int cpu;
    unsigned int freq = 0;

    for_each_online_cpu(cpu)
    {
        if (cpu >= MAX_CPUS)
            break;

        struct kernel_cpustat kcs = kcpustat_cpu(cpu);
        struct cpufreq_policy *policy = cpufreq_cpu_get(cpu);
        if (!policy)
            cpu_freq[cpu] = 0;
        cpu_freq[cpu] = policy->cur;

        u64 user = kcs.cpustat[CPUTIME_USER];
        u64 system = kcs.cpustat[CPUTIME_SYSTEM];
        u64 irq = kcs.cpustat[CPUTIME_IRQ];
        u64 softirq = kcs.cpustat[CPUTIME_SOFTIRQ];
        u64 idle = kcs.cpustat[CPUTIME_IDLE];
        u64 iowait = kcs.cpustat[CPUTIME_IOWAIT];

        u64 busy = user + system + irq + softirq;
        u64 total = busy + idle + iowait;

        u64 delta_busy = busy - prev_busy[cpu];
        u64 delta_total = total - prev_total[cpu];

        if (delta_total > 0)
            cpu_loads[cpu] = (delta_busy * 100) / delta_total;
        else
            cpu_loads[cpu] = 0;

        prev_busy[cpu] = busy;
        prev_total[cpu] = total;
    }
}
