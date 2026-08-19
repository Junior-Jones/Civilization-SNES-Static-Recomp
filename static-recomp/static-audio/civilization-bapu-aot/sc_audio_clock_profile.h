#ifndef SC_AUDIO_CLOCK_PROFILE_H
#define SC_AUDIO_CLOCK_PROFILE_H
#include <stdint.h>

/* Pinned oracle-qualified NTSC S-SMP clock relation.
 * MesenCE 2.2.1 runs its default SPC clock from an internal 32,040 Hz DSP
 * sample rate.  One S-SMP cycle is therefore exactly 32,040 * 32 =
 * 1,025,280 cycles/second against the NTSC 21,477,270 Hz S-CPU master clock.
 * The reduced exact rational is 34,176 / 715,909.  Keep this deterministic
 * profile tied to the pinned project oracle; do not substitute an approximate
 * host-audio rate or a wall-clock calibration. */
#define SC_AUDIO_CLOCK_PROFILE_NAME "mesence-2.2.1-ntsc-32040-exact"
#define SC_AUDIO_CLOCK_RATIO_NUMERATOR UINT32_C(34176)
#define SC_AUDIO_CLOCK_RATIO_DENOMINATOR UINT32_C(715909)
#define SC_AUDIO_CLOCK_MASTER_HZ UINT32_C(21477270)

static inline uint64_t sc_audio_clock_advance(uint64_t master_delta,
                                              uint32_t *remainder) {
    const uint64_t whole = master_delta / SC_AUDIO_CLOCK_RATIO_DENOMINATOR;
    const uint64_t partial = master_delta % SC_AUDIO_CLOCK_RATIO_DENOMINATOR;
    const uint64_t numerator = partial * SC_AUDIO_CLOCK_RATIO_NUMERATOR +
                               (remainder ? *remainder : 0u);
    const uint64_t cycles = whole * SC_AUDIO_CLOCK_RATIO_NUMERATOR +
                            numerator / SC_AUDIO_CLOCK_RATIO_DENOMINATOR;
    if (remainder)
        *remainder = (uint32_t)(numerator % SC_AUDIO_CLOCK_RATIO_DENOMINATOR);
    return cycles;
}

static inline uint32_t sc_audio_clock_remainder(uint64_t master_clock) {
    const uint64_t partial = master_clock % SC_AUDIO_CLOCK_RATIO_DENOMINATOR;
    return (uint32_t)((partial * SC_AUDIO_CLOCK_RATIO_NUMERATOR) %
                      SC_AUDIO_CLOCK_RATIO_DENOMINATOR);
}

static inline uint64_t sc_audio_clock_closed_form(uint64_t master_clock,
                                                  uint32_t initial_remainder) {
    const uint64_t whole = master_clock / SC_AUDIO_CLOCK_RATIO_DENOMINATOR;
    const uint64_t partial = master_clock % SC_AUDIO_CLOCK_RATIO_DENOMINATOR;
    return whole * SC_AUDIO_CLOCK_RATIO_NUMERATOR +
           (partial * SC_AUDIO_CLOCK_RATIO_NUMERATOR + initial_remainder) /
               SC_AUDIO_CLOCK_RATIO_DENOMINATOR;
}
#endif
