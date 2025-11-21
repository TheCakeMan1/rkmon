#include "mem_update.h"

void get_memory_info(long *total_kb, long *free_kb)
{
    *total_kb = totalram_pages() * (PAGE_SIZE / 1024);
    *free_kb = global_zone_page_state(NR_FREE_PAGES) * (PAGE_SIZE / 1024);
}