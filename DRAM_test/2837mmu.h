// --------------------------------------------------------------------
//
//   Title     :  2837mmu.h
//             :
//   Library   :
//             :
//   Developers:  Kevin Hsu
//             :
//   Purpose   :  RaspberryPi 3 "B" BCM2837 peripheral address mapping
//             :
//   Limitation:
//             :
//   Note      :
//             :
// --------------------------------------------------------------------
//   modification history :
// --------------------------------------------------------------------
//   Version| mod. date: |
//   V1.0   | 03-Dec-2017 | Programming Start

#ifndef __2837mmu_H__
#define __2837mmu_H__

#define PAGE_SIZE    (4*1024)
#define BLOCK_SIZE  (4*1024)

struct bcm2837_peripheral{
    unsigned long phi_addr;
    int mem_fd;
    char *mem;
    void *map;
    volatile unsigned int *addr;
};

void setup_io(struct bcm2837_peripheral *ptr);
void release_io(struct bcm2837_peripheral *ptr);

#endif  //__2837_H___
