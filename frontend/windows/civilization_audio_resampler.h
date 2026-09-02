#ifndef CIVILIZATION_AUDIO_RESAMPLER_H
#define CIVILIZATION_AUDIO_RESAMPLER_H

#include <stddef.h>
#include <stdint.h>

typedef enum CivilizationAudioResamplerMode {
    CIVILIZATION_AUDIO_RESAMPLER_HERMITE = 0,
    CIVILIZATION_AUDIO_RESAMPLER_LINEAR = 1,
    CIVILIZATION_AUDIO_RESAMPLER_NEAREST = 2
} CivilizationAudioResamplerMode;

#define CIVILIZATION_AUDIO_RESAMPLER_PENDING_FRAMES 16384u

typedef struct CivilizationAudioHermiteResampler {
    double previous_left[4];
    double previous_right[4];
    double rate_ratio;
    double fraction;
    int16_t last_left;
    int16_t last_right;
    int mode;
    int16_t pending_samples[CIVILIZATION_AUDIO_RESAMPLER_PENDING_FRAMES * 2u];
    size_t pending_frames;
} CivilizationAudioHermiteResampler;

void civilization_audio_resampler_reset(CivilizationAudioHermiteResampler *resampler);
void civilization_audio_resampler_set_rates(CivilizationAudioHermiteResampler *resampler,
                                      double source_rate,
                                      double destination_rate);
void civilization_audio_resampler_set_mode(CivilizationAudioHermiteResampler *resampler,
                                     int mode);

/* The caller must supply enough output storage.  For the Civilization
   32,040 Hz to 48,000 Hz path, twice input_frames is always sufficient. */
size_t civilization_audio_resampler_process(CivilizationAudioHermiteResampler *resampler,
                                      const int16_t *input,
                                      size_t input_frames,
                                      int16_t *output,
                                      size_t output_capacity_frames);

#endif
