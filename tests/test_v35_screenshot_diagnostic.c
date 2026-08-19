#include "civilization_app_core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, unsigned char **bytes, size_t *size)
{
    FILE *file;
    long length;
    *bytes = NULL;
    *size = 0u;
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, 0, SEEK_END) != 0) {
        (void)fclose(file);
        return 0;
    }
    length = ftell(file);
    if (length < 0 || fseek(file, 0, SEEK_SET) != 0) {
        (void)fclose(file);
        return 0;
    }
    *bytes = (unsigned char *)malloc((size_t)length + 1u);
    if (!*bytes) {
        (void)fclose(file);
        return 0;
    }
    if (fread(*bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(*bytes);
        *bytes = NULL;
        (void)fclose(file);
        return 0;
    }
    (*bytes)[length] = 0u;
    *size = (size_t)length;
    return fclose(file) == 0;
}

static int contains(const unsigned char *text, const char *token)
{
    return strstr((const char *)text, token) != NULL;
}

int main(int argc, char **argv)
{
    static const char *required[] = {
        "format=civilization-v35-screenshot-static-log-v1",
        "authority_contexts=103584",
        "runtime_decoder=0",
        "runtime_learning=0",
        "runtime_fallback=0",
        "cpu_context=",
        "cpu_context_key=",
        "framebuffer_fnv1a64=",
        "wram_fnv1a64=",
        "sram_fnv1a64=",
        "guest_state_fnv1a64=",
        "audio_smp_instructions=",
        "frontier_reason=",
    };
    CivilizationRecomp *instance = NULL;
    CivilizationRecompFrameResult frame;
    unsigned char *rom = NULL;
    unsigned char *log = NULL;
    size_t rom_size = 0u;
    size_t log_size = 0u;
    size_t index;
    char error[256] = {0};
    int result = 1;
    if (argc != 3) {
        (void)fprintf(stderr, "usage: test ROM LOG\n");
        return 2;
    }
    if (!read_file(argv[1], &rom, &rom_size) ||
        rom_size != CIVILIZATION_RECOMP_ROM_SIZE) {
        (void)fprintf(stderr, "could not read exact ROM\n");
        goto done;
    }
    if (!civilization_recomp_create(&instance, rom, rom_size,
                                    error, sizeof(error))) {
        (void)fprintf(stderr, "create failed: %s\n", error);
        goto done;
    }
    if (!civilization_recomp_advance_headless(instance, 0u, 1u, &frame)) {
        (void)fprintf(stderr, "advance failed: %s\n",
                      civilization_recomp_last_error(instance));
        goto done;
    }
    if (!civilization_recomp_write_diagnostic_log(
            instance, argv[2], "Screenshots\\diagnostic-test.png",
            error, sizeof(error))) {
        (void)fprintf(stderr, "log failed: %s\n", error);
        goto done;
    }
    if (!read_file(argv[2], &log, &log_size) || log_size == 0u) {
        (void)fprintf(stderr, "could not read diagnostic log\n");
        goto done;
    }
    for (index = 0u; index < sizeof(required) / sizeof(required[0]); ++index) {
        if (!contains(log, required[index])) {
            (void)fprintf(stderr, "missing diagnostic field: %s\n",
                          required[index]);
            goto done;
        }
    }
    (void)printf("PASS screenshot diagnostic log: %lu bytes, frame=%u\n",
                 (unsigned long)log_size, frame.end_frame);
    result = 0;
done:
    civilization_recomp_destroy(instance);
    free(rom);
    free(log);
    (void)remove(argv[2]);
    return result;
}
