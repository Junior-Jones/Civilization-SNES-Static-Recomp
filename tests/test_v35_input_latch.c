#include "civilization_input_latch.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

enum {
    INPUT_UP = 0x0800u,
    INPUT_DOWN = 0x0400u,
    INPUT_A = 0x0080u
};

int main(void)
{
    CivInputLatch latch = {0u, 0u};
    uint16_t sampled;

    /* A complete tap between host frames must survive for one guest sample. */
    civ_input_latch_press(&latch, INPUT_UP, INPUT_DOWN, 0);
    civ_input_latch_release(&latch, INPUT_UP);
    sampled = civ_input_latch_sample(&latch);
    assert(sampled == INPUT_UP);
    civ_input_latch_consume(&latch, sampled);
    assert(civ_input_latch_sample(&latch) == 0u);

    /* A newer opposite tap replaces an older unsampled direction. */
    civ_input_latch_press(&latch, INPUT_UP, INPUT_DOWN, 0);
    civ_input_latch_release(&latch, INPUT_UP);
    civ_input_latch_press(&latch, INPUT_DOWN, INPUT_UP, 0);
    civ_input_latch_release(&latch, INPUT_DOWN);
    sampled = civ_input_latch_sample(&latch);
    assert(sampled == INPUT_DOWN);
    civ_input_latch_consume(&latch, sampled);
    assert(civ_input_latch_sample(&latch) == 0u);

    /* Held input remains active after its initial edge is consumed. */
    civ_input_latch_press(&latch, INPUT_A, 0u, 0);
    sampled = civ_input_latch_sample(&latch);
    assert(sampled == INPUT_A);
    civ_input_latch_consume(&latch, sampled);
    assert(civ_input_latch_sample(&latch) == INPUT_A);
    civ_input_latch_release(&latch, INPUT_A);
    assert(civ_input_latch_sample(&latch) == 0u);

    /* Windows auto-repeat must not create a release-tail press. */
    civ_input_latch_press(&latch, INPUT_A, 0u, 0);
    sampled = civ_input_latch_sample(&latch);
    civ_input_latch_consume(&latch, sampled);
    civ_input_latch_press(&latch, INPUT_A, 0u, 1);
    civ_input_latch_release(&latch, INPUT_A);
    assert(civ_input_latch_sample(&latch) == 0u);

    civ_input_latch_reset(&latch);
    assert(latch.held == 0u && latch.pending_press == 0u);
    puts("PASS Version 35 host keyboard tap latch and opposite-direction priority");
    return 0;
}
