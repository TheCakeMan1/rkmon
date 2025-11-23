#include "cpu_update.h"

u64 prev_busy[MAX_CPUS];
u64 prev_total[MAX_CPUS];
u64 cpu_freq[MAX_CPUS];
int cpu_loads[MAX_CPUS];
struct cpufreq_policy *policy_cache[MAX_CPUS];

int set_cpu_freq_limits(struct rkmon_set_cpufreq_request *req)
{
    struct cpufreq_policy *policy;
    const struct cpufreq_frequency_table *t;
    bool found = false;
    int i = 0;
    int ret;

    if (req->cpu >= nr_cpu_ids || !cpu_online(req->cpu))
        return -EINVAL;

    if (!policy_cache[req->cpu])
        return -EINVAL;

    t = policy_cache[req->cpu]->freq_table;
    if (!t)
        return -EINVAL;

    while (t[i].frequency != CPUFREQ_TABLE_END)
    {
        pr_info("freq[%d] = %u\n", i, t[i].frequency);
        if (t[i].frequency == (u32)req->freq)
        {
            found = true;
            break;
        }
        i++;
    }

    if (!found)
        return -EINVAL;

    policy = cpufreq_cpu_get(req->cpu);
    if (!policy)
        return -EINVAL;

    ret = __cpufreq_driver_target(policy, (u32)req->freq,
                                  CPUFREQ_RELATION_L);
    if (ret)
    {
        pr_info("driver_target failed: %d\n", ret);
        cpufreq_cpu_put(policy);
        return ret;
    }

    policy->min = (u32)req->freq;
    policy->max = (u32)req->freq;

    cpufreq_update_policy(req->cpu);

    cpufreq_cpu_put(policy);

    return 0;
}

int update_ioctl_struct_update_cpu_load(struct rkmon_cpufreq_request *req)
{
    if (req->cpu < 0 || req->cpu > MAX_CPUS)
        return 1;

    if (!policy_cache[req->cpu])
        return 1;

    req->cur = policy_cache[req->cpu]->cur;
    req->min = policy_cache[req->cpu]->min;
    req->max = policy_cache[req->cpu]->max;

    if (policy_cache[req->cpu]->freq_table)
    {
        const struct cpufreq_frequency_table *t = policy_cache[req->cpu]->freq_table;
        __u32 i = 0;

        while (t[i].frequency != CPUFREQ_TABLE_END &&
               i < RKMON_MAX_FREQS)
        {
            req->table[i] = t[i].frequency;
            i++;
        }
        req->table_count = i;
    }

    return 0;
}

void update_cpu_load_all(void)
{
    int cpu;

    for_each_online_cpu(cpu)
    {
        struct kernel_cpustat kcs = kcpustat_cpu(cpu);

        __u64 user, system, irq, softirq, idle, iowait, busy, total, delta_busy, delta_total;

        if (cpu >= MAX_CPUS)
            break;

        if (!policy_cache[cpu])
        {
            cpu_freq[cpu] = 0;
        }
        else
        {
            cpu_freq[cpu] = policy_cache[cpu]->cur;
        }

        user = kcs.cpustat[CPUTIME_USER];
        system = kcs.cpustat[CPUTIME_SYSTEM];
        irq = kcs.cpustat[CPUTIME_IRQ];
        softirq = kcs.cpustat[CPUTIME_SOFTIRQ];
        idle = kcs.cpustat[CPUTIME_IDLE];
        iowait = kcs.cpustat[CPUTIME_IOWAIT];

        busy = user + system + irq + softirq;
        total = busy + idle + iowait;

        delta_busy = busy - prev_busy[cpu];
        delta_total = total - prev_total[cpu];

        if (delta_total > 0)
            cpu_loads[cpu] = (delta_busy * 100) / delta_total;
        else
            cpu_loads[cpu] = 0;

        prev_busy[cpu] = busy;
        prev_total[cpu] = total;
    }
}
