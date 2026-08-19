#include "civilization_diagnostics.h"

#include <stdint.h>

int main(void)
{
    CivRecomp *core = civ_create(NULL, 0u);
    CivDiagnosticState state;
    CivVideoCheckpoint video;
    uint8_t bytes[4] = {1u, 1u, 1u, 1u};
    if (!core) return 1;
    if (!civ_diagnostics_capture(core, &state) || state.cpu.pbr != 0u ||
        state.cpu.pc != 0x804Au || state.instruction_count != 0u ||
        state.frame_count != 0u || state.failed != 0u) {
        civ_destroy(core); return 2;
    }
    if (!civ_diagnostics_read_wram(core, 0u, bytes, sizeof(bytes)) ||
        bytes[0] || bytes[1] || bytes[2] || bytes[3] ||
        civ_diagnostics_read_wram(core, CIV_WRAM_SIZE - 1u, bytes, sizeof(bytes))) {
        civ_destroy(core); return 3;
    }
    civ_capture_video_checkpoint(core, &video);
    if (video.frame_count != 0u || video.master_clock != 0u ||
        video.forced_blank != state.ppu.forced_blank) {
        civ_destroy(core); return 4;
    }
    civ_destroy(core);
    return 0;
}
