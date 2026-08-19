#include "civilization_internal.h"

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <thread>

static void prepare(CivRecomp *core, std::uint16_t backdrop)
{
    civ_reset(core);
    core->ppu.forced_blank = 0u;
    core->ppu.brightness = 15u;
    core->ppu.bg_mode = 1u;
    core->ppu.main_screen_layers = 0u;
    core->ppu.sub_screen_layers = 0u;
    core->cgram[0] = static_cast<std::uint8_t>(backdrop);
    core->cgram[1] = static_cast<std::uint8_t>(backdrop >> 8);
}

static void render_many(CivRecomp *core, std::uint32_t expected,
                        std::atomic<int> *failed)
{
    for (unsigned iteration = 0; iteration < 32u; ++iteration) {
        const std::uint32_t *pixels;
        if (!civ_render_current_frame(core)) {
            failed->store(1);
            return;
        }
        pixels = civ_get_framebuffer_rgba(core);
        if (!pixels || pixels[0] != expected ||
            pixels[CIV_FRAME_PIXELS - 1u] != expected) {
            failed->store(1);
            return;
        }
    }
}

int main()
{
    CivRecomp *red = civ_create(nullptr, 0u);
    CivRecomp *green = civ_create(nullptr, 0u);
    std::atomic<int> failed{0};
    if (!red || !green) {
        civ_destroy(red);
        civ_destroy(green);
        return 1;
    }
    prepare(red, 0x001Fu);
    prepare(green, 0x03E0u);
    std::thread red_thread(render_many, red, UINT32_C(0xFFFF0000), &failed);
    std::thread green_thread(render_many, green, UINT32_C(0xFF00FF00), &failed);
    red_thread.join();
    green_thread.join();
    civ_destroy(red);
    civ_destroy(green);
    return failed.load() != 0;
}
