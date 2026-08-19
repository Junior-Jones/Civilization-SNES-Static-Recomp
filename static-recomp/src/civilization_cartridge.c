#include "civilization_internal.h"

int civ_cartridge_read(CivRecomp *i,uint8_t bank,uint16_t local,uint8_t *value)
{
    size_t offset;
    if(civ_hirom_sram_offset(bank,local,&offset)){*value=i->sram[offset];return 1;}
    if(civ_hirom_rom_offset(bank,local,&offset)){
        if(!i->rom||i->rom_size!=CIV_ROM_SIZE)
            return civ_fail_frontier(i,"ROM data read requires the exact verified external Civilization ROM to be attached.",NULL);
        *value=i->rom[offset];return 1;
    }
    return -1;
}

int civ_cartridge_write(CivRecomp *i,uint32_t address,uint8_t value)
{
    size_t offset;
    uint8_t bank=(uint8_t)(address>>16);uint16_t local=(uint16_t)address;
    if(civ_hirom_sram_offset(bank,local,&offset)){
        if(i->sram[offset]!=value){i->sram[offset]=value;i->sram_dirty=1u;}
        return 1;
    }
    if(civ_hirom_rom_offset(bank,local,&offset)){
        i->ignored_rom_write_count++;i->last_ignored_rom_write_address=address;i->last_ignored_rom_write_value=value;
        return 1;
    }
    return -1;
}
