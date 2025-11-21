#ifndef MEM_UPDATE_H
#define MEM_UPDATE_H
#include <linux/mm.h>
#include <linux/vmstat.h>

void get_memory_info(long *total_kb, long *free_kb);

#endif