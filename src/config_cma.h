#ifndef CONFIG_CMA_H
#define CONFIG_CMA_H

#include <unistd.h> // Unix Standard Header
#include <stdint.h>



struct config_cma {
    uint32_t speed_sample;
    uint32_t number_chennal;
    float sizeshot; // Размер кадра в секундах
    uint32_t last_number_word;
    uint32_t speed_write_inddr;
    size_t size_shot_one; // Размер кадра в одной секунде
    size_t size_ddr;
    void (*calculate_size_byte_ptr)(struct config_cma*);
};


void calculate_size_byte(struct config_cma*);


#endif