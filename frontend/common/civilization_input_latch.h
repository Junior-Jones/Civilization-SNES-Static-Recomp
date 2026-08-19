#ifndef CIVILIZATION_INPUT_LATCH_H
#define CIVILIZATION_INPUT_LATCH_H

#include <stdint.h>

typedef struct CivInputLatch {
    uint16_t held;
    uint16_t pending_press;
} CivInputLatch;

void civ_input_latch_reset(CivInputLatch *latch);
void civ_input_latch_press(CivInputLatch *latch, uint16_t mask,
                           uint16_t unsampled_opposite_mask, int repeated);
void civ_input_latch_release(CivInputLatch *latch, uint16_t mask);
uint16_t civ_input_latch_sample(const CivInputLatch *latch);
void civ_input_latch_consume(CivInputLatch *latch, uint16_t sampled_mask);

#endif
