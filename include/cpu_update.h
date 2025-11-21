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

#define MAX_CPUS 16

extern u64 prev_busy[MAX_CPUS];
extern u64 prev_total[MAX_CPUS];
extern int cpu_loads[MAX_CPUS];
extern u64 cpu_freq[MAX_CPUS];

void update_cpu_load_all(void);

#endif