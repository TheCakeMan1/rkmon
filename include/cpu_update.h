#ifndef CPU_UPDATE_H
#define CPU_UPDATE_H
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/timer.h>
#include <linux/kernel_stat.h>
#include <linux/sched.h>
#include <linux/tick.h>
#include <linux/cpufreq.h>

#include "rkmon_uapi.h"

#define MAX_CPUS 16
#define MAX_FREQS 32

extern u64 prev_busy[MAX_CPUS];
extern u64 prev_total[MAX_CPUS];
extern int cpu_loads[MAX_CPUS];
extern u64 cpu_freq[MAX_CPUS];
extern u64 cpu_freq_table[MAX_CPUS][MAX_FREQS];
extern u64 cpu_freq_count[MAX_CPUS];
extern struct cpufreq_policy *policy_cache[MAX_CPUS];

void update_cpu_load_all(void);
int set_cpu_freq_limits(struct rkmon_set_cpufreq_request *req);
int update_ioctl_struct_update_cpu_load(struct rkmon_cpufreq_request *req);

#endif