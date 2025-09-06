#include "dma_pl.h"



int dma_pl_wr_init(struct dma_pl* obj_dma) {
    if (obj_dma->ptr == NULL) {
        perror("Ошибка получения физического адреса");
        return -1;
    }
    obj_dma->ptr[0] = 0;
    usleep(1);
    obj_dma->ptr[4] = obj_dma->phys_addr;
    obj_dma->ptr[5] = obj_dma->size_ddr_byte;
    dma_pl_wr_enable(obj_dma, obj_dma->default_stopstart);
    obj_dma->ptr[0] = 1;
    return 0;
}

void dma_pl_wr_enable(struct dma_pl* obj_dma, uint32_t default_stopstart) {
    obj_dma->ptr[0] = default_stopstart;
}


void dma_pl_wr_close(struct dma_pl* obj_dma) {
    dma_pl_wr_enable(obj_dma, 0);
    obj_dma->ptr[0] = 0;
}


uint32_t dma_pl_wr_addr(struct dma_pl* obj_dma) {
    return (uint32_t)obj_dma->ptr[10];
}


// uint32_t DMA_get_decriment_word_write(uint32_t *ptr) {
//     return ptr[14];
// }
// uint32_t DMA_get_decriment_addr_word_write(uint32_t *ptr) {
//     return ptr[15];
// }
// void DMA_set_decimator_val_write(uint32_t *ptr, uint32_t value) {
//     ptr[9] = value;
// }


// void DMA_enable_read(uint32_t *ptr, uint32_t stopstart) {
//     ptr[3] = stopstart;
// }
// void DMA_start_read(uint32_t *ptr, uint32_t phys_addr, uint32_t size_ddr_byte, uint32_t last_ddr_word, uint32_t default_stopstart) {
//     ptr[1] = 0;
//     usleep(1);
//     ptr[6] = phys_addr;
//     ptr[7] = size_ddr_byte;
//     DMA_enable_read(ptr, default_stopstart);
//     ptr[1] = 1;
// }
// void DMA_finish_read(uint32_t *ptr) {
//     ptr[1] = 0;
//     // enable_read_dma(ptr, 0);
// }
// uint32_t DMA_get_addr_word_read(uint32_t *ptr) {
//     return ptr[11];
// }