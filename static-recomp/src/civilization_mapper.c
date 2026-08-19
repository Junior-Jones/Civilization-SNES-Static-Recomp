#include "civilization_static_recomp.h"

#include <stddef.h>
#include <stdint.h>

/*
 * Target-specific Civilization HiROM mapping.
 *
 * The exact dump is 0x180000 bytes while its header declares a 0x200000-byte
 * ROM address space.  SNES cartridge mirroring for a non-power-of-two image is
 * therefore not equivalent to a simple address % file_size operation.
 */
static size_t mirror_address(size_t address, size_t size) {
    size_t base = 0;
    size_t mask = 1;
    size_t largest;

    largest = address > (size - 1u) ? address : (size - 1u);
    while (mask <= largest / 2u) {
        mask <<= 1u;
    }

    while (address >= size) {
        while (mask != 0u && (address & mask) == 0u) {
            mask >>= 1u;
        }
        if (mask == 0u) {
            return base + (address % size);
        }
        address -= mask;
        if (size > mask) {
            size -= mask;
            base += mask;
        }
        mask >>= 1u;
    }
    return base + address;
}

int civ_hirom_rom_offset(uint8_t bank, uint16_t address, size_t *offset) {
    size_t linear;

    if (!offset) return 0;
    if (bank == 0x7Eu || bank == 0x7Fu) return 0;

    if (bank <= 0x3Fu || (bank >= 0x80u && bank <= 0xBFu)) {
        if (address < 0x8000u) return 0;
        linear = ((size_t)(bank & 0x3Fu) << 16u) | (size_t)address;
    } else if ((bank >= 0x40u && bank <= 0x7Du) || bank >= 0xC0u) {
        linear = ((size_t)(bank & 0x3Fu) << 16u) | (size_t)address;
    } else {
        return 0;
    }
    *offset = mirror_address(linear, CIV_ROM_SIZE);
    return 1;
}

/*
 * Standard HiROM save-RAM aperture used by the Civilization cartridge class:
 * banks $20-$3F/$A0-$BF, addresses $6000-$7FFF.  The board map removes the
 * aperture bits, then the exact 32 KiB SRAM mirrors every four banks.  For
 * this target that reduces to the compact expression below.
 */
int civ_hirom_sram_offset(uint8_t bank, uint16_t address, size_t *offset) {
    if (!offset) return 0;
    if (!((bank >= 0x20u && bank <= 0x3Fu) || (bank >= 0xA0u && bank <= 0xBFu))) return 0;
    if (address < 0x6000u || address > 0x7FFFu) return 0;
    *offset = ((size_t)(bank & 0x03u) << 13u) | (size_t)(address & 0x1FFFu);
    return *offset < CIV_SRAM_SIZE;
}
