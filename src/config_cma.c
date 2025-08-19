#include "config_cma.h"


void calculate_size_byte(struct config_cma* cmastr) {
    uint32_t remains;
    cmastr->speed_write_inddr = (cmastr->speed_sample * 4);
    cmastr->size_shot_one = cmastr->speed_write_inddr * cmastr->number_chennal;
    remains = (uint32_t)(((float)(cmastr->size_shot_one)) * cmastr->sizeshot) % 4096;
    cmastr->size_ddr = (size_t)((((float)(cmastr->size_shot_one) * cmastr->sizeshot) - remains) + 4096);
    cmastr->last_number_word = cmastr->size_ddr / 4;
}