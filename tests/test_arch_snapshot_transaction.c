#include "civilization_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **data, size_t *size)
{
    FILE *file = fopen(path, "rb");
    long length;
    if (!file) return 0;
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) { fclose(file); return 0; }
    *data = (uint8_t *)malloc((size_t)length);
    if (!*data || fread(*data, 1u, (size_t)length, file) != (size_t)length) {
        free(*data); *data = NULL; fclose(file); return 0;
    }
    fclose(file); *size = (size_t)length; return 1;
}

static int write_file(const char *path, const uint8_t *data, size_t size)
{
    FILE *file = fopen(path, "wb");
    int ok, close_ok;
    if (!file) return 0;
    ok = fwrite(data, 1u, size, file) == size;
    close_ok = fclose(file) == 0;
    return ok && close_ok;
}

int main(int argc, char **argv)
{
    CivRecomp *core;
    uint8_t *rom = NULL, *snapshot = NULL;
    size_t rom_size = 0u, snapshot_size = 0u;
    char error[256];
    uint16_t sentinel_pc;
    uint8_t sentinel_wram;
    uint64_t sentinel_frame;
    int result = 1;
    if (argc != 4) return 2;
    if (!read_file(argv[1], &rom, &rom_size)) return 3;
    core = civ_create(error, sizeof(error));
    if (!core) { free(rom); return 4; }
    civ_reset(core);
    if (!civ_attach_verified_rom(core, rom, rom_size, error, sizeof(error))) goto done;
    core->wram[0x1234u] = 0x42u;
    core->frame_count = 17u;
    if (!civ_snapshot_save(core, argv[2], error, sizeof(error)) ||
        !read_file(argv[2], &snapshot, &snapshot_size) || snapshot_size < 128u) goto done;

    core->cpu.pc = 0xBEEFu;
    core->wram[0x1234u] = 0xA5u;
    core->frame_count = UINT64_C(123456);
    sentinel_pc = core->cpu.pc;
    sentinel_wram = core->wram[0x1234u];
    sentinel_frame = core->frame_count;

    snapshot[snapshot_size / 2u] ^= 0x80u;
    if (!write_file(argv[3], snapshot, snapshot_size)) goto done;
    if (civ_snapshot_load(core, rom, rom_size, argv[3], error, sizeof(error)) ||
        core->cpu.pc != sentinel_pc || core->wram[0x1234u] != sentinel_wram ||
        core->frame_count != sentinel_frame) goto done;

    if (!civ_snapshot_load(core, rom, rom_size, argv[2], error, sizeof(error)) ||
        core->cpu.pc != 0x804Au || core->wram[0x1234u] != 0x42u ||
        core->frame_count != 17u || core->rom != rom) goto done;
    result = 0;

done:
    (void)remove(argv[2]);
    (void)remove(argv[3]);
    free(snapshot);
    civ_destroy(core);
    free(rom);
    return result;
}
