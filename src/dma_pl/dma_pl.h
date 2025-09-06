#ifndef DMA_PL_H
#define DMA_PL_H


#include <unistd.h>
#include <stdint.h>


struct dma_pl {
    uint32_t *ptr;
    uint32_t phys_addr;
    uint32_t size_ddr_byte;
    uint32_t default_stopstart;
};


int dma_pl_wr_init(struct dma_pl* obj_dma);
void dma_pl_wr_enable(struct dma_pl* obj_dma, uint32_t default_stopstart);
void dma_pl_wr_close(struct dma_pl* obj_dma);
uint32_t dma_pl_wr_addr(struct dma_pl* obj_dma);


// uint32_t DMA_get_decriment_word_write(uint32_t *ptr);

// uint32_t DMA_get_decriment_addr_word_write(uint32_t *ptr);

// void DMA_set_decimator_val_write(uint32_t *ptr, uint32_t value);



#endif