#include "mapping_memory.h"



volatile void* maping(int fd, long base_phy_addr, unsigned long number_byte) {
    volatile void* ptr;
    ptr = (volatile void*)mmap(NULL, number_byte, PROT_READ | PROT_WRITE, MAP_SHARED, fd, base_phy_addr);
    if (ptr == MAP_FAILED) {
        perror("Ошибка отображения памяти");
        close(fd);
        exit(EXIT_FAILURE);
    }
    return ptr;
}


void DMAmemory_alloc(int fd, size_t *number_byte) {
    // Выделение памяти через IOCTL
    if (ioctl(fd, MEM_ALLOC, number_byte) < 0) {
        perror("Ошибка выделения памяти");
        close(fd);
        exit(EXIT_FAILURE);
    }
}

void DMAmemory_free(int fd) {
    // Выделение памяти через IOCTL
    if (ioctl(fd, MEM_FREE) < 0) {
        perror("Ошибка выделения памяти");
        close(fd);
        exit(EXIT_FAILURE);
    }
}


void DMAmemory_get_phys_addr(int fd, unsigned long* phys_addr) {
    // Получение физического адреса выделенной памяти
    if (ioctl(fd, MEM_GET_PHYS_ADDR, phys_addr) < 0) {
        perror("Ошибка получения физического адреса");
        ioctl(fd, MEM_FREE); // Освобождение памяти
        close(fd);
        exit(EXIT_FAILURE);
    }
}


// volatile void* DMAmemory_maping(int fd, unsigned long base_phy_addr, unsigned long number_byte) {
//     volatile void* ptr;
//     ptr = (volatile void*)mmap(NULL, number_byte, PROT_READ | PROT_WRITE, MAP_SHARED, fd, base_phy_addr);
//     if (ptr == MAP_FAILED) {
//         perror("Ошибка отображения памяти DMAmemory_maping");
//         ioctl(fd, MEM_FREE); // Освобождение памяти
//         close(fd);
//         exit(EXIT_FAILURE);
//     }
//     return ptr;
// }