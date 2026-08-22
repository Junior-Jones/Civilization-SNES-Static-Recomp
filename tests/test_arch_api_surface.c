#include "civilization_static_recomp.h"

#include <stddef.h>
#include <stdint.h>

_Static_assert(CIV_ROM_SIZE == 1572864u, "unexpected ROM size");
_Static_assert(CIV_FRAME_WIDTH == 256u, "unexpected frame width");
_Static_assert(CIV_FRAME_HEIGHT == 224u, "unexpected frame height");
_Static_assert(CIV_SRAM_SIZE == 32768u, "unexpected SRAM size");

int main(void)
{
    CivRecomp *instance;
    int (*verify)(const uint8_t *, size_t, CivRomInfo *, char *, size_t) = civ_verify_rom;
    CivRecomp *(*create)(char *, size_t) = civ_create;
    void (*destroy)(CivRecomp *) = civ_destroy;
    int (*attach)(CivRecomp *, const uint8_t *, size_t, char *, size_t) = civ_attach_verified_rom;
    void (*reset)(CivRecomp *) = civ_reset;
    int (*run)(CivRecomp *, uint64_t) = civ_run_static;
    int (*render)(CivRecomp *) = civ_render_current_frame;
    const uint32_t *(*framebuffer)(const CivRecomp *) = civ_get_framebuffer_rgba;
    int (*save)(const CivRecomp *, const char *, char *, size_t) = civ_snapshot_save;
    int (*load)(CivRecomp *, const uint8_t *, size_t, const char *, char *, size_t) = civ_snapshot_load;
    void (*input)(CivRecomp *, unsigned, uint16_t) = civ_set_controller_input;
    void (*pcm)(CivRecomp *, CivHostPcmSink, void *) = civ_v20_set_host_pcm_sink;

    if (!(verify && create && destroy && attach && reset && run && render &&
          framebuffer && save && load && input && pcm)) return 1;
    instance = civ_create(NULL, 0u);
    if (!instance || civ_frame_count(instance) != 0u ||
        civ_instruction_count(instance) != 0u || civ_master_clock(instance) != 0u ||
        civ_has_failed(instance) || civ_audio_active(instance) ||
        !civ_run_to_frame(instance, 0u, 0u)) {
        civ_destroy(instance);
        return 2;
    }
    civ_reset(instance);
    if (civ_frame_count(instance) != 0u || civ_has_failed(instance)) {
        civ_destroy(instance);
        return 3;
    }
    civ_destroy(instance);
    civ_destroy(NULL);
    return 0;
}
