#include "civilization_internal.h"
#include "civilization_generated_core.h"

#include <stdio.h>
#include <stdlib.h>

static int load_file(const char *path, uint8_t **data, size_t *size)
{
    FILE *f = fopen(path, "rb");
    long n;

    if (f == NULL) {
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    n = ftell(f);
    if (n < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return 0;
    }
    *data = (uint8_t *)malloc((size_t)n);
    if (*data == NULL) {
        fclose(f);
        return 0;
    }
    if (fread(*data, 1, (size_t)n, f) != (size_t)n) {
        free(*data);
        fclose(f);
        return 0;
    }
    fclose(f);
    *size = (size_t)n;
    return 1;
}

static int run_case(const uint8_t *rom, size_t rom_size,
                    uint8_t bank, uint16_t pc, uint16_t raw_x, uint16_t target)
{
    CivRecomp c = {0};
    char err[256];
    const uint16_t old_s = 0x01FFu;

    civ_reset(&c);
    if (!civ_attach_verified_rom(&c, rom, rom_size, err, sizeof(err))) {
        return 0;
    }
    c.cpu.pbr = bank;
    c.cpu.pc = pc;
    c.cpu.e = 0u;
    c.cpu.p = (uint8_t)(c.cpu.p & ~(CIV_P_M | CIV_P_X));
    c.cpu.s = old_s;
    c.cpu.x = raw_x;

    if (!civ_generated_core_step(&c) || c.failed) {
        return 0;
    }
    if (c.cpu.pbr != bank || c.cpu.pc != target ||
        c.cpu.s != (uint16_t)(old_s - 2u)) {
        return 0;
    }
    /* JSR pushes PC+2, high then low. RTS therefore returns to PC+3. */
    if (c.wram[old_s] != (uint8_t)((pc + 2u) >> 8)) {
        return 0;
    }
    if (c.wram[(uint16_t)(old_s - 1u)] != (uint8_t)(pc + 2u)) {
        return 0;
    }
    return 1;
}

static int run_reject(const uint8_t *rom, size_t rom_size,
                      uint8_t bank, uint16_t pc, uint16_t raw_x)
{
    CivRecomp c = {0};
    char err[256];

    civ_reset(&c);
    if (!civ_attach_verified_rom(&c, rom, rom_size, err, sizeof(err))) {
        return 0;
    }
    c.cpu.pbr = bank;
    c.cpu.pc = pc;
    c.cpu.e = 0u;
    c.cpu.p = (uint8_t)(c.cpu.p & ~(CIV_P_M | CIV_P_X));
    c.cpu.s = 0x01FFu;
    c.cpu.x = raw_x;

    if (civ_generated_core_step(&c) != 0 || !c.failed) {
        return 0;
    }
    return c.frontier_address[0] != '\0';
}

int main(int argc, char **argv)
{
    static const struct {
        uint8_t bank;
        uint16_t pc;
        uint16_t x;
        uint16_t target;
    } legal[] = {
        {0xC2u, 0x37A7u, 0x37ACu, 0x37BAu},
        {0xC2u, 0x37A7u, 0x37AEu, 0x37FEu},
        {0xC2u, 0x37A7u, 0x37B0u, 0x385Fu},
        {0xC2u, 0x37A7u, 0x37B2u, 0x3A37u},
        {0xC2u, 0x37A7u, 0x37B4u, 0x37D6u},
        {0xC3u, 0x236Eu, 0x2373u, 0x237Bu},
        {0xC3u, 0x236Eu, 0x2375u, 0x24D2u},
        {0xC3u, 0x236Eu, 0x2377u, 0x25EEu},
        {0xC3u, 0x236Eu, 0x2379u, 0x2732u}
    };
    uint8_t *rom = NULL;
    size_t rom_size = 0;
    size_t n;

    if (argc != 2 || !load_file(argv[1], &rom, &rom_size)) {
        return 2;
    }
    for (n = 0; n < sizeof(legal) / sizeof(legal[0]); n++) {
        if (!run_case(rom, rom_size, legal[n].bank, legal[n].pc,
                      legal[n].x, legal[n].target)) {
            free(rom);
            return (int)(10u + n);
        }
    }
    if (!run_reject(rom, rom_size, 0xC2u, 0x37A7u, 0x37AAu) ||
        !run_reject(rom, rom_size, 0xC2u, 0x37A7u, 0x37B6u) ||
        !run_reject(rom, rom_size, 0xC3u, 0x236Eu, 0x2371u) ||
        !run_reject(rom, rom_size, 0xC3u, 0x236Eu, 0x237Bu)) {
        free(rom);
        return 30;
    }
    free(rom);
    puts("v33 pre-biased indexed-indirect generated cases PASS");
    return 0;
}
