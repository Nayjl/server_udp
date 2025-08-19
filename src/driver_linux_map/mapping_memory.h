#ifndef MAPPING_MEMORY_H
#define MAPPING_MEMORY_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>


#define MEM_ALLOC _IOW('m', 1, size_t)
#define MEM_FREE  _IO('m', 2)
#define MEM_GET_PHYS_ADDR _IOR('m', 3, unsigned long)


volatile void* maping(int fd, long base_phy_addr, unsigned long number_byte);

void DMAmemory_alloc(int fd, size_t *number_byte);

void DMAmemory_free(int fd);

void DMAmemory_get_phys_addr(int fd, unsigned long* phys_addr);

// volatile void* DMAmemory_maping(int fd, unsigned long base_phy_addr, unsigned long number_byte);

#endif