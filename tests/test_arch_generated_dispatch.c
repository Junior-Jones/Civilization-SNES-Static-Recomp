#include "civilization_internal.h"
#include "civilization_generated_core.h"

#include <stdlib.h>
#include <string.h>

int main(void)
{
    CivRecomp *core = civ_create(NULL, 0u);
    int result;
    if (!core) return 1;
    civ_reset(core);
    core->cpu.pbr = 0x7Eu;
    core->cpu.pc = 0x1234u;
    result = civ_generated_core_step(core);
    if (result != 0 || !core->failed ||
        strstr(civ_frontier_reason(core), "outside the closed generated authority") == NULL ||
        strcmp(civ_frontier_address(core), "7E:1234") != 0) {
        civ_destroy(core);
        return 2;
    }
    civ_destroy(core);
    return 0;
}
