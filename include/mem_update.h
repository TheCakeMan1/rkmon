#ifndef MEM_UPDATE_H
#define MEM_UPDATE_H
#include <linux/mm.h>
#include <linux/vmstat.h>

#include "rkmon_uapi.h"

void get_memory_info(struct rkmon_mem_ram *mem_ram);

#endif