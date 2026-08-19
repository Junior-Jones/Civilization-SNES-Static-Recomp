#include "civilization_audio.h"
#include "civilization_diagnostics.h"

#include <cstdint>
#include <cstdlib>

static void count_pcm(void *context, std::int16_t, std::int16_t)
{
    auto *count = static_cast<std::uint64_t *>(context);
    ++*count;
}

int main()
{
    CivRecomp *first = civ_create(nullptr, 0u);
    CivRecomp *second = civ_create(nullptr, 0u);
    CivV20AudioStatus status{};
    std::uint64_t first_pcm = 0u;
    std::uint64_t before_transfer;
    if (!first || !second) {
        civ_destroy(first);
        civ_destroy(second);
        return 1;
    }
    civ_reset(first);
    civ_reset(second);
    if (!civ_v20_audio_begin(first)) return 2;
    civ_v20_set_host_pcm_sink(first, count_pcm, &first_pcm);
    first->master_clock = UINT64_C(500000);
    if (!civ_v20_audio_sync(first) || first_pcm == 0u ||
        !civ_v20_get_audio_status(first, &status)) return 3;

    before_transfer = first_pcm;
    if (!civ_v20_audio_begin(second)) return 4;
    if (civ_v20_get_audio_status(first, &status) ||
        !civ_v20_get_audio_status(second, &status)) return 5;
    second->master_clock = UINT64_C(500000);
    if (!civ_v20_audio_sync(second) || first_pcm != before_transfer) return 6;

    civ_v20_audio_release_internal(second);
    civ_v20_set_host_pcm_sink(first, nullptr, nullptr);
    civ_destroy(first);
    civ_destroy(second);
    return 0;
}
