#include "civilization_static_recomp.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct MappingCase { uint8_t bank; uint16_t address; size_t expected; } MappingCase;

int main(void) {
    const MappingCase rom_mapped[] = {
        {0x00u,0x8000u,0x008000u},{0x00u,0xFFFFu,0x00FFFFu},
        {0x3Fu,0x8000u,0x178000u},{0x40u,0x0000u,0x000000u},
        {0x54u,0x0000u,0x140000u},{0x57u,0xFFFFu,0x17FFFFu},
        {0x58u,0x0000u,0x100000u},{0x80u,0x8000u,0x008000u},
        {0xC0u,0x0000u,0x000000u},{0xD4u,0x8000u,0x148000u},
        {0xD8u,0x0000u,0x100000u},{0xFFu,0xFFFFu,0x17FFFFu},
    };
    const MappingCase sram_mapped[] = {
        {0x20u,0x6000u,0x0000u},{0x20u,0x7FFFu,0x1FFFu},
        {0x21u,0x6000u,0x2000u},{0x22u,0x6000u,0x4000u},
        {0x23u,0x6000u,0x6000u},{0x24u,0x6000u,0x0000u},
        {0x3Fu,0x7FFFu,0x7FFFu},{0xA0u,0x6000u,0x0000u},
        {0xBFu,0x7FFFu,0x7FFFu},
    };
    const struct { uint8_t bank; uint16_t address; } rom_unmapped[] = {
        {0x00u,0x0000u},{0x3Fu,0x7FFFu},{0x7Eu,0x8000u},{0x7Fu,0xFFFFu},{0x80u,0x2000u},
    };
    const struct { uint8_t bank; uint16_t address; } sram_unmapped[] = {
        {0x1Fu,0x6000u},{0x20u,0x5FFFu},{0x20u,0x8000u},{0x40u,0x6000u},{0x9Fu,0x6000u},{0xC0u,0x6000u},
    };
    size_t i,offset=0;
    for(i=0;i<sizeof(rom_mapped)/sizeof(rom_mapped[0]);++i){
        if(!civ_hirom_rom_offset(rom_mapped[i].bank,rom_mapped[i].address,&offset)||offset!=rom_mapped[i].expected){
            fprintf(stderr,"ROM map failed %02X:%04X expected=%zx got=%zx\n",rom_mapped[i].bank,rom_mapped[i].address,rom_mapped[i].expected,offset);return 1;}
    }
    for(i=0;i<sizeof(rom_unmapped)/sizeof(rom_unmapped[0]);++i){if(civ_hirom_rom_offset(rom_unmapped[i].bank,rom_unmapped[i].address,&offset)){fprintf(stderr,"unexpected ROM map %02X:%04X\n",rom_unmapped[i].bank,rom_unmapped[i].address);return 2;}}
    for(i=0;i<sizeof(sram_mapped)/sizeof(sram_mapped[0]);++i){
        if(!civ_hirom_sram_offset(sram_mapped[i].bank,sram_mapped[i].address,&offset)||offset!=sram_mapped[i].expected){
            fprintf(stderr,"SRAM map failed %02X:%04X expected=%zx got=%zx\n",sram_mapped[i].bank,sram_mapped[i].address,sram_mapped[i].expected,offset);return 3;}
    }
    for(i=0;i<sizeof(sram_unmapped)/sizeof(sram_unmapped[0]);++i){if(civ_hirom_sram_offset(sram_unmapped[i].bank,sram_unmapped[i].address,&offset)){fprintf(stderr,"unexpected SRAM map %02X:%04X\n",sram_unmapped[i].bank,sram_unmapped[i].address);return 4;}}
    puts("PASS Civilization FastROM HiROM ROM mirrors plus 32 KiB HiROM SRAM aperture/mirroring");
    return 0;
}
