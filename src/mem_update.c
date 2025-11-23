#include "mem_update.h"

void get_memory_info(struct rkmon_mem_ram *mem_ram)
{
    struct sysinfo si;

    si_meminfo(&si);

    mem_ram->total = (si.totalram * si.mem_unit) / 1024;
    mem_ram->free = (si.freeram * si.mem_unit) / 1024;
    mem_ram->buffers = (si.bufferram * si.mem_unit) / 1024;

    mem_ram->avail = (si_mem_available() * si.mem_unit) / 1024;

    mem_ram->totalswap = (si.totalswap * si.mem_unit) / 1024;
    mem_ram->freeswap = (si.freeswap * si.mem_unit) / 1024;
    // *total_kb = totalram_pages() * (PAGE_SIZE / 1024);
    // *free_kb = global_zone_page_state(NR_FREE_PAGES) * (PAGE_SIZE / 1024);
}